#include <Arduino.h>
#include <ArduinoJson.h>
#include <TimerOne.h>

DynamicJsonDocument doc(1024);

#define U8GLIB_H
//#define U8G2LIB_H

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif
#ifdef U8G2LIB_H
#include <U8g2lib.h>
U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* CS=*/ 10, /* reset=*/ 8);
#endif
#ifdef U8GLIB_H
#include <U8glib.h>
U8GLIB_KS0108_128 u8g(3, 4, 5, 6, 7, 8, 9, 10, 2, 11, 12, A2, A1); // 8Bit Com: DB0..DB7: 3,4,5,6,7,8,9,10, en=2, cs1=11, cs2=12,di=A2,rw=A1 || wiring 12864a display uchun
#endif

float weight = 75.4;
float height = 178.9;
float temp   = 36.7;
bool js = false;
String jsData = "";

void serialRequest () {
  Serial.print("r");
}

void setup(void) {
  #ifdef U8GLIB_H
    if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {
      u8g.setColorIndex(255);     // white
    }
    else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) {
      u8g.setColorIndex(3);         // max intensity
    }
    else if ( u8g.getMode() == U8G_MODE_BW ) {
      u8g.setColorIndex(1);         // pixel on
    }
    else if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
      u8g.setHiColorByRGB(255,255,255);
    }
    pinMode(8, OUTPUT);
  #endif
  #ifdef U8G2LIB_H
    u8g2.begin();
  #endif
  Serial.begin(115200);
  Timer1.initialize(200000);
  Timer1.attachInterrupt(serialRequest);
  interrupts();
  drawData(weight, height, getType(height/100, weight));
  delay(3000);
}

void loop(void) {
  if (Serial.available()) {
    char a = Serial. read();
    if (a == '{') {
      jsData = "";
      js = true;
    } else if (a == '}') {
      jsData += a;
      js = false;
    } else if (a == '\r' || a == '\n' || a == '\t' || a == ' ') {
      return;
    }
    if (js && a) {
      jsData += a;
    } else if (!js) {
      deserializeJson(doc, jsData);
      JsonObject obj = doc.as<JsonObject>();
      weight = obj[String("weight")];
      height = obj[String("height")];
      temp = obj[String("temp")];
      drawData(weight, height, getType(height/100, weight));
      jsData = "";
    }
  }
}

void drawData(float w, float h, const char* t) {
  char wght[5], hght[5];
  dtostrf(w, 3, 1, wght);
  dtostrf(h, 3, 1, hght);
  #ifdef U8G2LIB_H
    u8g2.clearBuffer(); 
    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.drawStr(3, 14, "Bo'y:");       u8g2.drawStr(52, 14, hght);         u8g2.drawStr(100, 14, "sm"); 
    u8g2.drawStr(3, 37, "Vazn:");       u8g2.drawStr(52, 37, wght);         u8g2.drawStr(100, 37, "kg");
    u8g2.drawStr(3, 61, "TVI:");        u8g2.setFont(u8g2_font_ncenB08_tr); u8g2.drawStr(45, 61, t);
    u8g2.sendBuffer();
  #endif
  #ifdef U8GLIB_H
    u8g.firstPage();  
    do {
      u8g.setFont(u8g_font_timB12);
      u8g.drawStr(3, 14, "Bo'y:");       u8g.drawStr(52, 14, hght);         u8g.drawStr(100, 14, "sm"); 
      u8g.drawStr(3, 37, "Vazn:");       u8g.drawStr(52, 37, wght);         u8g.drawStr(100, 37, "kg");
      u8g.drawStr(3, 61, "TVI:");        u8g.setFont(u8g_font_timB10);      u8g.drawStr(45, 61, t);
    } while(u8g.nextPage());
  #endif
}

const char* getType(float h, float w) {
  float tvi = w/(h * h);  //height is not centimeter maybe meter
  if (tvi <= 18.0) {
    return "Yetishmasligi";
  } else if (tvi > 18.0 && tvi <= 20.0) {
    return "Kamligi";
  } else if (tvi > 20.0 && tvi <= 25.0) {
    return "Normal";
  } else if (tvi > 25.0 && tvi <= 27.0) {
    return "Yuqoriligi";
  } else if (tvi > 27.0 && tvi <= 31.0) {
    return "1-daraja";
  } else if (tvi > 31.0 && tvi <= 36.0) {
    return "2-daraja";
  } else if (tvi > 36.0 && tvi <= 41.0) {
    return "3-daraja";
  } else {
    return "4-daraja";
  }
}
