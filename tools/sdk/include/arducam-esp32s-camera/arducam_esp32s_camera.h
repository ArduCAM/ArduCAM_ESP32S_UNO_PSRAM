#pragma once

#include "esp_camera.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the camera driver
 *
 * @param config  Camera configuration parameters
 *
 * @return ESP_OK on success
 */
esp_err_t arducam_camera_init(pixformat_t PIXFORMAT);

/**
 * @brief Deinitialize the camera driver
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_STATE if the driver hasn't been initialized yet
 */
esp_err_t arducam_camera_deinit();

/**
 * @brief Obtain pointer to a frame buffer.
 *
 * @return pointer to the frame buffer
 */
camera_fb_t* arducam_camera_fb_get();

/**
 * @brief Return the frame buffer to be reused again.
 *
 * @param fb    Pointer to the frame buffer
 */
void arducam_camera_fb_return(camera_fb_t * fb);

/**
 * @brief Get a pointer to the image sensor control structure
 *
 * @return pointer to the sensor
 */
sensor_t * arducam_camera_sensor_get();

/**
 * @brief Get a fram data
 *
 * @return pointer to the data
 */
uint8_t * arducam_camera_fram_get();

/**
 * @brief set resolution
 *
 * @return success return 0
 */
esp_err_t arducam_set_resolution(framesize_t res);

/**
 * @brief sccb init
 *
 * @return success return 0
 */
int arducam_sccb_init(int pin_sda, int pin_scl);

/**
 * @brief sccb detect the sensor address
 *
 * @return I2C the sensor address
 */
uint8_t arducam_sensor_detect(void);

/**
 * @brief write 8 bit value to 8 bit register address
 *
 * @return success return 0
 */
uint8_t arducam_wrSensorReg8_8(uint8_t reg, uint8_t data);

/**
 * @brief read 8 bit value from 8 bit register address
 *
 * @return success return 0
 */
uint8_t arducam_rdSensorReg8_8(uint8_t reg, uint8_t *regdat);

/**
 * @brief write 8 bit value from 16 bit register address
 *
 * @return success return 0
 */
uint8_t arducam_wrSensorReg16_8(uint16_t reg, uint8_t data);

/**
 * @brief read 8 bit value from 16 bit register address
 *
 * @return success return 0
 */
uint8_t arducam_rdSensorReg16_8(uint16_t reg, uint8_t *regdat);


#ifdef __cplusplus
}
#endif



