#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include <libgen.h>

#define LOG_DEBUG(format, ...) printf("[%s:%d][Debug]" format "\n", basename((char*)__FILE__), __LINE__, ##__VA_ARGS__)
#define LOG_INFO(format, ...) printf("[%s:%d][Info]" format "\n", basename((char*)__FILE__), __LINE__, ##__VA_ARGS__)
#define LOG_WARN(format, ...) printf("[%s:%d][Warn]" format "\n", basename((char*)__FILE__), __LINE__, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) printf("[%s:%d][Error]" format "\n", basename((char*)__FILE__), __LINE__, ##__VA_ARGS__)

#endif
