#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "esp_heap_caps.h"
#include "esp_jpeg.h"

#define TJPGD_WORKSPACE_SIZE 3100

typedef struct {
	uint8_t 	workspace[TJPGD_WORKSPACE_SIZE] __attribute__((aligned(4)));
	const char 	*file_path;
	uint64_t 	file_pos;
	int16_t 	jpeg_x;
	int16_t 	jpeg_y;
	uint8_t 	jpeg_scale;
} esp_jpeg_t;

static esp_jpeg_t *g_jpegdec;

uint16_t jd_input(JDEC* jdec, uint8_t* buf, uint16_t len)
{
	g_jpegdec->file_path = (const char*)"/spiffs/panda.jpg";

	FILE *fp = NULL;
	fp = fopen(g_jpegdec->file_path, "r");
	if (fp == NULL) {
		fprintf(stderr, "Error open file %s\n", g_jpegdec->file_path);
		return 0;
	}

	fseek(fp, 0, SEEK_END);
	long sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	if (sz <=0) {
		fclose(fp);
		fprintf(stderr, "Error no content size %ld\n", sz);
		return 0;
	}

	uint32_t bytes_left = sz - g_jpegdec->file_pos;
	if (bytes_left < len) len = bytes_left;

	if (buf) {
		char *content = heap_caps_calloc(1, sz + 16, MALLOC_CAP_SPIRAM);
		fread(content, 1, sz, fp);
		memcpy(buf, content+g_jpegdec->file_pos, len);
		free(content);
	}

	g_jpegdec->file_pos += len;

	return len;
}

esp_err_t esp_jpeg_init(void)
{
	g_jpegdec = calloc(1, sizeof(esp_jpeg_t));
	g_jpegdec->file_path = calloc(29, sizeof(uint8_t));

	return ESP_OK;
}

esp_err_t esp_jpeg_get_size(const char *fp, uint16_t *w, uint16_t *h)
{
	JDEC jdec;
	esp_err_t err = ESP_OK;

	*w = 0;
	*h = 0;

	err = jd_prepare(&jdec, jd_input, g_jpegdec->workspace, TJPGD_WORKSPACE_SIZE, 0);

	if (err == ESP_OK) {
		*w = jdec.width;
		*h = jdec.height;
	}

	return err;
}
