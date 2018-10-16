
#ifndef Y4M_YUV_H
#define Y4M_YUV_H

// RGB -> YUV 
// Y = 0.299R + 0.587G + 0.114B
// Cb = 0.564(B - Y ) 
// Cr = 0.713(R - Y )

// YUV -> RGB
// R = Y + 1.402Cr
// G = Y - 0.344Cb - 0.714Cr 
// B = Y + 1.772Cb

typedef enum
{
	YUV_420,
	YUV_422,
	YUV_444,
	YUV_411
} YUV_FORMAT;

typedef struct Y4MHeader
{
	int width;
	int height;
	double frame_rate;
	char mode[100];
	char pixel_aspect_ratio[20];
	int format;
} Y4MHeader;

typedef struct Y4MContext
{
	Y4MHeader *header;
	char y4m_filepath[3096];
	char file_name[1024];
	char file_sub_directory[3096];
	uint32_t frame_count;
	FILE *y4m_file;
	FILE *yuv_file;
	FILE *yuv_info_file;
} Y4MContext;

int y4m_read_header(FILE *y4m_file, uint8_t *y4m_header_buffer);

Y4MContext *y4m_context_alloc();

int y4m_parse_header(Y4MHeader *header, uint8_t *y4m_header_buffer, int header_size);

void y4m_context_close(Y4MContext *y4m_context);

void y4m_print_yuv_info(Y4MContext *y4m_context);

FILE *y4m_get_yuv_info_file(char *file_directory, char *file_name);

FILE *y4m_open_yuv_file(char *file_directory, char *file_name);

size_t y4m_read_on_frame(Y4MContext *y4m_context, uint8_t *frame_buffer, size_t size);

size_t y4m_get_one_frame_size(int frame_format, int width, int height);

#endif