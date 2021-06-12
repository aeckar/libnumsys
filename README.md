# numsys
Number system conversion library made in C

In many applications of computer science, it is necessary to differentiate between the different number systems that programmers use on a day-to-day basis. The most common of these include binary, octal, and hexadecimal. Although one can simply practice conversion between them and regular decimal,such a feat is simply not practical for everyone who needs to use these representations

In other fields, such as in the study of historical number systems, there is also a need for a quick and easy-to-use conversion method. Although impractical to represent all symbols of each system in a single method, numeral systems such as the Sumerian, Babylonian, Mayan, Muisca, and Kaktovik all can have their digits represented in English using the beginning letters of the alphabet, similar to in hexadecimal.

Because of the need for a fast and convenient number system converter, I created this library. At a glance, its features include:
```C
// Number representation type
typedef enum {
	NEG_SIGN   = 1,	// Append negative sign to front
	SIGN_BIT   = 2,	// Append nonzero digit to front
	ONES_COMPL = 4,	// Set as complement of positive
	TWOS_COMPL = 8	// Set as complement of positive plus 1
} notation_t;

// Number system type
typedef struct {
	unsigned base;
	notation_t notat;
} numsys_t;

// Converts number string to integer value
long long numsys_tonum(const char *, numsys_t);

// Converts number string in one base/representation to number string in another
char *numsys_conv(const char *, numsys_t, numsys_t);

// Converts integer value to number string in given base and representation
char *numsys_tostring(long long, numsys_t);
```
The library contains a single header, `numsys.h`, from which further information can found.
