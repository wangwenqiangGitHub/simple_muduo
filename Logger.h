#ifndef SIMPLE_MUDUO_LOGGER_H
#define SIMPLE_MUDUO_LOGGER_H


#include <string>

#include "noncopyable.h"
namespace simple_muduo{
#define LOG_INFO(fmt, ...) fprintf(stdout, fmt, ##__VA_ARGS__);
/* abort() at OME MUST NOT BE REMOVED !!!*/
#define LOG_ERROR(fmt, ...) \
do{ \
	fprintf(stderr, "%s(%d)-<%s>: " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); \
	abort(); \
}while(0)
}
#endif // SIMPLE_MUDUO_LOGGER_H
