#include "stdafx.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "y4m_yuv.h"
#include "yuv_util.h"

int y4m_read_header(FILE *y4m_file, uint8_t *y4m_header_buffer) 
{
	if (!y4m_header_buffer || !y4m_file)
	{
		yuv_log(LOG_ERROR, "y4m_header_buffer or y4m_file is null");
		return -1;
	}
	uint8_t buffer[1024];
	int i = 0;
	int count_size;
	count_size = fread(buffer, 1, sizeof(buffer), y4m_file);
	
	while (buffer[i] != 0x0A)
		i++;
	memcpy(y4m_header_buffer, buffer, i);
	fseek(y4m_file, 0, SEEK_SET);
	return i;
}

void y4m_parse_mode(char *key, char *value) 
{
	if (!key || !value)
	{
		yuv_log(LOG_ERROR, "key or value is null");
		return;
	}
	key ++;
	switch (*key)
	{
		case 'p':
			strcpy(value, "Progressive");
			break;
		case 't':
			strcpy(value, "Top field First");
			break;
		case 'b':
			strcpy(value, "Button field First");
			break;
		case 'm':
			strcpy(value, "Mixed Mode");
			break;
		default:
			strcpy(value, "Unknown");
			break;
	}
}

void y4m_parse_format(char *key, int *value)
{
	if (!key || !value)
	{
		yuv_log(LOG_ERROR, "key or value is null");
		return;
	}
	int yuv_format;
	sscanf(key, "C%d", &yuv_format);
	switch (yuv_format) 
	{
		case 422:
			*value = YUV_422;
			break;
		case 444:
			*value = YUV_444;
			break;
		case 411:
			*value = YUV_411;
			break;
		case 420:
			*value = YUV_420;
			break;
		default:
			break;
	}
}

void y4m_get_format(int key_format, char *format)
{
	if (!format)
	{
		yuv_log(LOG_ERROR, "format is null");
		return;
	}
	
	switch (key_format)
	{
		case YUV_422:
			strcpy(format, "YUV 422");
			break;
		case YUV_444:
			strcpy(format, "YUV 444");
			break;
		case YUV_411:
			strcpy(format, "YUV 411");
			break;
		case YUV_420:
			strcpy(format, "YUV 420");
			break;
		default:
			strcpy(format, "Unknown");
			break;
	}
}


Y4MHeader *y4m_header_alloc()
{
	Y4MHeader *y4m_header = (Y4MHeader *)calloc(1, sizeof(Y4MHeader));
	y4m_header->format = YUV_420;
	return y4m_header;
}

void y4m_header_close(Y4MHeader *y4m_header)
{
	if (y4m_header)
		free(y4m_header);
}

Y4MContext *y4m_context_alloc()
{
	Y4MContext *y4m_context = (Y4MContext *)calloc(1, sizeof(Y4MContext));
	y4m_context->header = y4m_header_alloc();
	y4m_context->frame_count = 0;
	return y4m_context;
}

void y4m_context_close(Y4MContext *y4m_context)
{
	if (y4m_context)
	{
		y4m_header_close(y4m_context->header);
		free(y4m_context);
	}
}

int y4m_parse_header(Y4MHeader *header, uint8_t *y4m_header_buffer, int header_size) 
{
	char head[10] = {0};
	uint8_t *header_buffer;
	
	if (!header || !y4m_header_buffer || header_size == 0)
	{
		yuv_log(LOG_ERROR, "y4m_header_buffer or y4m_file is null, or header_size is equal zero");
		return -1;
	}
	
	memcpy(head, y4m_header_buffer, 9);
	yuv_log(LOG_INFO, "head = %s", head);
	if (strcmp(head, "YUV4MPEG2"))
	{
		yuv_log(LOG_ERROR, "head not equal YUV4MPEG2");
		return -1;
	}
	header_buffer = y4m_header_buffer;
	header_buffer += 10;
	
	char *temp = strtok((char *)header_buffer, " ");
	while (temp)
	{
		yuv_log(LOG_INFO, "temp : %s", temp);
		switch(*temp)
		{
			case 'W':
				temp ++;
				header->width = atoi(temp);
				break;
			case 'H':
				temp ++;
				header->height = atoi(temp);
				break;
			case 'F':
				int framerate_molecular, framerate_denominator;
				sscanf(temp, "F%d:%d", &framerate_denominator, &framerate_molecular);
				header->frame_rate = framerate_denominator/framerate_molecular;
				break;
			case 'I':
				y4m_parse_mode(temp, header->mode);
				break;
			case 'A':
				temp ++;
				strcpy(header->pixel_aspect_ratio, temp);
				break;
			case 'C':
				y4m_parse_format(temp, &(header->format));
				break;
			default:
				break;
		}
		temp = strtok(NULL, " ");
	}
}

