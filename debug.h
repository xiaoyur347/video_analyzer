#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include <libgen.h>

#define LOG_DEBUG(format, ...) printf("[%s][%s:%d][Debug]" format "\n", basename((char*)__FILE__), __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOG_INFO(format, ...) printf("[%s][%s:%d][Info]" format "\n", basename((char*)__FILE__), __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOG_WARN(format, ...) printf("[%s][%s:%d][Warn]" format "\n", basename((char*)__FILE__), __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) printf("[%s][%s:%d][Error]" format "\n", basename((char*)__FILE__), __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__)

#endif
