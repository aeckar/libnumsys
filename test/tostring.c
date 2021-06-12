#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "../src/numsys.h"

#define loop(body)    for (int i = 0; i < 38; ++i) body

#define tostring_test(_sys, _base, _notat) {                        \
errno = 0; _sys.base = _base; _sys.notat = _notat;                  \
printf("base = %i\n"                                                \
       "notation = %d\n"                                            \
       "errno = %d\n"                                               \
       "result = %s\n\n",                                           \
    _base, _notat, errno, numsys_tostring(atoll(argv[1]), _sys));   \
}

int main(int argc, char *argv[]) {
    numsys_t sys;

    query();
    loop(tostring_test(sys, i, NEG_SIGN));
    loop(tostring_test(sys, i, SIGN_BIT));
    loop(tostring_test(sys, i, ONES_COMPL));
    loop(tostring_test(sys, i, TWOS_COMPL));
    return EXIT_SUCCESS;
}
