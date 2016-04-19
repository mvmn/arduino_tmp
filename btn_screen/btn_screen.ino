#include <SPI.h>
#include <MySensor.h>
#include <Time.h>
#include <DS3232RTC.h>  // A  DS3231/DS3232 library
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//MySensor gw;
boolean timeReceived = false;
unsigned long lastUpdate = 0, lastRequest = 0;
int pushButton = 4;

// Initialize display. Google the correct settings for your display.
// The follwoing setting should work for the recommended display in the MySensors "shop".
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

uint8_t charDefs[][8]  = {
  {
    B00011111,
    B00011111,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000
  }, {
    B00011111,
    B00011111,
    B00011111,
    B00011111,
    B00000000,
    B00000000,
    B00000000,
    B00000000
  }, {
    B00011111,
    B00011111,
    B00011111,
    B00011111,
    B00011111,
    B00011111,
    B00000000,
    B00000000
  }, {
    B00011111,
    B00011111,
    B00011111,
    B00011111,
    B00011111,
    B00011111,
    B00011111,
    B00011111
  }, {
    B00000000,
    B00000000,
    B00011111,
    B00011111,
    B00011111,
    B00011111,
    B00011111,
    B00011111
  }, {
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00011111,
    B00011111,
    B00011111,
    B00011111
  }, {
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00011111,
    B00011111
  }, {
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000
  }
};

void setup()
{
  Serial.begin(9600);
  pinMode(pushButton, INPUT);
  // initialize the lcd for 16 chars 2 lines and turn on backlight
  lcd.begin(16, 2);

  for (int i = 0; i < 7; i++) {
    lcd.createChar(i, charDefs[i]);
  }
}

int playDelay = 150;

int score = 0;
int scr_len = 12;
int block_offset = scr_len / 4;
int scr_pos = 0;

int blocks[8];

int height = 12;
int vspeed = 16;

bool gameOver = true;

int lastButtonState = 0;
void loop()
{
  if (gameOver) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Game over");
    drawScore();
    int buttonState = digitalRead(pushButton);
    if (buttonState == 1 && lastButtonState != buttonState) {
      gameOver = false;
      gameInit();
    }
    lastButtonState = buttonState;
    delay(500);
  } else {
    play();
  }
}

void gameInit() {
  lcd.clear();
  score = 0;
  scr_pos = 0;
  height = 12;
  vspeed = 16;
  for (int i = 0; i < 8; i++) {
    blocks[i] = -1;
  }
  //  genBlock(0);
  //  genBlock(1);
  //  genBlock(2);
  //  genBlock(3);
}

int moveMul = 3;
void play() {
  int buttonState = digitalRead(pushButton);
  //unsigned long now = millis();

  scr_pos++;
  scr_pos = scr_pos % (block_offset * moveMul);
  // Serial.write("scr_pos: ");
  // Serial.println(scr_pos);

  //lcd.clear();
  if (scr_pos % moveMul == 0) {
    drawBlocks();
  }
  drawBird();

  checkCollision();

  if (buttonState == 0) {
    if (buttonState != lastButtonState) {
      vspeed = 16;
    } else {
      vspeed++;
    }
    if (vspeed > 30) {
      vspeed = 30;
    }
  } else {
    if (buttonState != lastButtonState) {
      vspeed = 16;
    } else {
      vspeed--;
    }
    if (vspeed < 10) {
      vspeed = 10;
    }
  }

  height += 8;
  height -= vspeed/2;
  if (height > 16) {
    height = 16;
  }
  if (height < 0 ) {
    height = 12;
    vspeed = 16;
    Serial.println("Fallen.");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Fall x_X");
    delay(1000);
    gameOver = true;
  }
  //  Serial.write("Height: ");
  //  Serial.println(height);
  //  Serial.write("Vspeed: ");
  //  Serial.println(vspeed);

  drawScore();

  lastButtonState = buttonState;
  delay(playDelay);
}

