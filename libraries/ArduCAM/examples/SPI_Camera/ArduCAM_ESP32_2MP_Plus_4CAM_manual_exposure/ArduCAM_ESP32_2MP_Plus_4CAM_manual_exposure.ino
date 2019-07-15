// ArduCAM ESP32 demo
// Web: http://www.ArduCAM.com
// This program is a demo of how to use most of the functions
// of the library with ArduCAM Mini 2MP Plus camera, and can run on any Arduino platform.
//
// This demo was made for ArduCAM Mini Camera.
// It needs to be used in combination with PC software.It can take 4 photos at the same time with 4 cameras.
// The demo sketch will do the following tasks:
// 1. Set the 4 cameras to JPEG output mode.
// 2. Read data from Serial port and deal with it
// 3. If receive 0x00-0x08,the resolution will be changed.
// 4. If receive 0x15,cameras will capture and buffer the image to FIFO. 
// 5. Check the CAP_DONE_MASK bit and write datas to Serial port.
// This program requires the ArduCAM V4.0.0 (or later) library and ArduCAM Mini 2MP Plus camera
// and use Arduino IDE 1.6.8 compiler or above

#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "FS.h"
#include <ArduCAM.h>
#include "memorysaver.h"

//This demo can only work on OV2640_MINI_2MP_PLUS or OV5642_MINI_5MP_PLUS platform.
#if !(defined (OV2640_MINI_2MP_PLUS)||defined (OV5642_MINI_5MP_PLUS))
#error Please select the hardware platform and camera module in the ../libraries/ArduCAM/memorysaver.h file
#endif
#define   FRAMES_NUM    9    //0-255
#define   EXPOSURE_MS   50   //0-99.928

void set_exposure(ArduCAM myCAM);
void capture2SD(fs::FS &fs, ArduCAM myCAM);
// set pin 14,15,16,17 as the slave select for SPI:
int i, j;
static int  k = 1;
uint8_t temp = 0, temp_last = 0;
uint32_t length = 0;
bool is_header = false;
char pname[20];
byte buf[256];
const int CS1 = D4;
const int CS2 = D5;
const int CS3 = D6;
const int CS4 = D7;
const int CAM_POWER_ON = D10;
uint32_t tex = EXPOSURE_MS * 1000000;
uint16_t AEC = tex / (1922*41.66);
char exposure1 = AEC & 0x3;
char exposure2 = (AEC >> 2)& 0xff;
char exposure3 = (AEC >> 10)& 0x3f;

//the falgs of camera modules
bool cam1=true, cam2=true, cam3=true, cam4=true;
//the flag of JEPG data header
//the falg data of 4 cameras' data
byte flag[5]={0xFF,0xAA,0x01,0xFF,0x55};
int count = 0;

  ArduCAM myCAM1(OV2640,CS1);
  ArduCAM myCAM2(OV2640,CS2);
  ArduCAM myCAM3(OV2640,CS3);
  ArduCAM myCAM4(OV2640,CS4);
  
void setup() {
// put your setup code here, to run once:
uint8_t vid, pid;
uint8_t temp;
pinMode(CAM_POWER_ON, OUTPUT);
digitalWrite(CAM_POWER_ON, HIGH);
Wire.begin(); 
Serial.begin(115200);
Serial.println(F(" ArduCAM Start! ")); 
// set the CS output:
pinMode(CS1, OUTPUT);
digitalWrite(CS1, HIGH);
pinMode(CS2, OUTPUT);
digitalWrite(CS2, HIGH);
pinMode(CS3, OUTPUT);
digitalWrite(CS3, HIGH);
pinMode(CS4, OUTPUT);
digitalWrite(CS4, HIGH);
// initialize SPI:
SPI.begin();  
  if(!SD.begin()){
    Serial.println(" SD Card Mount Failed ");
    return;
   }else
    Serial.println(F(" SD Card detected! "));
//Reset the CPLD
myCAM1.write_reg(0x07, 0x80);
delay(100);
myCAM1.write_reg(0x07, 0x00);
delay(100);  
myCAM2.write_reg(0x07, 0x80);
delay(100);
myCAM2.write_reg(0x07, 0x00);
delay(100);  
myCAM3.write_reg(0x07, 0x80);
delay(100);
myCAM3.write_reg(0x07, 0x00);
delay(100);  
myCAM4.write_reg(0x07, 0x80);
delay(100);
myCAM4.write_reg(0x07, 0x00);
delay(100);  


while(1){
  //Check if the 4 ArduCAM Mini 2MP Cameras' SPI bus is OK
  myCAM1.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM1.read_reg(ARDUCHIP_TEST1);
  if(temp == 0x55)
  {
    Serial.println(F(" SPI1 interface Success! "));
  } 
  else
  cam1 = false;
  myCAM2.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM2.read_reg(ARDUCHIP_TEST1);
  if(temp == 0x55)
  {
    Serial.println(F(" SPI2 interface Success! "));
  }
   else
     cam2 = false; 
  myCAM3.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM3.read_reg(ARDUCHIP_TEST1);
  if(temp == 0x55)
  {
    Serial.println(F(" SPI3 interface Success! "));

  }
  else
      cam3 = false;
    
  myCAM4.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM4.read_reg(ARDUCHIP_TEST1);
  if(temp == 0x55)
  {
    Serial.println(F(" SPI4 interface Success! "));
  }
  else
    cam4 = false;
  if(!(cam1||cam2||cam3||cam4)){
    delay(1000);continue;
    }else
    break;
}
#if defined (OV2640_MINI_2MP_PLUS)
  while(1){
  //Check if the camera module type is OV2640
  myCAM1.wrSensorReg8_8(0xff, 0x01);
  myCAM1.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
  myCAM1.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
  if ((vid != 0x26 ) && (( pid != 0x41 ) || ( pid != 0x42 ))){
    Serial.println(F(" Can't find OV2640 module! "));
    delay(1000);continue;
  }else{
    Serial.println(F(" OV2640 detected. "));break;
   } 
}
#endif

//Change to JPEG capture mode and initialize the OV2640 module
Serial.println(F(" Set JPEG Format. "));
myCAM1.set_format(JPEG);
Serial.println(F(" OV2640 Init. "));
myCAM1.InitCAM();
Serial.println(F(" Set JPEG size 1600x1200. "));
myCAM1.OV2640_set_JPEG_size(OV2640_1600x1200);
delay(1000);
myCAM1.clear_fifo_flag();
myCAM2.clear_fifo_flag();
myCAM3.clear_fifo_flag();
myCAM4.clear_fifo_flag();
myCAM1.write_reg(ARDUCHIP_FRAMES, FRAMES_NUM);
myCAM2.write_reg(ARDUCHIP_FRAMES, FRAMES_NUM);
myCAM3.write_reg(ARDUCHIP_FRAMES, FRAMES_NUM);
myCAM4.write_reg(ARDUCHIP_FRAMES, FRAMES_NUM);
set_exposure(myCAM1);
Serial.println(F(" Waiting Capture.... "));
}

