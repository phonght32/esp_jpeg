#ifndef _ESP_JPEG_H_
#define _ESP_JPEG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"
#include "tjpgd.h"

esp_err_t esp_jpeg_init(void);
esp_err_t esp_jpeg_get_size(const char *fp, uint16_t *w, uint16_t *h);


#ifdef __cplusplus
}
#endif

#endif /* _ESP_JPEG_H_ */