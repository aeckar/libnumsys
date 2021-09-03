# libnumsys
Number system conversion library made in C.

## Reasoning
In many applications of computer science, it is necessary to differentiate between the different number systems that programmers use on a day-to-day basis. The most common of these include binary, octal, and hexadecimal. Although one can simply practice conversion between them and regular decimal, such a feat is simply not practical for everyone who needs to use these representations. Because of the need for a fast and convenient number system converter, I created this library.

## Usage Notes
At a glance, the library's features include:
```C++
// Information pertaining to a number's string representation
typedef struct numinfo_t {
    unsigned min;   // Minimum # of digits
    unsigned space; // # of digits between spaces, 0 for no spaces
} numinfo_t;

// Negative number representation type
typedef enum {
    NR_NEGSGN = 1, // Append negative sign to front
    NR_SPLACE = 2, // Append nonzero digit to front
    NR_1COMPL = 4, // Set as complement of positive
    NR_2COMPL = 8  // Set as complement of positive plus 1
} numrep_t;

// Number system type
typedef struct {
    unsigned base;
    numrep_t rep;
} numsys_t;

// Converts string in given base/representation to integer
long long nsys_tonum(const char *, numsys_t);
unsigned long long nsys_utonum(const char *, unsigned);   // Unsigned ver.

// Converts string in one base/representation to string in another
char *nsys_conv(const char *, numsys_t, numsys_t, numinfo_t);
char *nsys_uconv(const char *, unsigned, unsigned, numinfo_t);

// Converts integer to string in given base/representation
char *nsys_tostr(long long, numsys_t, numinfo_t);
char *nsys_utostr(unsigned long long, unsigned, numinfo_t);
```
The library contains a single header, `numsys.h`, from which further information can found.

If using the library with standard C99 or later, it is useful to know that arguments of type `numsys_t` can be passed without the use of a temporary buffer by using a [compound literal](https://en.cppreference.com/w/c/language/compound_literal). Underscores are allowed in number strings, and are encouraged to preserve readability.
```C
nsys_tonum("1_028", (numsys_t){10, NEG_SIGN});
```
Similarly, in C++, arguments of type `numsys_t` can be passed using an [initializer list](https://en.cppreference.com/w/cpp/utility/initializer_list), as `numsys_t` is considered a [POD type](https://stackoverflow.com/questions/146452/what-are-pod-types-in-c).
```C++
nsys_tostring(0b101101, {2, SIGN_PLACE});
```
