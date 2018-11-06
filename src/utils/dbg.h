#ifndef __dbg_h__
#define __dbg_h__

#include <stdio.h>
#include <errno.h>
#include <string.h>

#ifdef NDEBUG
#define debug(M, ...)
#else
#define debug(M, ...) fprintf(stderr, "DEBUG %s:%d " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif

//Not thread safe lol
char buff[500];

#ifdef _WIN32

#define LOG_ERR(M, ...)\
{strerror_s(buff, 500, errno);\
fprintf(stderr, "\x1B[31m[E] (%s:%s:%d errno %s) " M "\x1B[0m\n", __FILE__, __FUNCTION__, __LINE__,\
buff, ##__VA_ARGS__);}

#define LOG_WARN(M, ...)\
{strerror_s(buff, 500, errno);\
char buff[500]; fprintf(stderr, "\x1B[35m[W] (%s:%s:%d errno %s) " M "\x1B[0m\n", __FILE__, __FUNCTION__, __LINE__,\
buff, ##__VA_ARGS__);}

#define LOG_INFO(M, ...)\
{strerror_s(buff, 500, errno);\
char buff[500]; fprintf(stderr, "\x1B[32m[I] (%s:%s:%d errno %s) " M "\x1B[0m\n", __FILE__, __FUNCTION__, __LINE__,\
buff, ##__VA_ARGS__);}

#endif

#ifndef _WIN32
#define CLEAN_ERRNO() (errno == 0 ? "None" : strerror(errno))

#define LOG_ERR(M, ...) fprintf(stderr, "\x1B[31m[E] (%s:%s:%d errno %s) " M "\x1B[0m\n", __FILE__, __FUNCTION__, __LINE__,\
CLEAN_ERRNO(), ##__VA_ARGS__)

#define LOG_WARN(M, ...) fprintf(stderr, "\x1B[35m[W] (%s:%s:%d errno %s) " M "\x1B[0m\n", __FILE__, __FUNCTION__, __LINE__,\
CLEAN_ERRNO(), ##__VA_ARGS__)

#define LOG_INFO(M, ...) fprintf(stderr, "\x1B[32m[I] (%s:%s:%d errno %s) " M "\x1B[0m\n", __FILE__, __FUNCTION__, __LINE__,\
CLEAN_ERRNO(), ##__VA_ARGS__)

#endif

#define CHECK(A, M, ...) if (!(A)) { LOG_ERR(M, ##__VA_ARGS__); errno = 0; goto error; }

#define SENTINEL(M, ...) {LOG_ERR(M, ##__VA_ARGS__); errno = 0; goto error; }

#define CHECK_MEM(A) CHECK((A), "Out of memory")

#define CHECK_DEBUG(A, M, ...) if(!(A)) { debug(M, ##__VA_ARGS__); errno = 0; goto error; }

#endif