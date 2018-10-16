// y4m_yuv.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <direct.h>
#include "y4m_yuv.h"
#include "yuv_util.h"

#include "getopt.h"

const char *version = "0.0.1";

const char *opt_string = "vhf:";

const char *help = 
	"Usage: y4m_yuv [options] infiles\n"
	"\n"
	"Options: \n"
	"-v, --version     The program version\n"
	"-h, --help        Help on using for y4m_yuv\n"
	"-f, --infiles     The input y4m file\n";

const static struct option long_options[] = {
	{"version", no_argument, 0, 'v'},
	{"help", no_argument, 0, 'h'},
	{"file", required_argument, 0, 'f'}
};

typedef struct Y4m_Yuv_Opts 
{
	char *file_path;
} Y4m_Yuv_Opts;

void print_version ()
{
	fprintf(stdout, 
		"--------------------------------------------------\n"
		"\t Y4m_Yuv.exe  %s\n"
		"--------------------------------------------------\n", version);
}

void print_help ()
{
	fprintf(stdout, "%s", help);
}

void parse_cmd_opt (int argc, char **argv, Y4m_Yuv_Opts *opts) 
{
	if (!opts) 
	{
		yuv_log(LOG_ERROR, "opts is null");
		return;
	}
		
	int opt;

	while ((opt = getopt_long(argc, argv, opt_string, long_options, NULL)) != -1)
	{
		switch (opt)
		{
		case 'h':
			print_help();
			break;
		case 'v':
			print_version();
			break;
		case 'f':
			opts->file_path = optarg;
			break;
		default:
			break;
		}
	}
}

Y4MContext *init (const char *file_path)
{
	FILE *y4m_file = NULL;
	Y4MContext *y4m_context;
	char file_name[1024] = {0};
	char *file_directory = NULL;
	char file_sub_directory[3096] = {0};
	
	uint8_t y4m_header_buffer[1024];
	int ret;
	if (!file_path)
	{
		yuv_log(LOG_ERROR, "file path is null %s", file_path);
		return NULL;
	}
	
	y4m_file = fopen(file_path, "rb+");
	
	if (!y4m_file)
	{
		yuv_log(LOG_ERROR, "fopen error : %s", file_path);
		return NULL;
	}
	
	yuv_get_file_name(file_path, file_name);
	file_directory = yuv_get_file_directory(file_path);
	sprintf(file_sub_directory, "%s%s%s", file_directory, "/", file_name);
	mkdir(file_sub_directory);
	
	y4m_context = y4m_context_alloc();
	strcpy(y4m_context->file_name, file_name);
	strcpy(y4m_context->file_sub_directory, file_sub_directory);
	strcpy(y4m_context->y4m_filepath, file_path);
	y4m_context->y4m_file = y4m_file;

	memset(y4m_header_buffer, 0, sizeof(y4m_header_buffer));
	ret = y4m_read_header(y4m_file, y4m_header_buffer);
	if (ret < 0)
		return NULL;
	fseek(y4m_file, ret+1, SEEK_CUR);
	ret = y4m_parse_header(y4m_context->header, y4m_header_buffer, ret);
	if (ret < 0)
		return NULL;
	
	y4m_context->yuv_info_file = y4m_get_yuv_info_file(y4m_context->file_sub_directory, y4m_context->file_name);
	y4m_print_yuv_info(y4m_context);
	return y4m_context;
}

int read_frame(Y4MContext *y4m_context)
{
	FILE *yuv_file = NULL;
	Y4MHeader *header = y4m_context->header;
	size_t frame_size;
	int len;
	
	frame_size = y4m_get_one_frame_size(header->format, header->width, header->height);
	yuv_file = y4m_open_yuv_file(y4m_context->file_sub_directory, y4m_context->file_name);
	uint8_t *frame_buffer = (uint8_t *)calloc(frame_size, 1);
	memset(frame_buffer, 0, frame_size);
	while ((len = y4m_read_on_frame(y4m_context, frame_buffer, frame_size)) > 0)
	{
		yuv_log(LOG_INFO, "read on frame, start write to yuv file");
		fwrite(frame_buffer, 1, frame_size, yuv_file);
		y4m_context->frame_count ++;
	}
	free(frame_buffer);
	return 0;
}

int close_context(Y4MContext *y4m_context) 
{
	if (y4m_context)
	{
		if (y4m_context->yuv_info_file)
		{
			fclose(y4m_context->yuv_info_file);
			y4m_context->yuv_info_file = NULL;
		}
		if (y4m_context->yuv_file)
		{
			fclose(y4m_context->yuv_file);
			y4m_context->yuv_file = NULL;
		}
		if (y4m_context->y4m_file)
		{
			fclose(y4m_context->y4m_file);
			y4m_context->y4m_file = NULL;
		}
		y4m_context_close(y4m_context);
		y4m_context = NULL;
	}
	return 0;
}

int main(int argc, char* argv[])
{
	Y4m_Yuv_Opts *opts = (Y4m_Yuv_Opts *)calloc(1, sizeof(Y4m_Yuv_Opts));
	set_log_level(LOG_INFO);

	yuv_log(LOG_INFO, "start to parse y4m");

	parse_cmd_opt(argc, argv, opts);

	/*
	if (argc <= 1)
	{
		yuv_log(LOG_ERROR, "error...............");
	}

	Y4MContext *y4m_context = init(argv[1]);
	read_frame(y4m_context);
	close_context(y4m_context);
	*/
	getchar();
	return 0;
}

