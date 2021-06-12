#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "../src/numsys.h"

#define loop(body)	for (int i = 0; i < 38; ++i) body

#define tonum_test(_sys, _base, _notat) {				\
errno = 0; _sys.base = _base; _sys.notat = _notat;		\
printf("base = %i\n"									\
	   "notation = %d\n"								\
	   "errno = %d\n"									\
	   "result = %lld\n\n",								\
	_base, _notat, errno, numsys_tonum(argv[1], _sys));	\
}

int main(int argc, char *argv[]) {
	numsys_t sys;

	query();
	loop(tonum_test(sys, i, NEG_SIGN));
	loop(tonum_test(sys, i, SIGN_BIT));
	loop(tonum_test(sys, i, ONES_COMPL));
	loop(tonum_test(sys, i, TWOS_COMPL));

	return EXIT_SUCCESS;
}