#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_log.h"
#include "esp_heap_caps.h"
#include "esp_jpeg.h"

#define mutex_lock(x)       while (xSemaphoreTake(x, portMAX_DELAY) != pdPASS);
#define mutex_unlock(x)     xSemaphoreGive(x)
#define mutex_create()      xSemaphoreCreateMutex()
#define mutex_destroy(x)    vQueueDelete(x)

#define TJPGD_WORKSPACE_SIZE 3100

#define ESPJPEG_CHECK(a, str, action) if(!(a)) {                             	\
    ESP_LOGE(TAG, "%s:%d (%s):%s", __FILE__, __LINE__, __FUNCTION__, str);  \
    action;                                                                 \
}

typedef struct esp_jpeg {
	uint8_t 			workspace[TJPGD_WORKSPACE_SIZE] __attribute__((aligned(4)));
	const char 			*file_path;
	uint64_t 			file_pos;
	int16_t 			jpeg_x;
	int16_t 			jpeg_y;
	uint8_t 			jpeg_scale;
	uint8_t				*buffer;
	SemaphoreHandle_t 	lock;
} esp_jpeg_t;

static esp_jpeg_t *g_jpegdec;
static const char *TAG = "ESP_JPEG";

uint16_t jd_input(JDEC* jdec, uint8_t* buf, uint16_t len)
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

	if (sz <=0) {
		fclose(fp);
		ESP_LOGE(TAG, "Error no content size %ld", sz);
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

esp_jpeg_handle_t esp_jpeg_init(uint8_t *buffer)
{
	ESPJPEG_CHECK(buffer, "error buffer null", return NULL);

	g_jpegdec = calloc(1, sizeof(esp_jpeg_t));
	ESPJPEG_CHECK(g_jpegdec, "error handle null", return NULL);

	g_jpegdec->file_path = calloc(29, sizeof(uint8_t));
	g_jpegdec->file_pos = 0;
	g_jpegdec->jpeg_x = 0;
	g_jpegdec->jpeg_y = 0;
	g_jpegdec->jpeg_scale = 0;
	g_jpegdec->buffer = buffer;
	g_jpegdec->lock = mutex_create();

	return g_jpegdec;
}

esp_err_t esp_jpeg_set_file(esp_jpeg_handle_t handle, const char *file_path)
{
	ESPJPEG_CHECK(handle, "error handle null", return ESP_ERR_INVALID_ARG);

	mutex_lock(handle->lock);
	handle->file_path = file_path;
	mutex_unlock(handle->lock);

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
