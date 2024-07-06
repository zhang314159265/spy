#ifndef SUTIL_H
#define SUTIL_H

#define true 1
#define false 0

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*(x)))

#define fatal(fmt, ...) \
	fprintf(stderr, "FATAL %s:%d: ", __FILE__, __LINE__ ); \
	fprintf(stderr, fmt, __VA_ARGS__); \
	fprintf(stderr, "\n"); \
	exit(1);

#endif
