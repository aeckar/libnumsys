#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "numsys.h"

// Allows for null check in spite of 'nonnull' attribute
#pragma GCC diagnostic ignored  "-Wnonnull-compare"

// ---- Macros ----

/* Takes:   char
 * Returns: unsigned
 *
 * Returns int equivalent of char digit
 * Causes side-effects */
#define digit_to_num(digit)                     \
    (isdigit(digit) ?                           \
      (digit) - '0'  :                          \
      ((digit) > 'Z' ?                          \
        (digit) + 10 - 'a' :    /* Uppercase */ \
        (digit) + 10 - 'A'))    /* Lowercase */ \

/* Error handler
 * For use within an if-statement
 * Preserves original cause of error */
#define error(err, ret) {   \
    if (!errno)             \
        errno = err;        \
    return ret;             \
}

/* Takes:   unsigned
 * Returns: bool
 *
 * Returns true if base is invalid
 * Causes side-effects */
#define inval_base(base)    (!(base) || (base) > 36)

/* Takes:   numrep_t
 * Returns: bool
 *
 * Returns true if representation is invalid */
#define inval_rep(rep)      (!((rep) & (NEG_SIGN|SIGN_PLACE|ONES_COMPL|TWOS_COMPL)))

/* Takes:   numinfo_t, unsigned
 * Returns: bool
 *
 * Returns true if number string information is invalid
 * Causes side-effects */
#define inval_info(info, base)                  \
    ((info).min > sizeof(long long) * 8 ||      \
      (info).space > sizeof(long long) * 8 ||   \
      ((info).space && base == 1))              \

#define max(x, y)   ((x) > (y) ? (x) : (y))

/* Takes:   unsigned
 * Returns: char
 *
 * Returns digit of maximum value for given base
 * Causes side-effects */
#define max_digit(base)     ((base) <= 10 ? (base) + '0' - 1 : (base) + 'A' - 11)

/* Takes:   unsigned
 * Returns: unsigned
 *
 * Returns number of spaces present in number string
 * Causes side-effects */
#define nspaces(min, space) ((space) ? (((min) ? (min) : 1) - 1) / (space) : 0)

// ---- Constants ----

// String of standard whitespace characters and underscore
#define IGNORE      "\t\n\v\f\r _"
#define IGNORE_LEN  7

// String constant from which new validity strings are created 
static const char *DIGITS = "0123456789"
                            "aAbBcCdDeEfFgGhHiIjJkKlLm"
                            "MnNoOpPqQrRsStTuUvVwWxXyYzZ";

// ---- Static Functions ----

// For negative numbers, returns index of sign bit or negative sign
static size_t locate_sign(const char *restrict numstr) {
    for (size_t i = 0;; ++i) {
        if (!strchr(IGNORE, numstr[i]))
            return i;
    }
}

/* Returns number of digits in equivalent number string
 * Returns 0 and sets errno to ERANGE on error */
static unsigned ndigits(unsigned long long num, unsigned base) {
    unsigned count = 0;

    if (base == 1) {
        if (num > UINT_MAX) // Can't print more than UINT_MAX '0's
            error(ERANGE, 0)
        return num;
    }
    do {
        num /= base;
        ++count;
    } while (num);
    return count;
}

// Fast exponentiation algorithm
static unsigned long long ullpow(unsigned long long base, unsigned long long pow){
    unsigned long long result = 1;

    while (pow) {
        if (pow & 1)
            result *= base;
        pow >>= 1;
        base *= base;
    }
    return result;
}

/* Returns string of valid characters in number string of given system
 * Returns NULL on error */ 
static char *valid_chrs(numsys_t sys) {
    const bool neg_is_valid = sys.rep == NEG_SIGN;
    const size_t memsize = IGNORE_LEN + sys.base +
      (sys.base > 10 ? sys.base - 10 : 0) + neg_is_valid + 1;
    char *const valid = malloc(memsize * sizeof(char));

    if (!valid)    // malloc() fails
        return NULL;

    size_t digit_index = 0, i;

    memcpy(valid, IGNORE, IGNORE_LEN);
    if (neg_is_valid)
        valid[IGNORE_LEN] = '-';
    for (i = IGNORE_LEN + neg_is_valid; i < memsize - 1; ++i) {
        valid[i] = DIGITS[digit_index++];
        if (i >= IGNORE_LEN - 1 + 10)
            valid[++i] = DIGITS[digit_index++];
    }
    valid[i] = '\0';
    return valid;
}

// ---- Non-Static Functions ----

