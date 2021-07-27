#ifndef NUMSYS_H
#define NUMSYS_H

// Ensures attribute portability
#ifdef __GNUC__
    #define attribute(...)  __attribute__((__VA_ARGS__))
#else
    #define attribute(...)
#endif  // glibc || libstdc++
#ifdef _MSC_VER
    #define declspec(...)   __declspec(__VA_ARGS__)
#else
    #define declspec(...)
#endif  // MSVC

// Determines representation of number strings
typedef enum numrep_t {
    NEG_SIGN   = 1, // Append negative sign to front
    SIGN_PLACE = 2, // Append nonzero digit to front
    ONES_COMPL = 4, // Set as complement of positive
    TWOS_COMPL = 8  // Set as complement of positive plus 1
} numrep_t;

// Represents a unique number system
typedef struct numsys_t {
    unsigned base;
    numrep_t rep;
} numsys_t;

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
extern char *declspec(nothrow, restrict)
numsys_conv(const char *numstr, numsys_t src, numsys_t dest)
attribute(nonnull, nothrow, warn_unused_result);

extern char *declspec(nothrow, restrict)
numsys_uconv(const char *numstr, unsigned src, unsigned dest)
attribute(nonnull, nothrow, warn_unused_result);

/* Returns value of number string according to given number system
 * Returns 0 and sets errno accordingly on error
 *
 * Error Code    Cause
 *  EINVAL        Null string or an invalid system base or notation
 *  EOVERFLOW     Conversion causes integer overflow
 *  (else)        Internal error */
extern long long declspec(nothrow)
numsys_tonum(const char *numstr, numsys_t sys)
attribute(nonnull, nothrow, pure);

extern unsigned long long declspec(nothrow)
numsys_utonum(const char *numstr, unsigned base)
attribute(nonnull, nothrow, pure);

/* Returns malloc'd number string of value according to given system
 * Returns NULL and sets errno accordingly on error
 * Base-1 negatives will hold the value of their absolute value
 *
 * Error Code    Cause
 *  EINVAL        Null string or an invalid system base or notation
 *  ERANGE        Number cannot be represented in string form
 *  (else)        Internal error */
extern char *declspec(nothrow, restrict)
numsys_tostring(long long num, numsys_t sys)
attribute(nothrow, warn_unused_result);

extern char *declspec(nothrow, restrict)
numsys_utostring(unsigned long long num, unsigned base)
attribute(nothrow, warn_unused_result);

#undef attribute
#endif  // #ifndef NUMSYS_H
