#ifndef DEBUG_H
#define DEBUG_H


#if !(defined(__unix__) || (defined(__APPLE__) && defined(__MACH__)))
#error	"POSIX environment required for debugging."
#endif

#define query()						\
system("clear");					\
puts("Press ENTER for next page.");	\
getchar()

#endif