/**
 * @file esp_jpeg.c
 *
 * ESP-IDF's component takes a JPEG image and converts it to RGB888 data. This 
 * component is based on the Tiny JPEG Decompessor, which has low memory 
 * consumption and is highly optimized for small embedded systems.
 *
 * MIT License
 *
 * Copyright (c) 2023 phonght32
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <string.h>

#include "esp_log.h"
#include "esp_heap_caps.h"
#include "tjpgd.h"

#include "esp_jpeg.h"

/**
 * @macro   Tiny JPEG Decompressor workspace.
 */
#define TJPGD_WORKSPACE_SIZE 3100

/**
 * @macro   ESP JPEG check.
 */
#define ESPJPEG_CHECK(a, str, action) if(!(a)) {                             	\
    ESP_LOGE(TAG, "%s:%d (%s):%s", __FILE__, __LINE__, __FUNCTION__, str);  \
    action;                                                                 \
}

/**
 * @struct 	Structure stores the ESP JPEG information.
 *
 * @param 	workspace Tiny JPEG Decompressor workspace.
 * @param 	file_path File path.
 * @param 	file_pos Current file position referenced to.
 * @param 	width Image width in pixel.
 * @param 	height Image height in pixel.
 * @param 	buffer Pointer references to the image data.
 */
typedef struct esp_jpeg {
	uint8_t 			*workspace;
	char 				*file_path;
	uint64_t 			file_pos;
	uint16_t 			width;
	uint16_t 			height;
	uint8_t				*buffer;
} esp_jpeg_t;

/**
 * @var 	Global variable stores the ESP JPEG module information.
 */
static esp_jpeg_t *g_jpegdec;

/**
 * @brief   Module tag that is displayed in ESP_LOG.
 */
static const char *TAG = "ESP_JPEG";

/**
 * @func    _jd_input
 */
static uint16_t _jd_input(JDEC *jdec, uint8_t *buf, uint16_t len)
{
	FILE *fp = NULL;
	fp = fopen(g_jpegdec->file_path, "r");
	if (fp == NULL) {
		ESP_LOGE(TAG, "Error open file %s", g_jpegdec->file_path);
		return 0;
	}

	fseek(fp, 0, SEEK_END);
	long sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	if (sz <= 0) {
		fclose(fp);
		ESP_LOGE(TAG, "Error no content size %ld", sz);
		return 0;
	}

	uint32_t bytes_left = sz - g_jpegdec->file_pos;
	if (bytes_left < len) len = bytes_left;

	if (buf) {
		char *content = heap_caps_calloc(1, sz + 16, MALLOC_CAP_SPIRAM);
		fread(content, 1, sz, fp);
		memcpy(buf, content + g_jpegdec->file_pos, len);
		free(content);
	}

	fclose(fp);
	g_jpegdec->file_pos += len;

	return len;
}

/**
 * @func    _jd_output
 */
static uint16_t _jd_output(JDEC *jdec, void *bitmap, JRECT *rect)
{
	uint8_t *in = (uint8_t *)bitmap;

	for (int y = rect->top; y <= rect->bottom; y++) {
		for (int x = rect->left; x <= rect->right; x++) {
			uint8_t *p = g_jpegdec->buffer + (x + y * g_jpegdec->width) * 3;
			p[0] = in[0];
			p[1] = in[1];
			p[2] = in[2];
			in += 3;
		}
	}

	return 1;
}

/**
 * @func    esp_jpeg_init
 */
esp_jpeg_handle_t esp_jpeg_init(uint8_t *buffer)
{
	ESPJPEG_CHECK(buffer, "error buffer null", return NULL);

	g_jpegdec = calloc(1, sizeof(esp_jpeg_t));
	ESPJPEG_CHECK(g_jpegdec, "error handle null", return NULL);

	g_jpegdec->workspace = calloc(TJPGD_WORKSPACE_SIZE, sizeof(uint8_t));
	g_jpegdec->file_path = calloc(29, sizeof(uint8_t));
	g_jpegdec->file_pos = 0;
	g_jpegdec->buffer = buffer;

	return g_jpegdec;
}

/**
 * @func    esp_jpeg_set_file
 */
esp_err_t esp_jpeg_set_file(esp_jpeg_handle_t handle, const char *file_path)
{
	ESPJPEG_CHECK(handle, "error handle null", return ESP_ERR_INVALID_ARG);

	JDEC jdec;
	JRESULT jresult;

	if (handle->file_path) {
		free(handle->file_path);
		handle->file_path = NULL;
	}

	handle->file_path = strdup(file_path);
	handle->file_pos = 0;

	jresult = jd_prepare(&jdec, _jd_input, g_jpegdec->workspace, TJPGD_WORKSPACE_SIZE, 0);
	if (jresult != JDR_OK) {
		ESP_LOGE(TAG, "Image decoder: jd_prepare failed (%d)", jresult);
		return ESP_ERR_NOT_SUPPORTED;
	}

	handle->width = jdec.width;
	handle->height = jdec.height;

	jresult = jd_decomp(&jdec, _jd_output, 0);
	if (jresult != JDR_OK && jresult != JDR_FMT1) {
		ESP_LOGE(TAG, "Image decoder: jd_decode failed (%d)", jresult);
		return ESP_ERR_NOT_SUPPORTED;
	}

	ESP_LOGI(TAG, "Set image %s", file_path);

	return ESP_OK;
}