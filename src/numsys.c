#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "numsys.h"

/* String of standard whitespace characters */
#define WHITESP        "\t\n\v\f\r "
#define WHITESP_LEN    6
// Returns true if invalid
#define inval_base(base)   ((base) < 1 || (base) > 36)
#define inval_notat(notat) (!((notat) & (NEG_SIGN|SIGN_BIT|ONES_COMPL|TWOS_COMPL)))

// Returns digit of maximum value for given base
#define max_digit(base) ((base) < 11 ? (base) + 47 : (base) + 54)

// String constant from which new validity strings are created 
static const char *DIGITS = "0123456789"
                            "aAbBcCdDeEfFgGhHiIjJkKlLm"
                            "MnNoOpPqQrRsStTuUvVwWxXyYzZ";

/* Returns number of digits in equivalent number string
 * Returns 0 and sets errno to ERANGE on error */
static unsigned ndigits(long long num, unsigned base) {
    unsigned count = 0;

    num = llabs(num);
    if (base == 1) {
        if (num > UINT_MAX) {
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
    const size_t NCHRS = strlen(NUMSTR) - 1;

    for (size_t i = 0;; ++i) {
        if (!isspace(NUMSTR[i]))
            return i;
    }
}

/* Returns string of valid characters in number string of given system
 * Returns NULL on error */ 
static char *valid_chrs(numsys_t sys) {
    const bool NEG_IS_VAL = sys.notat == NEG_SIGN;
    const size_t MEMSIZE = WHITESP_LEN + sys.base +
        (sys.base > 10 ? sys.base - 10 : 0) + NEG_IS_VAL + 2;
    char *const valid = calloc(MEMSIZE, sizeof(char));

    if (!valid)
        return NULL;

    size_t digit_index = 0;

    memcpy(valid, WHITESP, WHITESP_LEN);
    if (NEG_IS_VAL)
        valid[WHITESP_LEN] = '-';
    for (size_t i = WHITESP_LEN + NEG_IS_VAL; i < MEMSIZE - 2; ++i) {
        valid[i] = DIGITS[digit_index++];
        if (i >= WHITESP_LEN - 1 + 10)
            valid[++i] = DIGITS[digit_index++];
    }
    return valid;
}

long long numsys_tonum(const char *NUMSTR, numsys_t sys) {
    if (!NUMSTR || inval_base(sys.base) || inval_notat(sys.notat)) {
        errno = EINVAL;
        return 0;
    }

    char *const valid = valid_chrs(sys);

    if (!valid)
        return 0;

    char cur;
    unsigned digit_val;
    size_t sign_index = locate_sign(NUMSTR);
    unsigned long long to_add, place_val = 1;
    long long result = 0;

    for (size_t i = strlen(NUMSTR) - 1;; --i) {
        cur = NUMSTR[i];
        if (!strchr(valid, cur)) {
            errno = EINVAL;
            free(valid);
            return 0;
        }
        if (sys.notat == NEG_SIGN && cur == '-') {
            if (i != sign_index) {
                errno = EINVAL;
                free(valid);
                return 0;
            }
            result *= -1;
        } else if (sys.notat != NEG_SIGN && i == sign_index && cur != '0')
            result *= -1;
        else if (!isblank(cur)) {
            digit_val = digit_to_num(cur);
            if (place_val > LLONG_MAX / digit_val) {
                errno = EOVERFLOW;
                free(valid);
                return 0;
            }
            to_add = digit_val * place_val;
            if (result > LLONG_MAX - to_add) {
                errno = EOVERFLOW;
                free(valid);
                return 0;
            }
            result += to_add;
            place_val *= sys.base;
        }
        if (i == 0)
            break;
    }
    free(valid);
    return result;
}
char *numsys_tostring(long long num, numsys_t sys) {
    if (inval_base(sys.base) || inval_notat(sys.notat)) {
        errno = EINVAL;
        return NULL;
    }

    const unsigned NCHRS = ndigits(num, sys.base);

    if (errno > 0)
        return NULL;

    const bool HAS_SIGN =
        (sys.notat == NEG_SIGN && num < 0 || sys.notat != NEG_SIGN) && sys.base != 1;
    char *const result = calloc(NCHRS + HAS_SIGN + 1, sizeof(char));

    if (!result)
        return NULL;

    unsigned place = 0;
    long long digit_val;
    const long long num_abs = llabs(num);

    for (size_t i = NCHRS + HAS_SIGN - 1;; --i) {
        digit_val = num_abs;
        if (HAS_SIGN && sys.notat == TWOS_COMPL)
            digit_val -= 1;
        digit_val /= pow(sys.base, place++);                 // Shift right to desired digit
        digit_val -= digit_val / sys.base * sys.base;        // Subtract leading digits
        if (HAS_SIGN && sys.notat & (ONES_COMPL|TWOS_COMPL))
            digit_val = sys.base - digit_val - 1;            // Get complement
        result[i] = digit_val +
            (digit_val >= 0 && digit_val <= 9 ? 48 : 55);    // 48 = '0', 58 = 10 + 'A'
        if (i == HAS_SIGN)
            break;
    }
    if (HAS_SIGN) {
        if (num < 0)
            result[0] = sys.notat == NEG_SIGN ? '-' : max_digit(sys.base);
        else
            result[0] = '0';

    }
        
    return result;
}
char *numsys_conv(const char *NUMSTR, numsys_t src, numsys_t dest) {
    const long long TMP = numsys_tonum(NUMSTR, src);

    if (errno > 0)
        return NULL;
    return numsys_tostring(TMP, dest);
}
