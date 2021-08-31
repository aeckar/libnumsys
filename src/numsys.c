#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <ladle/common/impl.h>
#include <ladle/collect.h>
#include <ladle/common/ptrcmp.h>

#include "numsys.h"

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
#define inval_rep(rep)      (!((rep) & (NR_NEGSGN|NR_SPLACE|NR_1COMPL|NR_2COMPL)))

/* Takes:   numinfo_t, unsigned
 * Returns: bool
 *
 * Returns true if number string information is invalid
 * Causes side-effects */
#define inval_info(info, base)                  \
    ((info).min > sizeof(long long) * 8 ||      \
      (info).space > sizeof(long long) * 8 ||   \
      ((info).space && base == 1))              \

/* Takes:   T, T; where T is an integral type
 * Returns: T
 *
 * Returns maximum of two integrals
 * Causes side-effects */
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
    const bool neg_is_valid = sys.rep == NR_NEGSGN;
    const size_t memsize = IGNORE_LEN + sys.base +
      (sys.base > 10 ? sys.base - 10 : 0) + neg_is_valid + 1;
    char *const valid = coll_queue(malloc(memsize * sizeof(char)));

    if (!valid)    // malloc() fails
        return NULL;

    size_t digit_index = 0, i;

    memcpy(valid, IGNORE, IGNORE_LEN);
    if (neg_is_valid)
        valid[IGNORE_LEN] = '-';
    for (i = IGNORE_LEN + neg_is_valid; i < memsize - 1; ++i) {
        valid[i] = DIGITS[digit_index++];
        if (i >= IGNORE_LEN + 10)
            valid[++i] = DIGITS[digit_index++];
    }
    valid[i] = '\0';
    return valid;
}

// ---- Non-Static Functions ----

