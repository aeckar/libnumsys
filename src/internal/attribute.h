#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

// Ensures GLIBC attribute portability
#ifdef __GNUC__
#define glibc_attribute(...)	__attribute__((__VA_ARGS__))
#else
#define glibc_attribute(...)
#endif // #ifdef __GNUC__

#endif
