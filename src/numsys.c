#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "numsys.h"

// Allows for null check in spite of 'nonnull' attribute
#pragma GCC diagnostic ignored  "-Wnonnull-compare"

// Allows for optimization in the case of uncommon conditionals
#ifdef __GNUC__
#define uncommon(cond) (__builtin_expect(cond, false))
#else
#define uncommon(cond) (cond)
#endif

// String of standard whitespace characters and underscore
#define IGNORE      "\t\n\v\f\r _"
#define IGNORE_LEN  7

// Returns true if invalid
#define inval_base(base)    ((base) < 1 || (base) > 36)
#define inval_rep(rep)      (!((rep) & 15))   // 15 = NEG_SIGN|SIGN_PLACE|ONES_COMPL|TWOS_COMPL

// Returns digit of maximum value for given base
#define max_digit(base) ((base) < 11 ? (base) + 47 : (base) + 54)

// String constant from which new validity strings are created 
static const char *DIGITS = "0123456789"
                            "aAbBcCdDeEfFgGhHiIjJkKlLm"
                            "MnNoOpPqQrRsStTuUvVwWxXyYzZ";

/* Returns number of digits in equivalent number string
 * Returns 0 and sets errno to ERANGE on error */
static unsigned ndigits(unsigned long long num, unsigned base) {
    unsigned count = 0;

    if (base == 1) {
        if uncommon (num > UINT_MAX) {
            errno = ERANGE;
            return 0;
        }
        return num;
    }
    do {
        num /= base;
        ++count;
    } while (num);
    return count;
}

// Returns int equivalent of char digit
static unsigned digit_to_num(char c) {
    if (isdigit(c)) {
        return c - 48;      // '0' = 48
    } else if (c > 90) {    // c is lowercase
        return c - 97 + 10; // 'a' = 97
    }                       // c is uppercase
    return c - 65 + 10;     // 'A' = 65
}

// For negative numbers, returns index of sign bit or negative sign
static size_t locate_sign(const char *NUMSTR) {
    for (size_t i = 0;; ++i) {
        if (!strchr(IGNORE, NUMSTR[i]))
            return i;
    }
}

// Fast exponentiation algorithm
unsigned long long ullpow(unsigned long long base, unsigned long long pow){
    unsigned long result = 1;

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
    const bool NEG_IS_VAL = sys.rep == NEG_SIGN;
    const size_t MEMSIZE = IGNORE_LEN + sys.base +
        (sys.base > 10 ? sys.base - 10 : 0) + NEG_IS_VAL + 2;
    char *const valid = calloc(MEMSIZE, sizeof(char));

    if uncommon (!valid)
        return NULL;

    size_t digit_index = 0;

    memcpy(valid, IGNORE, IGNORE_LEN);
    if (NEG_IS_VAL)
        valid[IGNORE_LEN] = '-';
    for (size_t i = IGNORE_LEN + NEG_IS_VAL; i < MEMSIZE - 2; ++i) {
        valid[i] = DIGITS[digit_index++];
        if (i >= IGNORE_LEN - 1 + 10)
            valid[++i] = DIGITS[digit_index++];
    }
    return valid;
}

