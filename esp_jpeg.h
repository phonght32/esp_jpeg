#ifndef _ESP_JPEG_H_
#define _ESP_JPEG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"
#include "tjpgd.h"

typedef struct esp_jpeg *esp_jpeg_handle_t;

esp_jpeg_handle_t esp_jpeg_init(uint8_t *buffer);
esp_err_t esp_jpeg_set_file(esp_jpeg_handle_t handle, const char *file_path);

#ifdef __cplusplus
}
#endif

#endif /* _ESP_JPEG_H_ */