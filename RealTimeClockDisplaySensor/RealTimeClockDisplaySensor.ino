/**
   The MySensors Arduino library handles the wireless radio link and protocol
   between your home built sensors/actuators and HA controller of choice.
   The sensors forms a self healing radio network with optional repeaters. Each
   repeater and gateway builds a routing tables in EEPROM which keeps track of the
   network topology allowing messages to be routed to nodes.

   Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
   Copyright (C) 2013-2015 Sensnology AB
   Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors

   Documentation: http://www.mysensors.org
   Support Forum: http://forum.mysensors.org

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   version 2 as published by the Free Software Foundation.

 *******************************

   REVISION HISTORY
   Version 1.0 - Henrik Ekblad

   DESCRIPTION
   Example sketch showing how to request time from controller which is stored in RTC module
   The time and temperature (DS3231/DS3232) is shown on an attached Crystal LCD display


   Wiring (radio wiring on www.mysensors.org)
   ------------------------------------
   Arduino   RTC-Module     I2C Display
   ------------------------------------
   GND       GND            GND
   +5V       VCC            VCC
   A4        SDA            SDA
   A5        SCL            SCL

   http://www.mysensors.org/build/display

*/


#include <SPI.h>
#include <MySensor.h>
#include <Time.h>
#include <DS3232RTC.h>  // A  DS3231/DS3232 library
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

MySensor gw;
boolean timeReceived = false;
unsigned long lastUpdate = 0, lastRequest = 0;

// Initialize display. Google the correct settings for your display.
// The follwoing setting should work for the recommended display in the MySensors "shop".
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

uint8_t charDefs[8][8] = {
  {255, 255, 255, 255, 255, 255, 255, 255},
  {255, 255, 255, 255, 255, 255, 255, 255},
  {255, 255, 255, 255, 255, 255, 255, 255},
  {255, 255, 255, 255, 255, 255, 255, 255},
  {255, 255, 255, 255, 255, 255, 255, 255},
  {255, 255, 255, 255, 255, 255, 255, 255},
  {255, 255, 255, 255, 255, 255, 255, 255},
  {255, 255, 255, 255, 255, 255, 255, 255}
};


void setup()
{
  gw.begin();

  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo("RTC Clock", "1.0");

  // the function to get the time from the RTC
  setSyncProvider(RTC.get);

  // Request latest time from controller at startup
  gw.requestTime(receiveTime);

  // initialize the lcd for 16 chars 2 lines and turn on backlight
  lcd.begin(16, 2);

  updateCharDefs();
  for (int i = 0; i < 8; i++) {
    lcd.setCursor(12 + i % 4, i > 3 ? 1 : 0);
    lcd.write(i);
  }
  /*line(11, 8, 2, 12, 0);
    line(5, 0, 0, 17, 0);
    line(23, 0, 18, 17, 0);
    line(18, 0, 23, 17, 0);
    updateCharDefs();*/
}

void loop()
{
  unsigned long now = millis();
  gw.process();

  // If no time has been received yet, request it every 10 second from controller
  // When time has been received, request update every hour
  if ((!timeReceived && now - lastRequest > 10 * 1000)
      || (timeReceived && now - lastRequest > 60 * 1000 * 60)) {
    // Request time from controller.
    //Serial.println("requesting time");
    gw.requestTime(receiveTime);
    lastRequest = now;
  }

  // Update display every second
  if (now - lastUpdate > 1000) {
    //Serial.println("Update");
    lastUpdate = now;
    updateDisplay();
  }
}

// This is called when a new time value was received
void receiveTime(unsigned long controllerTime) {
  // Ok, set incoming time
  Serial.print("Time value received: ");
  Serial.println(controllerTime);
  RTC.set(controllerTime); // this sets the RTC to the time from controller - which we do want periodically
  timeReceived = true;
}

int pha = -1;
int pma = -1;
int psa = -1;
void updateDisplay() {
  tmElements_t tm;
  RTC.read(tm);
  // Print date and time
  lcd.home();
  lcd.setCursor ( 0, 1 );
  lcd.print(tmYearToCalendar(tm.Year));
  lcd.print("-");
  printDigits(tm.Month);
  lcd.print("-");
  printDigits(tm.Day);

  lcd.setCursor ( 0, 0 );
  printDigits(tm.Hour);
  lcd.print(":");
  printDigits(tm.Minute);
  //lcd.print(":");
  //printDigits(tm.Second);
  lcd.print(" ");
  printDigits(RTC.temperature() / 4);
  lcd.write(223); // Degree-sign
  lcd.print("C");

  wipeCharDefs();

  /*
    if (pha > -1 && pha != tm.Hour) {
    drawArrow(2, 10, 7, 9, 6, 60, pha);
    drawArrow(2, 10, 9, 9, 6, 60, pha);
    drawArrow(2, 12, 7, 9, 6, 60, pha);
    drawArrow(2, 12, 9, 9, 6, 60, pha);
    }
    pha = tm.Hour;*/
  drawArrow(0, 11, 8, 9, 6, 12, tm.Hour);
  drawArrow(0, 10, 8, 9, 6, 12, tm.Hour);
  drawArrow(0, 11, 9, 9, 6, 12, tm.Hour);
  drawArrow(0, 10, 9, 9, 6, 12, tm.Hour);

  /*
    if (pma > -1 && pma != tm.Minute) {
      drawArrow(2, 10, 8, 11, 8, 60, pma);
      drawArrow(2, 12, 8, 11, 8, 60, pma);
      drawArrow(2, 11, 7, 11, 8, 60, pma);
      drawArrow(2, 11, 9, 11, 8, 60, pma);
    }
    pma = tm.Minute;*/
  drawArrow(0, 11, 8, 11, 8, 60, tm.Minute);
  drawArrow(0, 12, 8, 11, 8, 60, tm.Minute);
  drawArrow(0, 11, 9, 11, 8, 60, tm.Minute);
  drawArrow(0, 12, 9, 11, 8, 60, tm.Minute);
  /*if (psa > -1 && psa != tm.Second) {
    drawArrow(2, 11, 8, 12, 9, 60, psa);
    }
    psa = tm.Second;*/
  drawArrow(0, 11, 8, 12, 9, 60, tm.Second);
  
  updateCharDefs();

  /*
    lcd.print(" ");
    printDigits(tm.Hour);
    lcd.print(":");
    printDigits(tm.Minute);
    lcd.print(":");
    printDigits(tm.Second);*/

}

