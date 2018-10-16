#include "stdafx.h"
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "yuv_util.h"

int m_log_level = LOG_ERROR;

void set_log_level(int log_level)
{
	m_log_level = log_level;
}

void get_log_header(const int log_level, char *log_header)
{
	if (!log_header)
		return;
	
	switch(log_level)
	{
		case LOG_INFO: 
			strcpy(log_header, "Info");
			break;
		case LOG_DEBUG: 
			strcpy(log_header, "Debug");
			break;
		case LOG_WARNING: 
			strcpy(log_header, "Warning");
			break;
		case LOG_ERROR: 
			strcpy(log_header, "Error");
			break;
		default:
			strcpy(log_header, "Unknown");
			break;
	}
}

void yuv_log(int log_level, const char *fmt, ...)
{
	if (log_level < m_log_level)
		return;

	char log_str[256];
	char log_header[10] = {0};
	get_log_header(log_level, log_header);

	va_list arg;

	va_start(arg, fmt);
	vsprintf(log_str, fmt, arg);
	
	printf("[%s] %s\n", log_header, log_str);

	va_end(arg);
}

size_t yuv_strlcatf(char *dst, size_t size, const char *fmt, ...)
{
	size_t len = strlen(dst);
    va_list vl;

    va_start(vl, fmt);
    len += vsnprintf(dst + len, size > len ? size - len : 0, fmt, vl);
    va_end(vl);

    return len;
}

void yuv_get_file_name(const char *file_path, char *file_name) 
{
	
	if (!file_name)
	{
		yuv_log(LOG_ERROR, "file_name is null");
		return;
	}
	
	int len = strlen(file_path);
	int i = len - 1;
	while (i >= 0)
	{
		if (file_path[i] == '/'|| file_path[i] == '\\')
			break;
		i --;
	}
	i ++;
	int j = len - 1;
	while (j > 0)
	{
		if (file_path[j] == '.')
			break;
		j--;
	}
	if (j <= i)
		j = len;
	memcpy(file_name, file_path+i, j-i);
}

char *yuv_get_file_directory(const char *file_path)
{
	char *file_directory;
	int len = strlen(file_path);
	int i = len - 1;
	while (i >= 0)
	{
		if (file_path[i] == '/' || file_path[i] == '\\')
			break;
		i --;
	}
	if (i <= 0) 
		return NULL;
	file_directory = (char *)malloc((i+1)*sizeof(char));
	memcpy(file_directory, file_path, i);
	return file_directory;
}
