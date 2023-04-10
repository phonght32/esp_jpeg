/**
 * @file esp_jpeg.h
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

#ifndef _ESP_JPEG_H_
#define _ESP_JPEG_H_

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct  ESP JPEG handle structure.
 */
typedef struct esp_jpeg *esp_jpeg_handle_t;

/**
 * @brief  	Initialize ESP JPEG module.
 *
 * @note 	The buffer needs to be allocated before with malloc/calloc.
 * 
 * @param   buffer Pointer references to the image data. 
 *
 * @return  
 * 		- Pointer to the ESP JPEG handle structure.
 * 		- NULL: Failed.
 */
esp_jpeg_handle_t esp_jpeg_init(uint8_t *buffer);

/**
 * @brief   Set image path.
 * 
 * @note 	This module handles memory allocation for the file path. There is
 * 			no memory allocation action required.
 * 
 * @param   handle ESP JPEG handle structure.
 * @param 	file_path File path.
 *
 * @return  
 *      - ESP_OK:       			Success.
 *      - ESP_ERR_INVALID_ARG:  	ESP JPEG handle structure is NULL.
 * 		- ESP_ERR_NOT_SUPPORTED:	Can not decode image.
 */
esp_err_t esp_jpeg_set_file(esp_jpeg_handle_t handle, const char *file_path);

#ifdef __cplusplus
}
#endif

#endif /* _ESP_JPEG_H_ */