void loop() {
// put your main code here, to run repeatedly:  
if(cam1)
 capture2SD(SD, myCAM1);
if(cam2)
 capture2SD(SD, myCAM2);
if(cam3)
 capture2SD(SD, myCAM3);
if(cam4)
 capture2SD(SD, myCAM4); 
}

void set_exposure(ArduCAM myCAM)
{
  uint8_t temp;
  myCAM.rdSensorReg8_8(0x13, &temp);
//  Serial.print("temp ");
//  Serial.print(temp, HEX);
  temp &= ~(1 << 0);
  temp &= ~(1 << 2);
  myCAM.wrSensorReg8_8(0x13, temp);
//  myCAM.rdSensorReg8_8(0x13, &temp);
//  Serial.print("temp ");
//  Serial.print(temp, HEX);
 
  myCAM.rdSensorReg8_8(0x04, &temp);
//  Serial.print("temp ");
//  Serial.print(temp, HEX);
  temp = temp & 0XFC;
  myCAM.wrSensorReg8_8(0x04, temp | exposure1);
//  myCAM.rdSensorReg8_8(0x04, &temp);
//  Serial.print("temp ");
//  Serial.print(temp, HEX);
  
  myCAM.rdSensorReg8_8(0x10, &temp);
  temp = temp & 0;
  myCAM.wrSensorReg8_8(0x10, temp | exposure2);
  
  myCAM.rdSensorReg8_8(0x45, &temp);
  temp = temp & 0XE0;
  myCAM.wrSensorReg8_8(0x45, temp | exposure3);
}

void capture2SD(fs::FS &fs, ArduCAM myCAM){
 File file ; 
if(!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK))
  return ;
else {
  Serial.println(F(" Capture Done. "));  
  length = myCAM.read_fifo_length();
  Serial.print(F(" The fifo length is : "));
  Serial.println(length, DEC);
   if (length >= MAX_FIFO_SIZE) //8M
    {
      Serial.println(F(" Over size. "));
    }
      if (length == 0 ) //0 kb
    {
      Serial.println(F(" Size is 0. "));
    }
   i = 0;
   myCAM.CS_LOW();
   myCAM.set_fifo_burst(); 
    while ( length-- )
    {
      temp_last = temp;
      temp =  SPI.transfer(0x00);
      //Read JPEG data from FIFO
      if ( (temp == 0xD9) && (temp_last == 0xFF) ) //If find the end ,break while,
      {
          buf[i++] = temp;  //save the last  0XD9     
         //Write the remain bytes in the buffer
          myCAM.CS_HIGH();
          file.write(buf, i);    
        //Close the file
          file.close();
          Serial.printf(" Image %05d.jpg save OK.\n",(k-1));
          is_header = false;
          myCAM.CS_LOW();
          myCAM.set_fifo_burst();
          i = 0;   
      }  
      if (is_header == true)
      { 
         //Write image data to buffer if not full
          if (i < 256)
          buf[i++] = temp;
          else
          {
            //Write 256 bytes image data to file
            myCAM.CS_HIGH();
            file.write(buf, 256);
            i = 0;
            buf[i++] = temp;
            myCAM.CS_LOW();
            myCAM.set_fifo_burst();
          }        
      }
      else if ((temp == 0xD8) & (temp_last == 0xFF))
      {
        is_header = true;
        myCAM.CS_HIGH();
        sprintf((char*)pname,"/%05d.jpg",k);
        k++;
        file = fs.open(pname, FILE_WRITE);
        if(!file){
            Serial.println(" Failed to open file for writing ");
            return;
        }
        myCAM.CS_LOW();
        myCAM.set_fifo_burst(); 
        buf[i++] = temp_last;
        buf[i++] = temp;   
      } 
    } 
    //Clear the capture done flag 
    myCAM.flush_fifo();
    myCAM.clear_fifo_flag();
  }
}