long long numsys_tonum(const char *NUMSTR, numsys_t sys) {
    if uncommon (!NUMSTR || inval_base(sys.base) || inval_rep(sys.rep)) {
        errno = EINVAL;
        return 0;
    }

    char *const valid = valid_chrs(sys);

    if uncommon (!valid)
        return 0;

    const size_t SIGN_INDEX = locate_sign(NUMSTR);
    const bool IS_SIGNED =
        (sys.rep != NEG_SIGN && NUMSTR[SIGN_INDEX] != '0') || NUMSTR[SIGN_INDEX] == '-';
    char cur;
    unsigned digit_val;
    long long to_add, place_val = 1, result = 0;

    if (sys.rep == TWOS_COMPL && IS_SIGNED)
        ++result;
    for (size_t i = strlen(NUMSTR) - 1;; --i) {
        cur = NUMSTR[i];
        if uncommon (!strchr(valid, cur)) {
            errno = EINVAL;
            free(valid);
            return 0;
        }
        if uncommon (cur == '-' && (sys.rep != NEG_SIGN || i != SIGN_INDEX)) {
            errno = EINVAL;
            free(valid);
            return 0;
        } else if (!strchr(IGNORE, cur) && cur != '-') {
            digit_val =    sys.rep & 12 && IS_SIGNED ?  // 12 = ONES_COMPL|TWOS_COMPL
                sys.base - (sys.base != 1) - digit_to_num(cur) : digit_to_num(cur);
            if uncommon (digit_val && place_val > LLONG_MAX / digit_val) {
                errno = EOVERFLOW;
                free(valid);
                return 0;
            }
            to_add = digit_val * place_val;
            if uncommon (result > LLONG_MAX - to_add) {
                errno = EOVERFLOW;
                free(valid);
                return 0;
            }
            result += to_add;
            place_val *= sys.base;
        }
        if (i == (sys.rep != NEG_SIGN))
            break;
    }
    if (IS_SIGNED) {
        if uncommon (result == LLONG_MIN) {
            errno = EOVERFLOW;
            free(valid);
            return 0;
        }
        result = -result;
    }
    free(valid);
    return result;
}
unsigned long long numsys_utonum(const char *NUMSTR, unsigned base) {
    if uncommon (!NUMSTR || inval_base(base)) {
        errno = EINVAL;
        return 0;
    }

    char *const valid = valid_chrs((numsys_t) {base, SIGN_PLACE});

    if uncommon (!valid)
        return 0;

    char cur;
    unsigned digit_val;
    unsigned long long to_add, place_val = 1, result = 0;

    for (size_t i = strlen(NUMSTR) - 1;; --i) {
        cur = NUMSTR[i];
        if uncommon (!strchr(valid, cur)) {
            errno = EINVAL;
            free(valid);
            return 0;
        } else if (!strchr(IGNORE, cur) && cur != '-') {
            digit_val = digit_to_num(cur);
            if uncommon (digit_val && place_val > ULLONG_MAX / digit_val) {
                errno = EOVERFLOW;
                free(valid);
                return 0;
            }
            to_add = digit_val * place_val;
            if uncommon (result > ULLONG_MAX - to_add) {
                errno = EOVERFLOW;
                free(valid);
                return 0;
            }
            result += to_add;
            place_val *= base;
        }
        if (i == 0)
            break;
    }
    free(valid);
    return result;
}
char *numsys_conv(const char *NUMSTR, numsys_t src, numsys_t dest) {
    const long long TMP = numsys_tonum(NUMSTR, src);

    if uncommon (errno > 0)
        return NULL;
    return numsys_tostring(TMP, dest);
}
char *numsys_uconv(const char *NUMSTR, unsigned src, unsigned dest) {
    const unsigned long long TMP = numsys_utonum(NUMSTR, src);

    if uncommon (errno > 0)
        return NULL;
    return numsys_utostring(TMP, dest);
}
char *numsys_tostring(long long num, numsys_t sys) {
    if uncommon (inval_base(sys.base) || inval_rep(sys.rep)) {
        errno = EINVAL;
        return NULL;
    }

    const unsigned NCHRS = ndigits(llabs(num), sys.base);

    if uncommon (errno > 0)
        return NULL;

    const bool IS_SIGNED = num < 0;
    const bool HAS_SIGN_PLACE =
        ((sys.rep == NEG_SIGN && num < 0) || sys.rep != NEG_SIGN) && sys.base != 1;
    char *const result = calloc(NCHRS + HAS_SIGN_PLACE + 1, sizeof(char));

    if uncommon (!result)
        return NULL;

    unsigned place = 0;
    unsigned long long digit_val;
    const long long NUM_ABS = llabs(num);

    for (size_t i = NCHRS + HAS_SIGN_PLACE - 1;; --i) {
        digit_val = NUM_ABS;
        if (HAS_SIGN_PLACE && IS_SIGNED && sys.rep == TWOS_COMPL)
            digit_val -= 1;
        digit_val /= ullpow(sys.base, place++);             // Shift right to desired digit
        digit_val -= digit_val / sys.base * sys.base;       // Subtract leading digits
        if (HAS_SIGN_PLACE && IS_SIGNED && sys.rep & 12)    // 12 = ONES_COMPL|TWOS_COMPL
            digit_val = sys.base - digit_val - 1;           // Get complement
        result[i] = digit_val + (digit_val <= 9 ? 48 : 55); // 48 = '0', 58 = 10 + 'A'
        if (i == HAS_SIGN_PLACE)
            break;
    }
    if (HAS_SIGN_PLACE) {
        if (IS_SIGNED)
            result[0] = sys.rep == NEG_SIGN ? '-' : max_digit(sys.base);
        else
            result[0] = '0';
    }
    return result;
}
char *numsys_utostring(unsigned long long num, unsigned base) {
    if uncommon (inval_base(base)) {
        errno = EINVAL;
        return NULL;
    }

    const unsigned NCHRS = ndigits(num, base);

    if uncommon (errno > 0)
        return NULL;

    char *const result = calloc(NCHRS + 1, sizeof(char));

    if uncommon (!result)
        return NULL;

    unsigned place = 0;
    unsigned long long digit_val;

    for (size_t i = NCHRS - 1;; --i) {
        digit_val = num;
        digit_val /= ullpow(base, place++);
        digit_val -= digit_val / base * base;
        result[i] = digit_val + (digit_val <= 9 ? 48 : 55);
        if (i == 0)
            break;
    }
    return result;
}
