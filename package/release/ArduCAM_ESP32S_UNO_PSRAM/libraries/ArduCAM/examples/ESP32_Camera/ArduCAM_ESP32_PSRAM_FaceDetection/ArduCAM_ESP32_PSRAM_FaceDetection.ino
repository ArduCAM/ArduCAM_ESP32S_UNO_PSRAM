// ArduCAM ESP32S UNO PSRAM demo
// Web: http://www.ArduCAM.com
// This program is a demo of how to use most of the functions
// of the library with esp32 camera, and can run on any Arduino platform.
//
// This demo needs to be used in combination with PC software.
// It can operation face detection and face recognition,the image through the web page to view,
// and send commands through the web page to adjust resolution,brightness, exposure time and other parameters.
// This program requires Arduino IDE 1.8.1 compiler or above

#include "arducam_esp32s_camera.h"
#include <WiFi.h>

/*  OV2640 : PIXFORMAT_JPEG
 *  OV7725 : PIXFORMAT_RGB565
 *  OV7670 : PIXFORMAT_RGB565
 */
 

const char* ssid = "YOUR SSID";
const char* password = "YOUR PASSWORD";

const char* ap_ssid = "arducam_esp32s_camera";
const char* ap_password = "";

void startCameraServer();
uint8_t wifi_mode = 0;   //1:station mode, 0:access point mode

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  // camera init
  esp_err_t err = arducam_camera_init(PIXFORMAT_JPEG);
  if (err != ESP_OK) {
      Serial.printf("Camera init failed with error 0x%x", err);
      return;
  }
  else
      Serial.printf("Camera init success");
  //set frame size 
  //arducam_set_resolution(FRAMESIZE_QVGA);
    sensor_t * s = arducam_camera_sensor_get();
    s->set_framesize(s, FRAMESIZE_QVGA);
  if(wifi_mode){
    WiFi.begin(ssid, password);
  
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
  
    startCameraServer();
  
    Serial.print("Camera Ready! Use 'http://");
    Serial.print(WiFi.localIP());
    Serial.println("' to connect");
  }
  else{
    Serial.begin(115200);
    Serial.println();
    Serial.println("Configuring access point...");
  
    // You can remove the password parameter if you want the AP to be open.
    WiFi.softAP(ap_ssid, ap_password);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    startCameraServer();
    
    Serial.println("Server started");
    }
}

void loop() {

}
