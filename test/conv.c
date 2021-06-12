#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "../src/numsys.h"

#define loop(body)    for (int i = 0, j = 37; i < 38; ++i, --j) body

#define toconv_test(_src, _src_base, _src_notat, _dest, _dest_base, _dest_notat) {  \
errno = 0; _src.base = _src_base; _src.notat = _src_notat;                          \
_dest.base = _dest_base; _dest.notat = _dest_notat;                                 \
printf("src base = %i\n"                                                            \
       "src notation = %d\n"                                                        \
       "dest base = %i\n"                                                           \
       "dest notation = %d\n"                                                       \
       "errno = %d\n"                                                               \
       "result = %s\n\n",                                                           \
    _src_base, _src_notat, _dest_base, _dest_notat,                                 \
    errno, numsys_conv(argv[1], _src, _dest));                                      \
}

int main(int argc, char *argv[]) {
    numsys_t src, dest;

    query();
    loop(toconv_test(src, i, NEG_SIGN, dest, j, NEG_SIGN));
    loop(toconv_test(src, i, NEG_SIGN, dest, j, SIGN_BIT));
    query();
    loop(toconv_test(src, i, NEG_SIGN, dest, j, ONES_COMPL));
    loop(toconv_test(src, i, NEG_SIGN, dest, j, TWOS_COMPL));
    query();
    loop(toconv_test(src, i, SIGN_BIT, dest, j, NEG_SIGN));
    loop(toconv_test(src, i, SIGN_BIT, dest, j, SIGN_BIT));
    query();
    loop(toconv_test(src, i, SIGN_BIT, dest, j, ONES_COMPL));
    loop(toconv_test(src, i, SIGN_BIT, dest, j, TWOS_COMPL));
    query();
    loop(toconv_test(src, i, ONES_COMPL, dest, j, NEG_SIGN));
    loop(toconv_test(src, i, ONES_COMPL, dest, j, SIGN_BIT));
    query();
    loop(toconv_test(src, i, ONES_COMPL, dest, j, ONES_COMPL));
    loop(toconv_test(src, i, ONES_COMPL, dest, j, TWOS_COMPL));
    query();
    loop(toconv_test(src, i, TWOS_COMPL, dest, j, NEG_SIGN));
    loop(toconv_test(src, i, TWOS_COMPL, dest, j, SIGN_BIT));
    query();
    loop(toconv_test(src, i, TWOS_COMPL, dest, j, ONES_COMPL));
    loop(toconv_test(src, i, TWOS_COMPL, dest, j, TWOS_COMPL));
    return EXIT_SUCCESS;
}
