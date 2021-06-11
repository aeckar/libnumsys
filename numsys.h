#ifndef CONVERT_H
#define CONVERT_H

#include "attribute.h"

// Determines representation of number strings
typedef enum notation_t {
	NEG_SIGN   = 1,	// Append negative sign to front
	SIGN_BIT   = 2,	// Append nonzero digit to front
	ONES_COMPL = 4,	// Set as complement of positive
	TWOS_COMPL = 8	// Set as complement of positive plus 1
} notation_t;

// Represents a unique number system
typedef struct numsys_t {
	unsigned base;
	notation_t notat;
} numsys_t;

/* Returns value of number string according to given number system
 * Returns 0 and sets errno to EINVAL or EOVERFLOW on respective error */
extern long long numsys_tonum(const char *NUMSTR, numsys_t sys)
glibc_attribute(nonnull, nothrow, warn_unused_result);

/* Converts number string of number system 'src' to equivalent string of system 'dest'
 * Returns conversion as malloc'd number string */
extern char *numsys_conv(const char *NUMSTR, numsys_t src, numsys_t dest)
glibc_attribute(nonnull, nothrow, warn_unused_result);

/* Returns malloc'd number string of value according to given system
 * Returns NULL and sets errno to EINVAL on error */
extern char *numsys_tostring(long long num, numsys_t sys)
glibc_attribute(nothrow, warn_unused_result);

#undef glibc_attribute
#undef ATTRIBUTE_H
#endif