char *numsys_conv(const char *numstr, numsys_t src, numsys_t dest, numinfo_t info) {
    const long long tmp = numsys_tonum(numstr, src);

    if (errno > 0)
        return NULL;
    return numsys_tostring(tmp, dest, info);
}
long long numsys_tonum(const char *numstr, numsys_t sys) {
    if (!numstr || inval_base(sys.base) || inval_rep(sys.rep))
        error(EINVAL, 0);

    char *const valid = valid_chrs(sys);

    if (!valid)    // valid_chrs() fails
        return 0;

    const size_t sign_index = locate_sign(numstr);
    const bool is_signed =
      (sys.rep != NEG_SIGN && numstr[sign_index] != '0') || numstr[sign_index] == '-';
    char cur;
    unsigned digit_val;
    long long to_add, place_val = 1, result = 0;

    if (sys.rep == TWOS_COMPL && is_signed)
        ++result;
    for (size_t i = strlen(numstr) - 1;; --i) {
        cur = numstr[i];
        if (!strchr(valid, cur)) { // Found invalid character
            free(valid);
            error(EINVAL, 0);
        }
        if (cur == '-' && (sys.rep != NEG_SIGN || i != sign_index)) {
            free(valid);
            error(EINVAL, 0);
        } else if (!strchr(IGNORE, cur) && cur != '-') {
            digit_val = sys.rep & (ONES_COMPL|TWOS_COMPL) && is_signed ?
              sys.base - (sys.base != 1) - digit_to_num(cur) : digit_to_num(cur);
            if (digit_val && place_val > LLONG_MAX / digit_val) {
                free(valid);
                error(EOVERFLOW, 0);    // Overflow check for getting addition
            }
            to_add = digit_val * place_val;
            if (result > LLONG_MAX - to_add) {
                free(valid);
                error(EOVERFLOW, 0);    // Overflow check for getting result
            }
            result += to_add;
            place_val *= sys.base;
        }
        if (i == (sys.rep != NEG_SIGN))
            break;
    }
    if (is_signed) {
        if (result == LLONG_MIN) {
            free(valid);
            error(EOVERFLOW, 0);
        }
        result = -result;
    }
    free(valid);
    return result;
}
char *numsys_tostring(long long num, numsys_t sys, numinfo_t info) {
    if (inval_base(sys.base) || inval_rep(sys.rep) || inval_info(info, sys.base))
        error(EINVAL, NULL);

    unsigned nchrs = ndigits(llabs(num), sys.base);

    if (errno > 0) // ndigits() fails
        return NULL;


    const bool is_signed = num < 0;
    const bool has_sign_place = ((sys.rep == NEG_SIGN && num < 0) || sys.rep != NEG_SIGN)
      && sys.base != 1  // Not representable in base-1
      && num != 0       // Sign place not needed
      && !(num == LLONG_MIN && sys.base == 2);  // Fixes extra sign place bug
    const size_t total = max(info.min, nchrs);  // Total # of digits
    const size_t memsize = total + nspaces(total, info.space) + has_sign_place + 1;
    char *const result = malloc(memsize * sizeof(char));

    if (!result)   // malloc() fails
        return NULL;

    unsigned place = 0;
    unsigned long long digit_val;
    const char max = max_digit(sys.base);
    const long long num_abs = llabs(num);

    result[memsize - 1] = '\0';
    for (size_t i = memsize - 2;; --i) {
        if (nchrs) {
            digit_val = num_abs;
            if (has_sign_place && is_signed && sys.rep == TWOS_COMPL)
                digit_val -= 1;
            digit_val /= ullpow(sys.base, place++);         // Shift right to desired digit
            digit_val -= digit_val / sys.base * sys.base;   // Subtract leading digits
            if (has_sign_place && is_signed && sys.rep & (ONES_COMPL|TWOS_COMPL))
                digit_val = sys.base - digit_val - 1;       // Get complement
            result[i] = digit_val +                         // Get character representation
              (digit_val < 10 ? '0' : 'A' - 10);
            --nchrs;
        } else
            result[i] = is_signed ? max : '0';
        if (i == has_sign_place)
            break;
        if (info.space && !(place % info.space))
            result[--i] = ' ';
    }
    if (has_sign_place) {
        if (is_signed)
            result[0] = sys.rep == NEG_SIGN ? '-' : max;
        else
            result[0] = '0';
    }
    return result;
}
char *numsys_uconv(const char *numstr, unsigned src, unsigned dest, numinfo_t info) {
    const unsigned long long tmp = numsys_utonum(numstr, src);

    if (errno > 0)
        return NULL;
    return numsys_utostring(tmp, dest, info);
}
unsigned long long numsys_utonum(const char *numstr, unsigned base) {
    if (!numstr || inval_base(base)) {
        errno = EINVAL;
        return 0;
    }

    char *const valid = valid_chrs((numsys_t) {base, SIGN_PLACE});

    if (!valid)
        return 0;

    char cur;
    unsigned digit_val;
    unsigned long long to_add, place_val = 1, result = 0;

    for (size_t i = strlen(numstr) - 1;; --i) {
        cur = numstr[i];
        if (!strchr(valid, cur)) {
            free(valid);
            error(EINVAL, 0);
        } else if (!strchr(IGNORE, cur) && cur != '-') {
            digit_val = digit_to_num(cur);
            if (digit_val && place_val > ULLONG_MAX / digit_val) {
                free(valid);
                error(EOVERFLOW, 0);
            }
            to_add = digit_val * place_val;
            if (result > ULLONG_MAX - to_add) {
                free(valid);
                error(EOVERFLOW, 0);
            }
            result += to_add;
            place_val *= base;
        }
        if (!i)
            break;
    }
    free(valid);
    return result;
}
char *numsys_utostring(unsigned long long num, unsigned base, numinfo_t info) {
    if (inval_base(base) || inval_info(info, base))
        error(EINVAL, NULL);

    unsigned nchrs = ndigits(num, base);

    if (errno > 0)
        return NULL;
    
    const size_t total = max(info.min, nchrs);  // Total # of digits
    const size_t memsize = total + nspaces(total, info.space) + 1;
    char *const result = malloc(memsize * sizeof(char));

    if (!result)
        return NULL;

    unsigned place = 0;
    unsigned long long digit_val;

    result[memsize - 1] = '\0';
    for (size_t i = memsize - 2;; --i) {
        if (nchrs) {
            digit_val = num / ullpow(base, place++);
            digit_val -= digit_val / base * base;
            result[i] = digit_val + (digit_val < 10 ? '0' : 'A' - 10);
            --nchrs;
        } else
            result[i] = '0';
        if (!i)
            break;
        if (info.space && !(place % info.space))
            result[--i] = ' ';
    }
    return result;
}