char *nsys_conv(const char *numstr, numsys_t src, numsys_t dest, numinfo_t info) {
    const long long tmp = nsys_tonum(numstr, src);

    if (errno)
        return NULL;
    return nsys_tostr(tmp, dest, info);
}
long long nsys_tonum(const char *numstr, numsys_t sys) {
    if (!numstr || inval_base(sys.base) || inval_rep(sys.rep))
        error(EINVAL, 0);

    coll_einit(long long, nsys_tonum, 0, numstr, sys);
    char *const valid = coll_queue(valid_chrs(sys));

    if (!valid)    // valid_chrs() fails
        return 0;

    const size_t sign_index = locate_sign(numstr);
    const bool is_signed =
      (sys.rep != NR_NEGSGN && numstr[sign_index] != '0') || numstr[sign_index] == '-';
    char cur;
    unsigned digit_val;
    long long to_add, place_val = 1, result = 0;

    if (sys.rep == NR_2COMPL && is_signed)
        ++result;
    for (size_t i = strlen(numstr) - 1;; --i) {
        cur = numstr[i];
        if (!strchr(valid, cur))    // Found invalid character
            error(EINVAL, 0);
        if (cur == '-' && (sys.rep != NR_NEGSGN || i != sign_index)) {
            error(EINVAL, 0);
        } else if (!strchr(IGNORE, cur) && cur != '-') {
            digit_val = sys.rep & (NR_1COMPL|NR_2COMPL) && is_signed ?
              sys.base - (sys.base != 1) - digit_to_num(cur) : digit_to_num(cur);
            if (digit_val && place_val > LLONG_MAX / digit_val)
                error(EOVERFLOW, 0);    // Overflow check for getting addition
            to_add = digit_val * place_val;
            if (result > LLONG_MAX - to_add)
                error(EOVERFLOW, 0);    // Overflow check for getting result
            result += to_add;
            place_val *= sys.base;
        }
        if (i == (sys.rep != NR_NEGSGN))
            break;
    }
    if (is_signed) {
        if (result == LLONG_MIN)
            error(EOVERFLOW, 0);
        result = -result;
    }
    return result;
}
char *nsys_tostr(long long num, numsys_t sys, numinfo_t info) {
    if (inval_base(sys.base) || inval_rep(sys.rep) || inval_info(info, sys.base))
        error(EINVAL, NULL);

    unsigned nchrs = ndigits(llabs(num), sys.base);

    if (errno)  // ndigits() fails
        return NULL;

    const bool is_signed = num < 0;
    const bool has_sign_place = ((sys.rep == NR_NEGSGN && num < 0) || sys.rep != NR_NEGSGN)
      && sys.base != 1  // Not representable in base-1
      && num != 0       // Sign place not needed
      && !(num == LLONG_MIN && sys.base == 2);  // Fixes extra sign place bug
    const size_t total = max(info.min, nchrs);  // Total # of digits
    const size_t memsize = total + nspaces(total, info.space) + has_sign_place + 1;
    char *const result = coll_queue(malloc(memsize * sizeof(char)));

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
            if (has_sign_place && is_signed && sys.rep == NR_2COMPL)
                digit_val -= 1;
            digit_val /= ullpow(sys.base, place);           // Shift right to desired digit
            digit_val -= digit_val / sys.base * sys.base;   // Subtract leading digits
            if (has_sign_place && is_signed && sys.rep & (NR_1COMPL|NR_2COMPL))
                digit_val = sys.base - digit_val - 1;       // Get complement
            result[i] = digit_val +                         // Get character representation
              (digit_val < 10 ? '0' : 'A' - 10);
            --nchrs;
        } else
            result[i] = is_signed ? max : '0';
        if (i == has_sign_place)
            break;
        if (info.space && !(++place % info.space))
            result[--i] = ' ';
    }
    if (has_sign_place) {
        if (is_signed)
            result[0] = sys.rep == NR_NEGSGN ? '-' : max;
        else
            result[0] = '0';
    }
    return result;
}
char *nsys_uconv(const char *numstr, unsigned src, unsigned dest, numinfo_t info) {
    const unsigned long long tmp = nsys_utonum(numstr, src);

    if (errno)
        return NULL;
    return nsys_utostr(tmp, dest, info);
}
unsigned long long nsys_utonum(const char *numstr, unsigned base) {
    if (!numstr || inval_base(base)) {
        errno = EINVAL;
        return 0;
    }

    coll_einit(unsigned long long, nsys_utonum, 0, numstr, base);
    char *const valid = coll_queue(valid_chrs((numsys_t) {base, NR_SPLACE}));

    if (!valid)
        return 0;

    char cur;
    unsigned digit_val;
    unsigned long long to_add, place_val = 1, result = 0;

    for (size_t i = strlen(numstr) - 1;; --i) {
        cur = numstr[i];
        if (!strchr(valid, cur)) {
            error(EINVAL, 0);
        } else if (!strchr(IGNORE, cur) && cur != '-') {
            digit_val = digit_to_num(cur);
            if (digit_val && place_val > ULLONG_MAX / digit_val)
                error(EOVERFLOW, 0);
            to_add = digit_val * place_val;
            if (result > ULLONG_MAX - to_add)
                error(EOVERFLOW, 0);
            result += to_add;
            place_val *= base;
        }
        if (!i)
            break;
    }
    return result;
}
char *nsys_utostr(unsigned long long num, unsigned base, numinfo_t info) {
    if (inval_base(base) || inval_info(info, base))
        error(EINVAL, NULL);

    unsigned nchrs = ndigits(num, base);

    if (errno)
        return NULL;
    
    const size_t total = max(info.min, nchrs);  // Total # of digits
    const size_t memsize = total + nspaces(total, info.space) + 1;
    char *const result = coll_queue(malloc(memsize * sizeof(char)));

    if (!result)
        return NULL;

    unsigned place = 0;
    unsigned long long digit_val;

    result[memsize - 1] = '\0';
    for (size_t i = memsize - 2;; --i) {
        if (nchrs) {
            digit_val = num / ullpow(base, place);
            digit_val -= digit_val / base * base;
            result[i] = digit_val + (digit_val < 10 ? '0' : 'A' - 10);
            --nchrs;
        } else
            result[i] = '0';
        if (!i)
            break;
        if (info.space && !(++place % info.space))
            result[--i] = ' ';
    }
    return result;
}
