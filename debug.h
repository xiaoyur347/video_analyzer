#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include <libgen.h>

/** 亮色 */
#define BRIGHT		1
/** 背景为黑色 */
#define BG_BLACK	40

#define LOG_DEBUG(format, ...) printf("[%s][%s:%d][Debug]" format "\n", \
	basename((char*)__FILE__), __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOG_INFO(format, ...) printf("[%s][%s:%d][Info]" format "\n", \
	basename((char*)__FILE__), __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOG_WARN(format, ...) printf("%c[%d;%d;%dm[%s][%s:%d][Warn]" format "%c[%dm\n", \
	0x1B, BRIGHT, 34, BG_BLACK, \
	basename((char*)__FILE__), __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__, \
	0x1B, 0)
#define LOG_ERROR(format, ...) printf("%c[%d;%d;%dm[%s][%s:%d][Error]" format "%c[%dm\n", \
	0x1B, BRIGHT, 31, BG_BLACK, \
	basename((char*)__FILE__), __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__, \
	0x1B, 0)

#endif
