#ifndef YUV_UTIL_H
#define YUV_UTIL_H

typedef enum
{
	LOG_INFO,
	LOG_DEBUG,
	LOG_WARNING,
	LOG_ERROR,
} LOG_LEVEL;

void set_log_level(int log_level);

void yuv_log(int log_level, const char *fmt, ...);

size_t yuv_strlcatf(char *dst, size_t size, const char *fmt, ...);

void yuv_get_file_name(const char *file_path, char *file_name);

char *yuv_get_file_directory(const char *file_path);

#endif