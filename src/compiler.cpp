#include "wrap.h"

static bool isSource(const char* ext)
{

    switch (ext[0]) {
    //case 'i':
    //    return !strcmp(ext, "i")
    //        || !strcmp(ext, "ii");
    case 'c':
        return !strcmp(ext, "c")
            || !strcmp(ext, "cc")
            || !strcmp(ext, "cpp")
            || !strcmp(ext, "cxx")
            || !strcmp(ext, "cp")
            || !strcmp(ext, "c++");
    case 'C':
        return !strcmp(ext, "C");
    //case 'm':
    //    return !strcmp(ext,"m")
    //        || !strcmp(ext,"mm")
    //        || !strcmp(ext,"mi")
    //        || !strcmp(ext,"mii");
    //case 'M':
    //    return !strcmp(ext, "M");
//#ifdef ENABLE_REMOTE_ASSEMBLE
//    case 's':
//        return !strcmp(ext, "s");
//    case 'S':
//        return !strcmp(ext, "S");
//#endif
    default:
        return 0;
    }    
}

static bool parseCommandArgs(int argc, char** argv)
{
#ifndef NDEBUG
    std::cout << "gcc argv:\n";
    for (int i = 0; i < argc; ++i)
        std::cout << "argv[" << i << "] : " << argv[i] << std::endl; 
#endif

    char* a;
    bool was_driver = false, was_make_rule = false;
    bool was_opt_c = false;
    bool was_input_file = false;
    bool was_output_file = false;

    for (int i = 1; i < argc && (a = argv[i]); ++i) {
        if (a[0] == '-') {
            if (!strcmp(a, "-E")) {
                // only preprocessing
                err_msg("-E call for cpp must be local");
                return false;
            } else if (a[1] == 'M') {
                if (!strcmp(a, "-MD") || !strcmp(a, "-MMD")) {
                    // generates dependencies as a side effect of
                    // the compilation process
                    was_driver = true;
                }
                else if (!strncmp(a, "-MT", 3) ||
                         !strncmp(a, "-MQ", 3) ||
                         !strncmp(a, "-MF", 3)) {
                    // seems like in distcc they got this not quite right
                    // as {-MT,-MQ,-MF}target works correct as well,
                    // and {-MT, -MQ, -MF} can't be without extra argument,
                    // otherwise it's an error
                    if (a[3] == '\0') ++i;
                }
                was_make_rule = true;
            } else if (!strcmp(a, "-march=native")) {
                // proceed with local build, just quit for now
                err_msg("-march=native generates code for local machine, must be local");
                return false;
            } else if (!strcmp(a, "-mtune=native")) {
                // proceed with local build, just quit for now
                err_msg("-mtune=native optimizes for local machine, must be local");
                return false;
            } else if (!strcmp(a, "-c")) {
                was_opt_c = true;
            } else if (!strcmp(a, "-o")) {
                if (was_output_file)
                    err_quit("got more than one output file\n");
                ++i;
            }
        } else {
            const char *dot, *ext = NULL;
            dot = strrchr(a, '.');
            if (dot)
                ext = dot + 1;
            else
                err_quit("wrong argument: %s\n", a);

            if (!isSource(ext))
                err_quit("this doesn't look like source file: %s\n", a);

            was_input_file = true;
        }
    }

    // there are additional check in distcc, if we have -S and -c it imply
    // that compiler was called not to compile, but WHY ? isn't we just want
    // to produce .s and .o files and left it so for the linking stage ?
    
    if (!was_input_file)
        err_quit("input file missed\n");

    if (was_make_rule && !was_driver)
        return false;
    
    return true;
}

static void mangle_argv_for_cpp(char* argv[])
{
    for (char** it = argv; *it; ++it)
    {
        char* argp = *it;

        if (argp[0] == '-' && argp[1] != '\0')
        {
            if (argp[1] == 'c' && argp[2] == '\0')
                argp[1] = 'E'; // change to -E for preprocessing
            // -o indicates output file
            else if (argp[1] == 'o')
            {
                // force stdout output
                if (argp[2] != '\0') // inline parameter
                {
                    argp[2] = '-';
                    argp[3] = '\0';
                }
                else // split parameter
                {
                    argp = *++it;
                    argp[0] = '-';
                    argp[1] = '\0';
                }
            }
        }
    }
}

static pid_t run_preprocessor(char** argv, int* output_fg)
{
    int cpp_pipes[2];

    Unix::Pipe(cpp_pipes[2]);
    pid_t pid = Unix::Fork();

    if (pid == 0) // subprocess
    {
        Unix::Dup2(cpp_pipes[1], 1);
        Unix::Close(cpp_pipes[0]);
        Unix::Close(cpp_pipes[1]);

        // since we forked, we can mangle argv in-place
        mangle_argv_for_cpp(&argv[1]);

#ifndef NDEBUG
        std::cout << "Mangled argv\n";
        for (char** it = argv; *it; ++it) {
            std::cout << *it << std::endl;
        }
#endif

        if (execvp(argv[0], argv)) {
            std::cerr << "d2cc: Unable to spawn preprocessor "
                << ": " << strerror(errno) << std::endl;
            exit(1);
        }
    }

    Unix::Close(cpp_pipes[1]);
    *output_fd = cpp_pipes[0];

    return pid;
}

static bool runRemotely(int argc, char** argv)
{
    int output_fd;
    pid_t cpp_pid = run_preprocessor(argv, &output_fd);
    if (cpp_pid == -1)
        return false;

    int ret;
    if (waitpid(cpp_pid, &ret, 0) == -1)
    {
        std::cerr << "d2cc: Unable to reap preprocessor: "
            << strerror(errno) << std::endl;
        return false;
    }

    return false;
}

int main(int argc, char** argv)
{
// assume here that standart compiler is in PATH
    std::string compiler = basename(argv[0]);
    if      (compiler == "d2cc-gcc")     compiler = "gcc";
    else if (compiler == "d2cc-g++")     compiler = "g++";
    else if (compiler == "d2cc-clang")   compiler = "clang";
    else if (compiler == "d2cc-clang++") compiler = "clang++";
    else err_quit(compiler, " isn't supported by d2cc");

    if (parseCommandArgs(argc, argv)) {
#ifndef NDEBUG
        err_msg("Running remotely\n");
#endif
        if (runRemotely(argc, argv)) 
            return 0;
    }
#ifndef NDEBUG
    err_msg("Not possible to run remotely, run locally instead\n");
#endif
    strcpy(argv[0], compiler.str());
    if (!execvp(argv[0], argv)) {
        err_quit("Unable to run locally ", compiler, " .Exiting...");
    }
}
