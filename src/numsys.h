#ifndef LADLE_NUMSYS_H
#define LADLE_NUMSYS_H

#include <ladle/common/header.h>

// Information pertaining to a number's string representation
typedef struct numinfo_t {
    unsigned min;   // Minimum # of digits
    unsigned space; // # of digits between spaces, 0 for no spaces
} numinfo_t;

// Determines representation of number strings
typedef enum numrep_t {
    NR_NEGSGN = 1, // Append negative sign to front
    NR_SPLACE = 2, // Append nonzero digit to front
    NR_1COMPL = 4, // Set as complement of positive
    NR_2COMPL = 8  // Set as complement of positive plus 1
} numrep_t;

// Represents a unique number system
typedef struct numsys_t {
    unsigned base;
    numrep_t rep;
} numsys_t;

BEGIN

/* Converts number string of number system 'src' to equivalent string of system 'dest'
 * Returns conversion as malloc'd number string
 * Returns NULL and sets errno accordingly on error
 * Base-1 negatives will hold the value of their absolute value
 *
 * Error Code    Cause
 *  EINVAL        Null string or an invalid system base or notation
 *  EOVERFLOW     Conversion causes integer overflow
 *  ERANGE        Number string cannot be represented in 'dest' form
 *  (else)        Internal error */
export char *nsys_conv(const char *numstr, numsys_t src, numsys_t dest, numinfo_t info) nonnull noexcept;
export char *nsys_uconv(const char *numstr, unsigned src, unsigned dest, numinfo_t info) nonnull noexcept;

/* Returns value of number string according to given number system
 * Returns 0 and sets errno accordingly on error
 *
 * Error Code    Cause
 *  EINVAL        Null string or an invalid system base or notation
 *  EOVERFLOW     Conversion causes integer overflow
 *  (else)        Internal error */
export long long nsys_tonum(const char *numstr, numsys_t sys) nonnull noexcept pure;
export unsigned long long nsys_utonum(const char *numstr, unsigned base) nonnull noexcept pure;

/* Returns malloc'd number string of value according to given system
 * Returns NULL and sets errno accordingly on error
 * Base-1 negatives will hold the value of their absolute value
 *
 * Error Code    Cause
 *  EINVAL        Null string or an invalid system base or notation
 *  ERANGE        Number cannot be represented in string form
 *  (else)        Internal error */
export char *nsys_tostr(long long num, numsys_t sys, numinfo_t info) noexcept;
export char *nsys_utostr(unsigned long long num, unsigned base, numinfo_t info) noexcept;

END

#include <ladle/common/end_header.h>
#endif  // #ifndef LADLE_NUMSYS_H
