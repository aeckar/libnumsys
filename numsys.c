#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "numsys.h"

// Ensures GLIBC attribute portability
#ifdef __GNUC__
#define glibc_attribute(...)	__attribute__((__VA_ARGS__))
#else
#define glibc_attribute(...)
#endif // #ifdef __GNUC__
/* Sting of standard whitespace characters */
#define WHITESPACE "\t\n\v\f\r "

// Returns true if invalid
#define inval_base(base)	((base) < 1 || (base) > 36)
#define inval_notat(notat) ((notat) & (NEG_SIGN|SIGN_BIT|ONES_COMPL|TWOS_COMPL))

// Returns digit of maximum value for given base
#define max_digit(base) ((base) < 11 ? (base) + 47 : (base) + 54)

// String constant from which new validity strings are created 
static const char *DIGITS = "0123456789"
							"aAbBcCdDeEfFgGhHiIjJkKlLm"
							"MnNoOpPqQrRsStTuUvVwWxXyYzZ";

// Returns number of digits in equivalent number string
static unsigned ndigits(long long num, unsigned base) {
	int count = 0;
	num = llabs(num);
	do {
		num /= base;
		++count;
	} while (num);
	return count;
}

// Returns int equivalent of char digit
static unsigned digit_to_num(char c) {
	if (isdigit(c)) {
		return c - 48;	// '0' = 48
	} else if (c > 90) {	// c is lowercase
		return c - 97 + 10;	// 'a' = 97
	}	// c is uppercase
	return c - 65 + 10;	// 'A' = 65
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
 * Excludes negative sign */ 
static char *valid_chrs(numsys_t sys) {
	const size_t MEMSIZE = sizeof(WHITESPACE) + sys.base + (sys.base % 10) * (sys.base / 10);
	char *const s = calloc(MEMSIZE, sizeof(char));
	strcpy(s, WHITESPACE);
	for (size_t i = sizeof(WHITESPACE) - 1; i < MEMSIZE - 2; ++i) {
		s[i] = DIGITS[i];
		if (i >= sizeof(WHITESPACE) - 1 + 10)
			s[i] = DIGITS[++i];
	}
	return s;
}

long long numsys_tonum(const char *NUMSTR, numsys_t sys) {
	if (!NUMSTR || inval_base(sys.base)) {
		errno = EINVAL;
		return 0;
	}
	char cur;
	unsigned digit_val;
	size_t sign_index = locate_sign(NUMSTR);
	unsigned long long to_add, place_val = 1;
	char *const valid = valid_chrs(sys);
	long long result = 0;
	for (size_t i = strlen(NUMSTR) - 1;; --i) {
		cur = NUMSTR[i];
		if (!strchr(valid, cur)) {
			errno = EINVAL;
			return 0;
		}
		if (sys.notat == NEG_SIGN && cur == '-') {
			if (i != sign_index) {
				errno = EINVAL;
				return 0;
			}
			result *= -1;
		} else if (i == sign_index && cur != '0')
			result *= -1;
		else if (!isblank(cur)) {
			digit_val = digit_to_num(cur);
			if (place_val > LLONG_MAX / digit_val) {
				errno = EOVERFLOW;
				return 0;
			}
			to_add = digit_val * place_val;
			if (result > LLONG_MAX - to_add) {
				errno = EOVERFLOW;
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
	unsigned place = 0;
	const unsigned NCHRS = ndigits(num, sys.base);
	const unsigned SIGN = sys.notat == NEG_SIGN && num < 0 || sys.notat != NEG_SIGN;
	long long n;
	const long long num_abs = llabs(num);
	char *const s = calloc(NCHRS + SIGN + 1, sizeof(char));
	if (inval_notat(sys.notat)) {
		errno = EINVAL;
		return NULL;
	}
	for (size_t i = NCHRS + SIGN - 1;; --i) {
		n = num_abs;
		if (SIGN && sys.notat == TWOS_COMPL)
			n -= 1;
		n /= pow(sys.base, place++);				// Shift right to desired digit
		n -= n / sys.base * sys.base;				// Subtract leading digits
		if (SIGN && sys.notat & (ONES_COMPL|TWOS_COMPL))
			n = sys.base - 1 - n;					// Get complement
		s[i] = n + (n >= 0 && n <= 9 ? 48 : 55);	// 48 = '0', 58 = 10 + 'A'
		if (i == SIGN)
			break;
	}
	if (SIGN)
		s[0] = sys.notat == NEG_SIGN ? '-' : max_digit(sys.base);
	return s;
}
char *numsys_conv(const char *NUMSTR, numsys_t src, numsys_t dest) {
	const long long TMP = numsys_tonum(NUMSTR, src);
	if (errno > 0)
		return NULL;
	return numsys_tostring(TMP, dest);
}