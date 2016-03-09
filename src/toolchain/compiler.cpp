#include "wrap.h"

bool parseCommandArgs(int argc, char** argv);
bool isSource(const char* sfile);

int main(int argc, char** argv)
{
    parseCommandArgs(argc, argv);
    const char* compiler = basename(argv[0]);
}

bool isSource(const char* sfile)
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

bool parseCommandArgs(int argc, char** argv)
{
#ifndef NDEBUG
    std::cout << "gcc argv:\n";
    for (int i = 0; i < argc; ++i)
        std::cout << "argv[" << i << "] : " << argv[i] << std::endl; 
#endif

    const char *infile = NULL, *outfile = NULL;

    char* a;
    bool was_driver = false, was_make_rule = false;
    bool was_opt_c = false;

    for (int i = 1; i < argc && (a = argv[i]); ++i) {
        if (a[0] == '-') {
            if (!strcmp(a, "-E")) {
                err_quit("-E call for cpp must be local");
            } else if (a[1] == "M") {
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
                err_quit("-march=native generates code for local machine, must be local");
            } else if (!strcmp(a, "-mtune=native")) {
                // proceed with local build, just quit for now
                err_quit("-mtune=native optimizes for local machine, must be local");
            } else if (!strcmp(a, "-c")) {
                was_opt_c = true;
            } else if (!strcmp(a, "-o")) {
                if (outfile) err_quit("got more than one output file\n");
                outfile = argv[++i];
            }
        } else {
            const char* dot, ext;
            dot = strrchr('.')
            if (dot) ext = dot + 1;
            else err_quit("wrong argument: %s\n", a);
            if (d2cc_is_source(ext)) {
                if (infile) {
                    err_quit("got more than one input file\n");
                }
                infile = a;
            } else {
                err_quit("wrong argument: %s\n", a);
            } 
        }
    }

    // there are additional check in distcc, if we have -S and -c it imply
    // that compiler was called not to compile, but WHY ? isn't we just want
    // to produce .s and .o files and left it so for the linking stage ?
    
    if (!infile) err_quit("input file missed\n");
    
    return 0;
}
