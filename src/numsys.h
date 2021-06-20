#ifndef NUMSYS_H
#define NUMSYS_H

#ifdef __GNUC__
#define gcc_attribute(...)  __attribute__((__VA_ARGS__))
#elif defined(_MSC_VER)
#define msvc_attribute(...)  __declspec(__VA_ARGS__)
#endif

#ifndef __GNUC__
#define gcc_attribute(...)
#elif !defined(_MSC_VER)
#define msvc_attribute(...)
#endif

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

/* Returns value of number string according to given number system
 * Returns 0 and sets errno accordingly on error
 *
 * Error Code    Cause
 *  EINVAL        Null string or an invalid system base or notation
 *  ENOMEM        Out of memory 
 *  EOVERFLOW     Conversion causes integer overflow */
extern long long numsys_tonum(const char *NUMSTR, numsys_t sys)
gcc_attribute(nonnull, nothrow)
msvc_attribute(nothrow);

extern unsigned long long numsys_tounum(const char *NUMSTR, unsigned base)
gcc_attribute(nonnull, nothrow)
msvc_attribute(nothrow);

/* Converts number string of number system 'src' to equivalent string of system 'dest'
 * Returns conversion as malloc'd number string
 * Returns NULL and sets errno accordingly on error
 * Base-1 negatives will hold the value of their absolute value
 *
 * Error Code    Cause
 *  EINVAL        Null string or an invalid system base or notation
 *  ENOMEM        Out of memory 
 *  EOVERFLOW     Conversion causes integer overflow
 *  ERANGE        Number string cannot be represented in 'dest' form */
extern char *numsys_conv(const char *NUMSTR, numsys_t src, numsys_t dest)
gcc_attribute(nonnull, nothrow, warn_unused_result)
msvc_attribute(nothrow);

extern char *numsys_convu(const char *NUMSTR, unsigned src, unsigned dest)
gcc_attribute(nonnull, nothrow, warn_unused_result)
msvc_attribute(nothrow);

/* Returns malloc'd number string of value according to given system
 * Returns NULL and sets errno accordingly on error
 * Base-1 negatives will hold the value of their absolute value
 *
 * Error Code    Cause
 *  EINVAL        Null string or an invalid system base or notation
 *  ENOMEM        Out of memory
 *  ERANGE        Number cannot be represented in string form */
extern char *numsys_tostring(long long num, numsys_t sys)
gcc_attribute(nothrow, warn_unused_result)
msvc_attribute(nothrow);

extern char *numsys_toustring(unsigned long long num, unsigned base)
gcc_attribute(nothrow, warn_unused_result)
msvc_attribute(nothrow);

#undef attribute
#endif
