/**
 * Copyright (c) 2021 fukuen.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 * 
 * This software is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with This software.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <M5Stack.h>
#include <WiFi.h>
//#include "time.h"
#include <Wire.h>
#include "WireSlave.h"
#include "CommandHandler.h"

#define SDA_PIN 21
#define SCL_PIN 22
#define I2C_SLAVE_ADDR 0x04

TFT_eSprite sprite = TFT_eSprite(&M5.Lcd);

WiFiClient client;

//HardwareSerial serial_ext(2); // Grove
//HardwareSerial serial_ext(0); // USB

char msg[200];

int debug = 0;

#define BUFFER_LENGTH 2049
uint8_t commandBuffer[BUFFER_LENGTH];
uint8_t responseBuffer[BUFFER_LENGTH];
int responseLength;


void dumpBuffer(const char* label, uint8_t data[], int length) {
  ets_printf("%s: ", label);

  for (int i = 0; i < length; i++) {
    ets_printf("%02x", data[i]);
  }

  ets_printf("\r\n");
}

void setDebug(int d) {
  debug = d;

  if (debug) {
    ets_printf("*** DEBUG ON\n");
  } else {

  }
}

void disp0(String cmd) {
  sprite.setScrollRect(0, 0, 310, 220, TFT_BLACK);
  sprite.scroll(0, -10);
  sprite.drawString(cmd, 0, 210);
  sprite.pushSprite(5, 15);
}

void disp1(String cmd) {
  sprite.setTextColor(TFT_WHITE);
  disp0(cmd);
}

void disp2(String cmd) {
  sprite.setTextColor(TFT_GREEN);
  disp0(cmd);
}

void error(String cmd) {
  sprite.setTextColor(TFT_RED);
  disp0(cmd);
}

void receiveEvent(int howMany) {
  size_t size = WireSlave.readBytes(commandBuffer, howMany);
  memset(msg, 0x00, sizeof(msg));
  sprintf(msg, "CMD %d %x %x", howMany, commandBuffer[0], commandBuffer[1]);
  disp1(msg);

  responseLength = CommandHandler.handle(commandBuffer, responseBuffer);

  memset(msg, 0x00, sizeof(msg));
  sprintf(msg, "RES %d %x %x", responseLength, responseBuffer[0], responseBuffer[1]);
  disp1(msg);
}

void requestEvent()
{
  if (responseLength > 0) {
    WireSlave.write(0x55);
    WireSlave.write(responseBuffer, responseLength);
    WireSlave.write(0);
    WireSlave.write(0);
  }
  responseLength = 0;
}

void setup() {
  M5.begin();
  M5.Lcd.fillScreen(TFT_NAVY);

  M5.Lcd.setTextColor(TFT_CYAN);
  M5.Lcd.drawString("ESP32 WiFi", 0, 0);

  sprite.setColorDepth(8);
  sprite.createSprite(310, 220);
  sprite.setCursor(0, 0);
  sprite.setTextFont(0);

  CommandHandler.begin();

  bool success = WireSlave.begin(SDA_PIN, SCL_PIN, I2C_SLAVE_ADDR);
  if (!success) {
    Serial.println("I2C slave init failed");
    while(1) delay(100);
  }

  WireSlave.onReceive(receiveEvent);
  WireSlave.onRequest(requestEvent);
}

void loop() {
  WireSlave.update();
  delay(1);

  // connect info
  if (WiFi.isConnected()) {
    M5.Lcd.setTextColor(TFT_GREEN);
    M5.Lcd.drawString("@", 300, 0);
  } else {
    M5.Lcd.setTextColor(TFT_RED);
    M5.Lcd.drawString("O", 300, 0);
  }
}

