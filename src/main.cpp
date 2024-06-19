#include <Arduino.h>
#include <U8g2lib.h>


//I2C for SH1106 OLED 128x64
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

//Declaration
extern void ble_client_init();
extern void ble_client_handle();


void state_logo(){
  u8g2.clearBuffer();					// clear display buffer
  u8g2.setFont(u8g2_font_open_iconic_embedded_4x_t);//open iconic
  u8g2.drawStr(48,64-15,"N");
  u8g2.sendBuffer();
}

void setup() {

  Serial.begin(115200);

  //OLED SH1106 128x64
  u8g2.begin();
  state_logo();
  delay(2000);


  Serial.println();
  Serial.println("----------------Start Info-----------------");
  Serial.printf("Total heap:\t%d \r\n", ESP.getHeapSize());
  Serial.printf("Free heap:\t%d \r\n", ESP.getFreeHeap());
  Serial.printf("Total PSRAM:\t%d \r\n", ESP.getPsramSize());
  Serial.printf("Free PSRAM:\t%d \r\n", ESP.getFreePsram());
  Serial.printf("Flash size:\t%d (bytes)\r\n", ESP.getFlashChipSize());
  Serial.println("I2C_SDA= "+String(SDA));
  Serial.println("I2C_SCL= "+String(SCL));

  Serial.println("-------------------------------------------");

  
   
  ble_client_init();
  
}

void loop() {

  ble_client_handle();

  delay(5000); // Delay a second between loops.

}