float halfPi = 3.14159 / 2;
void drawArrow(int op, int x, int y, int hr, int vr, int vl, int v) {
  int vlq = vl / 4;
  int q = (v - v % vlq) / vlq;
  float angle = halfPi * (((float)(v - vlq * q)) / (float) vlq);
  int xt = ((q % 2 == 0) ? sin(angle) : cos(angle)) * hr;
  int yt = ((q % 2 == 0) ? cos(angle) : sin(angle)) * vr;

  xt = x + (q > 1 ? -xt : xt);
  yt = y + ((q > 0 && q < 3) ? yt : -yt);
  if (xt < 0) {
    xt = 0;
  } else if (xt > 22) {
    xt = 22;
  }
  if (yt < 0) {
    yt = 0;
  } else if (yt > 16) {
    yt = 16;
  }

  /*
    Serial.print("Arrow Xt/Yt: ");
    Serial.print(xt);
    Serial.print("/");
    Serial.print(yt);
    Serial.print(" ");
    Serial.print(vlq);
    Serial.print(" ");
    Serial.print(q);
    Serial.print(" ");
    Serial.print(v);
    Serial.print(" ");
    Serial.print(v - vlq * q);
    Serial.print(" ");
    Serial.println(angle);
  */
  line(x, y, xt, yt, op);
}


void printDigits(int digits) {
  if (digits < 10)
    lcd.print('0');
  lcd.print(digits);
}

void line(int xa, int ya, int xb, int yb, int op) {
  int tmp;
  if (xb < xa) {
    tmp = xa;
    xa = xb;
    xb = tmp;
    tmp = ya;
    ya = yb;
    yb = tmp;
  }
  int dx = xb - xa; // Never negative due to above
  int dy;
  boolean dyNegative;
  if (yb < ya) {
    dy = ya - yb;
    dyNegative = true;
  } else {
    dy = yb - ya;
    dyNegative = false;
  }

  /*Serial.write("Line from ");
    Serial.print(xa);
    Serial.write(":");
    Serial.print(ya);
    Serial.write(" to ");
    Serial.print(xb);
    Serial.write(":");
    Serial.print(yb);
    Serial.write(". dx=");
    Serial.print(dx);
    Serial.write(". dy=");
    Serial.print(dy);
    Serial.write(". dy negative ");
    Serial.println(dyNegative);*/
  if (dx == dy && dy == 0) {
    // Avoid div by zero
    changeBit(xa, ya, op);
  } else if (dx > dy) {
    for (int i = 0; i < dx; i++) {
      int y;
      if (dyNegative) {
        y = ya - (i * dy) / dx;
      } else {
        y = ya + (i * dy) / dx;
      }
      changeBit(xa + i, y , op);
    }
  } else {
    for (int i = 0; i < dy; i++) {
      int y;
      if (dyNegative) {
        y = ya - i;
      } else {
        y = ya + i;
      }
      changeBit(xa + (i * dx) / dy, y, op);
    }
  }
}

void changeBit(int x, int y, int op) {
  if (x < 0) {
    x = 0;
  } else if (x > 22) {
    x = 22;
  }
  if (y < 0) {
    y = 0;
  } else if (y > 16) {
    y = 16;
  }
  x++;
  y++;
  int cx = x % 6;
  int cy = y % 9;
  // Skip every 6-th screen colum and every 9-th screen row
  if (cx != 0 && cy != 0) {
    int row = (y - cy) / 9;
    int col = (x - cx) / 6;
    y = cy;
    x = cx;
    //x--;
    //uint8_t val = B00010000 >> x;
    y--;
    /*Serial.print("Changing bit at symbol ");
      Serial.print(row);
      Serial.print("x");
      Serial.print(col);
      Serial.print(" coord ");
      Serial.print(x-1);
      Serial.print("x");
      Serial.println(y);*/
    uint8_t val = B00100000 >> x;
    if (op == 1) {
      charDefs[row * 4 + col][y] |= val;
    } else if (op == 0) {
      charDefs[row * 4 + col][y] &= ~val;
    } else {
      charDefs[row * 4 + col][y] ^= val;
    }
  }
}

void wipeCharDefs() {
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      charDefs[i][j] = B00011111;
    }
  }
}
void updateCharDefs() {
  for (int i = 0; i < 8; i++) {
    lcd.createChar(i, charDefs[i]);
  }
}
