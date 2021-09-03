#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>

#include <ladle/collect.h>
#include <ladle/numsys.h>

static bool is_num(const char *);
static noreturn void print_help();
static noreturn void terminate(const char *, ...);

int main(int argc, char *argv[]) {
    coll_einit(EXIT_FAILURE, int, main, argc, argv);

    if (argc < 2)
        terminate(
          "numsys: Invalid # of arguments\n"
          "Try 'numsys -h' for more information\n"
        );

    char *sbuf;
    numsys_t src  = {10, NR_NEGSGN}, dest = src;
    numinfo_t info = {0};
    int eq_pos;
    unsigned base_count = 0, rep_count = 0;
    bool is_unsigned = false;

    for (size_t i = 1; i < argc; ++i) {    // Get arguments
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help"))
            print_help();
        if (!strcmp(argv[i], "-u") || !strcmp(argv[i], "--unsigned")) {
            is_unsigned = true;
            continue;
        } else if (!strcmp(argv[i], "-s") || !strcmp(argv[i], "--spaces")) {
            if (!is_num(argv[++i]))
                terminate("numsys: Expected an integer, but received `%s\'", argv[i]);
            info.space = atoi(argv[i]);
            continue;
        } else if (!strcmp(argv[i], "-m") || !strcmp(argv[i], "--maximum")) {
            if (!is_num(argv[++i]))
                terminate("numsys: Expected an integer, but received `%s\'", argv[i]);
            info.min = atoi(argv[i]);
            continue;
        }
        if (i >= argc - 1)
            break;

        /* -1 = Dot preceeds argument
         *  1 = Dot proceeds argument
         *  0 = neither */
        eq_pos = (argv[i][strlen(argv[i]) - 1] == '=') - (*(argv[i]) == '=');

        for (int j = 0;; ++j) {
            sbuf = strtok(j == 0 ? argv[i] : NULL, "=");
            if (!sbuf) {
                if (j != 2 && !(eq_pos && j == 1))
                    terminate("numsys: Missing conversion argument\n");
                break;
            }
            if (j == 2)
                terminate("numsys: Invalid conversion argument token: `%s\'\n", sbuf);
            if (is_num(sbuf)) {
                if (base_count == 2)
                    terminate("numsys: Conversion argument of duplicate type\n");
                if (!j && eq_pos >= 0)
                    src.base = atoi(sbuf);
                else
                     dest.base = atoi(sbuf);
                ++base_count;
            } else {
                if (rep_count == 2)
                    terminate("numsys: Conversion argument of duplicate type\n");
                if      (!strcmp(sbuf, "ns")) { if (!j && eq_pos >= 0) src.rep = NR_NEGSGN; else dest.rep = NR_NEGSGN; }
                else if (!strcmp(sbuf, "sp")) { if (!j && eq_pos >= 0) src.rep = NR_SPLACE; else dest.rep = NR_SPLACE; }
                else if (!strcmp(sbuf, "1c")) { if (!j && eq_pos >= 0) src.rep = NR_1COMPL; else dest.rep = NR_1COMPL; }
                else if (!strcmp(sbuf, "2c")) { if (!j && eq_pos >= 0) src.rep = NR_2COMPL; else dest.rep = NR_2COMPL; }
                else
                    terminate("numsys: Invalid conversion argument token: `%s\'\n",
                      sbuf);
                
                ++rep_count;
            }
        }
    }

    sbuf = is_unsigned ?
        nsys_uconv(argv[argc - 1], src.base, dest.base, info) :
        nsys_conv(argv[argc - 1], src, dest, info);
    if (errno > 0)
        terminate("numsys: %s\n", strerror(errno));
    puts(sbuf);
    return EXIT_SUCCESS;
}

bool is_num(const char *s) {
    bool is_number = false;
    char cbuf;
    for (size_t i = 0; cbuf = s[i]; ++i) {
        if (isdigit(cbuf))
            is_number = true;
        else if (!isblank(cbuf))
            return false;
    }
    return is_number;
}
noreturn void print_help() {
    puts(
        "Usage: numsys [OPTION] [[BASE]=[BASE]] [[REP]=[REP]] [INPUT]\n"
        "Converts number system or representation\n\n"

        "-h    --help        Prints the help page\n"
        "-u    --unsigned    Use unsigned arithmetic, larger maximum value\n"
        "-s    --spaces      # of characters between spaces, 0 for none\n"
        "-m    --minimum     Minimum # of digits, 0 for absolute minimum\n\n"

        "To change from one number system to another, pass two numbers seperated by an\n"
        "equals sign.\n\n"
    
        "\"10=2\"\n"
        " |  |_ Input is in base-10\n"
        " |____ Output will be printed in base-2\n\n"

        "Similarly, to change from one negative representation to another, pass any two\n"
        "of the four equivalent tokens.\n\n"

        "\"ns=sp\"\n"
        " |  |_ Negative input is denoted by a negative sign\n"
        " |__________ Negative output will be denoted using a non-zero sign place\n\n"

        "By default, input and output are in base-10 negative sign representation. To\n"
        "avoid clashes with terminal operators, it is recommended that arguments\n"
        "affecting conversion be encapsulated within parentheses.\n\n"

        "Representation    Token    Example\n\n"

        "Negative sign     ns       8, -12\n"
        "Sign place        sp       08, 112\n"
        "One's complement  1c       91, 187\n"
        "Two's complement  2c       92, 188\n\n"

        "To use or display numbers larger than the signed maximum, pass the `-u' or\n"
        "`--unsigned' flags to use unsigned arithmetic instead.\n\n"

        "GitHub repository: https://github.com/ladle-gh/libnumsys\n"
        "Report bugs to <ladle-gh@protonmail.com>"
    );
    exit(EXIT_SUCCESS);
}
noreturn void terminate(const char *fmt, ...) {
    va_list args;

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    exit(EXIT_FAILURE);
}
