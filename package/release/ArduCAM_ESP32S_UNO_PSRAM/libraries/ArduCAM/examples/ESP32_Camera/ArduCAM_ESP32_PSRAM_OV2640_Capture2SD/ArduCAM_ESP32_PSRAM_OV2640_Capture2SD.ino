// ArduCAM demo (C)2019 Lee
// Web: http://www.ArduCAM.com
// This program is a demo of how to use most of the functions
// of the library with a OV2640 digital camera, and can run on ArduCAM ESP32S UNO PSRAM platform.
// This demo will run the OV2640 digital camera on ArduCAM ESP32S UNO PSRAM platform, provide both JPEG capture
// and store it to SD card
// This demo sketch will do the following tasks:
// 1. Set the sensor to JPEG mode.
// 2. Capture every 5 seconds 
// 3. Store the image to Micro SD/TF card with JPEG format in sequential.
// 4. Resolution can be changed by config.frame_size.
// This program requires Arduino IDE 1.8.1 compiler or above

/*the config.frame_size can be set:
    FRAMESIZE_QQVGA,    // 160x120
    FRAMESIZE_QVGA,     // 320x240
    FRAMESIZE_VGA,      // 640x480
    FRAMESIZE_SVGA,     // 800x600
    FRAMESIZE_SXGA,     // 1280x1024
    FRAMESIZE_UXGA,     // 1600x1200
*/ 
#include "SD_MMC.h"
#include "arducam_esp32s_camera.h"

int i = 0;
char path[30];  //array for file path
    
void setup(){
    Serial.begin(115200);
    Serial.println("ArduCAM Start");
    // camera init
    esp_err_t err = arducam_camera_init(PIXFORMAT_JPEG);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }
    else
        Serial.printf("Camera init success");
    /*drop down frame size for higher initial frame rate*/
    sensor_t * s = arducam_camera_sensor_get();
    s->set_framesize(s, FRAMESIZE_QVGA);  
    /*SD Card init*/
    if(!SD_MMC.begin()){
          Serial.println("Card Mount Failed");
          return;
      }
    else{
          Serial.println("Card Mount success");
        }
      uint8_t cardType = SD_MMC.cardType();
    if(cardType == CARD_NONE){
          Serial.println("No SD_MMC card attached");
          return;
      }
    Serial.print("SD_MMC Card Type: ");
    if(cardType == CARD_MMC){
        Serial.println("MMC");
      } else if(cardType == CARD_SD){
          Serial.println("SDSC");
      } else if(cardType == CARD_SDHC){
          Serial.println("SDHC");
      } else {
          Serial.println("UNKNOWN");
      }
    uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
    Serial.printf("SD_MMC Card Size: %lluMB\n", cardSize);
    //Create ArduCAM folder
    sprintf(path, "/ArduCAM");
    Serial.printf("Creating Dir: %s\n", path);
    if(SD_MMC.mkdir(path)){
        Serial.println("Dir created");
      } 
    else {
        Serial.println("mkdir failed");
      }   
 }

void loop() {
    /*capture to SD card every 5 seconds*/
    Capture2SD();
    delay(5000);
}

static esp_err_t Capture2SD(){
    camera_fb_t * fb = NULL;  
    esp_err_t res = ESP_OK;
    size_t fb_len = 0;
    uint8_t *fb_buf;
    int64_t fr_start = esp_timer_get_time();
    /*start capture*/
    Serial.println("\ncamera start capture");
    fb = arducam_camera_fb_get(); //obtain pointer to a frame buffer 
    if (!fb) {
        Serial.println("Camera capture failed");
        return ESP_FAIL;
    }
    else{
        Serial.println("Camera capture done");
      }
    fb_len = fb->len; //frame buffer length
    fb_buf = fb->buf; //pointer to frame buffer
    /*write data to SD*/
    sprintf(path, "/ArduCAM/%d.jpg", ++i);
    File f = SD_MMC.open(path, "w");
    if(!f){
      Serial.println("Failed to open file for writing");
      }
      f.write(fb_buf, fb_len);
      f.close();
    strcat(path, " written");
    Serial.printf("%s\r\n", path);
    arducam_camera_fb_return(fb); //return the frame buffer to be reused
    int64_t fr_end = esp_timer_get_time();
    Serial.printf("%d.JPG:%uByte %ums\r\n", i, (uint32_t)(fb_len), (uint32_t)((fr_end - fr_start)/1000));
    return res;
}