size_t y4m_read_on_frame(Y4MContext *y4m_context, uint8_t *frame_buffer, size_t size)
{
	FILE *y4m_file = y4m_context->y4m_file;
	uint8_t *buffer = (uint8_t *)calloc(size+6, 1);
	size_t len;
	
	len = fread(buffer, 1, size+6, y4m_file);
	if (buffer[0] == 'F' && buffer[1] == 'R' && buffer[2] == 'A' && buffer[3] == 'M' &&
		buffer[4] == 'E' && buffer[5] == 0x0A)
	{
		memcpy(frame_buffer, buffer+6, size);
	}
	free(buffer);
	return len;
}

void y4m_print_yuv_info(Y4MContext *y4m_context)
{
	char yuv_info_str[2048] = {0};
	Y4MHeader *header = y4m_context->header;
	FILE *yuv_info_file = y4m_context->yuv_info_file;
	char yuv_format[20] = {0};
	
	if (!y4m_context)
	{
		yuv_log(LOG_ERROR, "y4m_context is null");
		return;
	}
	yuv_log(LOG_INFO, "Y4M File Path : %s", y4m_context->y4m_filepath );
	if (y4m_context->y4m_filepath)
		sprintf(yuv_info_str, "%-20s : %s\r\n", "Y4M File Path", y4m_context->y4m_filepath);
	
	yuv_strlcatf(yuv_info_str, sizeof(yuv_info_str), "%-20s : %d\r\n", "Width", header->width);
	yuv_strlcatf(yuv_info_str, sizeof(yuv_info_str), "%-20s : %d\r\n", "Height", header->height);
	yuv_strlcatf(yuv_info_str, sizeof(yuv_info_str), "%-20s : %lf\r\n", "Frame Rate", header->frame_rate);
	
	if (header->mode)
		yuv_strlcatf(yuv_info_str, sizeof(yuv_info_str), "%-20s : %s\r\n", "Mode", header->mode);
	if (header->pixel_aspect_ratio)
		yuv_strlcatf(yuv_info_str, sizeof(yuv_info_str), "%-20s : %s\r\n", "Pixel Aspect Ratio", header->pixel_aspect_ratio);
	
	y4m_get_format(header->format, yuv_format);
	if (yuv_format)
		yuv_strlcatf(yuv_info_str, sizeof(yuv_info_str), "%-20s : %s\r\n", "YUV Format", yuv_format);
	yuv_log(LOG_INFO, "%s", yuv_info_str);
	
	fwrite(yuv_info_str, 1, strlen(yuv_info_str), yuv_info_file);
}

FILE *y4m_get_yuv_info_file(char *file_directory, char *file_name)
{
	char *yuv_info_filepath;
	FILE *yuv_info_file = NULL;
	size_t len = strlen(file_directory)+1+strlen(file_name)+5;
	
	yuv_info_filepath = (char *)calloc(len, sizeof(char));
	sprintf(yuv_info_filepath, "%s%s%s%s", file_directory, "/", file_name, ".txt");
	
	yuv_log(LOG_INFO, "yuv_info_filepath: %s", yuv_info_filepath);

	yuv_info_file = fopen(yuv_info_filepath, "w+");
	if (!yuv_info_file)
	{
		yuv_log(LOG_ERROR, "%s fopen error", yuv_info_filepath);
	}
		
	return yuv_info_file;
}

FILE *y4m_open_yuv_file(char *file_directory, char *file_name)
{
	char *yuv_filepath;
	FILE *yuv_file = NULL;
	size_t len = strlen(file_directory)+1+strlen(file_name)+5;

	yuv_filepath = (char *)calloc(len, sizeof(char));
	sprintf(yuv_filepath, "%s%s%s%s", file_directory, "/", file_name, ".yuv");

	yuv_log(LOG_INFO, "yuv_filepath: %s", yuv_filepath);

	yuv_file = fopen(yuv_filepath, "wb+");
	if (!yuv_file)
	{
		yuv_log(LOG_ERROR, "%s fopen error", yuv_filepath);
	}
		
	return yuv_file;
}

size_t y4m_get_one_frame_size(int frame_format, int width, int height)
{
	size_t frame_size = 0;
	
	switch (frame_format)
	{
		case YUV_420:
			frame_size = width * height * 3 / 2;
			break;
		case YUV_422:
			frame_size = width * height * 2;
			break;
		case YUV_444:
			frame_size = width * height * 3;
			break;
		case YUV_411:
			frame_size = width * height * 3 / 2;
			break;
		default:
			break;
	}
	return frame_size;
}