void checkCollision() {
  if (scr_pos >= (block_offset - 1) * moveMul && scr_pos < block_offset * moveMul && blocks[0] > -1) {
    // Serial.println("Checking collision.");
    bool collided = false;
    if (height > 8) {
      int block = blocks[0];
      // Serial.println("Collision check");
      // Serial.println(height);
      // Serial.println(block);
      // Serial.println(8 - (block + 1) * 2);
      if (block > 2 || (height - 8) > (8 - (block + 1) * 2)) {
        collided = true;
      }
    } else {
      int block = blocks[1];
      // Serial.println("Collision check");
      // Serial.println(height);
      // Serial.println(block);
      // Serial.println(8 - (block - 3) * 2);
      if (block < 4 || height < (8 - ((block - 3) * 2))) {
        collided = true;
      }
    }
    if (collided) {
      Serial.println("Collided.");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Smack x_X");
      delay(1000);
      gameOver = true;
    } else {
      if ((scr_pos + 1) % moveMul == 0) {
        score++;
      }
    }
  }
}

void drawBlocks() {
  if (scr_pos == 0) {
    for (int i = 0; i < 3; i++) {
      blocks[0 + i * 2] = blocks[2 + i * 2];
      blocks[1 + i * 2] = blocks[3 + i * 2];
    }
    genBlock(3);
  }
  lcd.setCursor(1, 0);
  int blockIndexInit = 0;
  if ((1 + (scr_pos - scr_pos % moveMul) / moveMul) % block_offset == 0) {
    blockIndexInit = 1;
  }
  int blockIndex = blockIndexInit;
  //
  for (int i = 1; i < scr_len; i++) {
    int block = -1;
    // Serial.println(i + (scr_pos - scr_pos % 3) / 3);
    if ((i + 1 + (scr_pos - scr_pos % moveMul) / moveMul) % block_offset == 0) {
      // Serial.print("Block check: ");
      // Serial.println(blockIndex * 2);
      block = blocks[blockIndex * 2];
      blockIndex++;
    }
    if (block > -1) {
      // Serial.print("Block: ");
      // Serial.println(block);
      lcd.write(block);
    } else {
      lcd.write(' ');
    }
  }
  lcd.setCursor(1, 1);
  blockIndex = blockIndexInit;
  for (int i = 1; i < scr_len; i++) {
    // Serial.print("Iterating bottom: ");
    // Serial.println(i);
    int block = -1;
    if ((i + 1 + (scr_pos - scr_pos % moveMul) / moveMul) % block_offset == 0) {
      // Serial.print("Block check: ");
      // Serial.println(blockIndex*2+1);
      block = blocks[blockIndex * 2 + 1];
      blockIndex++;
    }
    if (block > -1) {
      // Serial.print("Block: ");
      // Serial.println(block);
      lcd.write(block);
    } else {
      lcd.write(' ');
    }
  }
}

void genBlock(int bnum) {
  int r1, r2;
  r1 = 3;
  r2 = 2;
  while (r2 - r1 < 2) {
    r1 = random(3);
    r2 = 3 + random(3);
  }
  blocks[bnum * 2] = r1;
  blocks[bnum * 2 + 1] = r2;
}

void drawScore() {
  int len = 16 - scr_len;
  int pos = 16 - len;
  int cpos = pos;
  while (cpos++ < 16) {
    lcd.setCursor(cpos, 0);
    lcd.print(' ');
  }
  lcd.setCursor(pos, 0);
  lcd.print(score);
}

void drawBird() {
  int chardefBird = 7;
  int chardefSpace = -1;
  if (scr_pos >= (block_offset - 1) * moveMul && scr_pos < block_offset * moveMul && blocks[0] > -1) {
    if (height > 8) {
      chardefBird = blocks[0];
      chardefSpace = blocks[1];
    } else {
      chardefBird = blocks[1];
      chardefSpace = blocks[0];
    }
  }
  int cdUp, cdDown;
  if (height > 8) {
    defineBird(height - 8, chardefBird);
    cdUp = 7;
    cdDown = chardefSpace;
  } else {
    defineBird(height, chardefBird);
    cdUp = chardefSpace;
    cdDown = 7;
  }
  lcd.setCursor(0, 0);
  if (cdUp > -1 ) {
    lcd.write(cdUp);
  } else {
    lcd.print(' ');
  }
  lcd.setCursor(0, 1);
  if (cdDown > -1 ) {
    lcd.write(cdDown);
  } else {
    lcd.print(' ');
  }
}

uint8_t birdCharDef[8];

void defineBird(int height, int chardefOver) {
  height = 8 - height;
  for (int i = 0; i < 8; i++) {
    birdCharDef[i] = charDefs[chardefOver][i];
    if (i == height) {
      birdCharDef[i] = B00011111;
    }
  }

  lcd.createChar(7, birdCharDef);
}

