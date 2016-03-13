#include "wrap.h"

static bool isSource(const char* ext)
{

    switch (ext[0]) {
    case 'i':
        return !strcmp(ext, "i")
            || !strcmp(ext, "ii");
    case 'c':
        return !strcmp(ext, "c")
            || !strcmp(ext, "cc")
            || !strcmp(ext, "cpp")
            || !strcmp(ext, "cxx")
            || !strcmp(ext, "cp")
            || !strcmp(ext, "c++");
    case 'C':
        return !strcmp(ext, "C");
    case 'm':
        return !strcmp(ext,"m")
            || !strcmp(ext,"mm")
            || !strcmp(ext,"mi")
            || !strcmp(ext,"mii");
    case 'M':
        return !strcmp(ext, "M");
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

static bool runRemotely(int argc, char** argv)
{
}

int main(int argc, char** argv)
{
    const char* compiler = basename(argv[0]);
    if      (!strcmp(compiler, "d2cc-gcc"))     compiler = "gcc";
    else if (!strcmp(compiler, "d2cc-g++"))     compiler = "g++";
    else if (!strcmp(compiler, "d2cc-clang"))   compiler = "clang";
    else if (!strcmp(compiler, "d2cc-clang++")) compiler = "clang++";
    else err_quit(compiler, " isn't supported by d2cc");

    if (parseCommandArgs(argc, argv)) {
#ifndef NDEBUG
        err_msg("Running remotely\n");
#endif
        if (!runRemotely(argc, argv)) {
            err_quit("Error running remotely\n");
        }
    } else {
#ifndef NDEBUG
        err_msg("Not possible to run remotely, run locally instead\n");
#endif
        if (!execvp(compiler, &argv[1])) {
            err_quit("Unable to run ", compiler, " .Exiting...");
        }
    }
}
