/*
  Simon Says Memory Game - Enhanced Edition
  © 2023-2025 Omar Alhalawani. All rights reserved.

  This software and its associated documentation files (the "Software") may be used for educational or personal purposes only.
  No part of this project may be copied, modified, or redistributed for commercial use without prior written permission.

  Created by: Omar Alhalawani
  Description: A multi-mode Simon Says handheld memory game implemented with Arduino.
               Includes Classic, Speed, Reverse, and Stealth modes with sound control,
               EEPROM-based high score saving, and debounced inputs.
  
  📬 Contact Information:
  Email: omar.alhalawani2006@gmail.com
  Phone: 343-598-5533
*/



#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include "pitches.h"

// Setup the LCD using I2C connection (address, columns, rows)
LiquidCrystal_I2C lcd(0x3F, 4, 5);

// Pins for the LEDs: Yellow, Green, Red, Blue
const byte ledPins[] = {4, 5, 6, 7};
// Pins for the buttons corresponding to each LED
const byte buttonPins[] = {0, 1, 2, 3};
// Pin connected to the buzzer for sounds
const byte speaker = 10;
// Extra button to reset the high score
const byte resetPin = 8;
// Maximum number of levels in the game
const byte MAX_GAME_LENGTH = 100;
// Different tones for each color/LED
const int gameTones[] = {NOTE_G3, NOTE_C4, NOTE_E4, NOTE_G5};

byte gameSequence[MAX_GAME_LENGTH] = {0};
byte Level = 0;
byte lives = 3;
const byte highScoreAddress = 0;
bool isMuted = false;

// These are the available game modes
enum GameMode { CLASSIC, SPEED, REVERSE, STEALTH };
GameMode currentMode = CLASSIC;

void flashAllLEDsSync(int times, int delayTime) {
  for (int t = 0; t < times; t++) {
    for (byte i = 0; i < 4; i++) digitalWrite(ledPins[i], HIGH);
    delay(delayTime);
    for (byte i = 0; i < 4; i++) digitalWrite(ledPins[i], LOW);
    delay(delayTime);
  }
}

void celebrateCorrectSequence() {
  int notes[] = {NOTE_E4, NOTE_G4, NOTE_E5, NOTE_C5, NOTE_D5, NOTE_G5};
  for (int i = 0; i < 6; i++) {
    digitalWrite(ledPins[i % 4], HIGH);
    if (!isMuted) tone(speaker, notes[i]);
    delay(150);
    digitalWrite(ledPins[i % 4], LOW);
    if (!isMuted) noTone(speaker);
  }
}

void playIntroMusic() {
  int melody[] = {NOTE_C4, NOTE_E4, NOTE_G4, NOTE_C5, NOTE_G4, NOTE_E4, NOTE_C4};
  int durations[] = {300, 300, 300, 500, 300, 300, 400};
  for (int i = 0; i < 7; i++) {
    if (!isMuted) tone(speaker, melody[i]);
    delay(durations[i]);
    if (!isMuted) noTone(speaker);
    delay(50);
  }
}

void playMistakeSound() {
  if (!isMuted) tone(speaker, NOTE_A2); delay(150);
  if (!isMuted) tone(speaker, NOTE_D2); delay(150);
  if (!isMuted) noTone(speaker);
}

void playModeSelectSound(GameMode mode) {
  if (isMuted) return;
  switch (mode) {
    case CLASSIC:  tone(speaker, NOTE_C4); delay(150); break;
    case SPEED:    tone(speaker, NOTE_G4); delay(80); tone(speaker, NOTE_G5); delay(80); break;
    case REVERSE:  tone(speaker, NOTE_C5); delay(100); tone(speaker, NOTE_A4); delay(100); break;
    case STEALTH:  tone(speaker, NOTE_E4); delay(70); noTone(speaker); delay(50); tone(speaker, NOTE_E4); delay(70); break;
  }
  noTone(speaker);
}

// Light up LED and play sound for the color at given index
void lightLedAndPlayTone(byte i) {
  digitalWrite(ledPins[i], HIGH);
  if (!isMuted) tone(speaker, gameTones[i]);
  delay(300);
  digitalWrite(ledPins[i], LOW);
  if (!isMuted) noTone(speaker);
}

void playToneOnly(byte i) {
  if (!isMuted) tone(speaker, gameTones[i]);
  delay(300);
  if (!isMuted) noTone(speaker);
}

void playStealthIntro() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Memorize Sounds");
  lcd.setCursor(0, 1);
  lcd.print("1 per color...");
  delay(1500);
  for (byte i = 0; i < 4; i++) {
    digitalWrite(ledPins[i], HIGH);
    if (!isMuted) tone(speaker, gameTones[i]);
    delay(600);
    digitalWrite(ledPins[i], LOW);
    if (!isMuted) noTone(speaker);
    delay(200);
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Replicate the");
  lcd.setCursor(0, 1);
  lcd.print("Sequence!");
  delay(2000);
}

void playSequence() {
  int speed = max(100, 400 - Level * 10);
  for (int i = 0; i < Level; i++) {
    byte current = gameSequence[i];

    if (currentMode == STEALTH) {
      if (!isMuted) {
        tone(speaker, gameTones[current]);
        delay(300);
        noTone(speaker);
      } else {
        delay(300);
      }
    } else {
// Light up LED and play sound for the color at given index
      lightLedAndPlayTone(current);
    }

    delay(speed);
  }
}

// Read button press with debouncing to avoid multiple triggers
byte readButtonsDebounced() {
  while (true) {
    for (byte i = 0; i < 4; i++) {
      if (digitalRead(buttonPins[i]) == LOW) {
        delay(20);
        if (digitalRead(buttonPins[i]) == LOW) {
          while (digitalRead(buttonPins[i]) == LOW);
          return i;
        }
      }
    }
    delay(1);
  }
}

byte readButtonsWithCountdown(unsigned long countdownTime) {
  unsigned long start = millis();
  int lastSecond = -1;
  while (millis() - start < countdownTime) {
    unsigned long elapsed = millis() - start;
    int remaining = (countdownTime - elapsed) / 1000;
    if (remaining != lastSecond) {
      lcd.clear();
      lcd.setCursor(3, 0);
      lcd.print("Your turn!");
      lcd.setCursor(3, 1);
      lcd.print("Time left: ");
      lcd.print(remaining + 1);
      lastSecond = remaining;
    }
    for (byte i = 0; i < 4; i++) {
      if (digitalRead(buttonPins[i]) == LOW) {
        delay(20);
        if (digitalRead(buttonPins[i]) == LOW) {
          while (digitalRead(buttonPins[i]) == LOW);
          return i;
        }
      }
    }
    delay(10);
  }
  return 255;
}

// Compare user's button presses to the correct sequence
bool checkUserSequence() {
  for (int i = 0; i < Level; i++) {
    byte expected = (currentMode == REVERSE) ? gameSequence[Level - 1 - i] : gameSequence[i];
// Read button press with debouncing to avoid multiple triggers
    byte actual = (currentMode == SPEED) ? readButtonsWithCountdown(3000) : readButtonsDebounced();
    if (actual == 255 || expected != actual) {
      lives--;
      return false;
    }
    if (currentMode == STEALTH)
      playToneOnly(actual);
    else
// Light up LED and play sound for the color at given index
      lightLedAndPlayTone(actual);
  }
  return true;
}

void updateHighScore(byte score) {
// Read saved high score from EEPROM (memory that keeps value even after turning off)
  byte currentHigh = EEPROM.read(highScoreAddress);
  if (currentHigh == 255 || score > currentHigh) {
// Save new high score to EEPROM
    EEPROM.write(highScoreAddress, score);
  }
}


// Show the Game Over screen and offer a replay
void showGameOver() {
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Game Over!");
  lcd.setCursor(2, 1);
  lcd.print("Score: ");
  lcd.print(Level - 1);
  delay(2000);

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 4; j++) digitalWrite(ledPins[j], HIGH);
    if (!isMuted) tone(speaker, NOTE_DS5); delay(150);
    if (!isMuted) tone(speaker, NOTE_D5); delay(150);
    if (!isMuted) tone(speaker, NOTE_CS5); delay(150);
    for (int j = 0; j < 4; j++) digitalWrite(ledPins[j], LOW);
    if (!isMuted) noTone(speaker);
    delay(100);
  }

  if (!isMuted) {
    for (int i = 0; i < 10; i++) {
      for (int pitch = -10; pitch <= 10; pitch++) {
        tone(speaker, NOTE_C5 + pitch);
        delay(5);
      }
    }
    noTone(speaker);
  }

  updateHighScore(Level - 1);
// Read saved high score from EEPROM (memory that keeps value even after turning off)
  byte high = EEPROM.read(highScoreAddress);

  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("High Score: ");
  lcd.print(high);
  delay(4000);
  lcd.clear();

  while (true);
}

void checkForMuteToggle() {
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("Hold Green");
  lcd.setCursor(3, 1);
  lcd.print("to Mute...");
  delay(1500);
  if (digitalRead(buttonPins[1]) == LOW) {
    isMuted = true;
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("Muted!");
    delay(1000);
  } else {
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print("Sound ON");
    delay(1000);
  }
}


// Show mode selection screen and wait for user to choose
void showModeSelection() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Select Game Mode");
  lcd.setCursor(0, 1);
  lcd.print("Y:C G:S R:R B:St");
  while (true) {
    for (int i = 0; i < 4; i++) {
      if (digitalRead(buttonPins[i]) == LOW) {
        delay(20);
        if (digitalRead(buttonPins[i]) == LOW) {
          while (digitalRead(buttonPins[i]) == LOW);
          currentMode = static_cast<GameMode>(i);
          playModeSelectSound(currentMode);
          lcd.clear();
          lcd.print((i == 0) ? "Classic" :
                    (i == 1) ? "Speed" :
                    (i == 2) ? "Reverse" : "Stealth");
          lcd.print(" Mode");
          delay(2000);
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Replicate the");
          lcd.setCursor(0, 1);
          lcd.print("Sequence!");
          delay(2000);
          if (currentMode == STEALTH) playStealthIntro();
          return;
        }
      }
    }
  }
}


// -------------------- SETUP FUNCTION --------------------
// This function runs once at the start to initialize everything
void setup() {
  Serial.begin(9600);
  for (byte i = 0; i < 4; i++) {
    pinMode(ledPins[i], OUTPUT);
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
  pinMode(speaker, OUTPUT);
  pinMode(resetPin, INPUT_PULLUP);
  randomSeed(analogRead(A0));

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Welcome to");
  lcd.setCursor(5, 1);
  lcd.print("Simon!");
  playIntroMusic();
  noTone(speaker);
  delay(1000);

  checkForMuteToggle();

  if (digitalRead(resetPin) == LOW) {
// Save new high score to EEPROM
    EEPROM.write(highScoreAddress, 0);
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("High Score Reset");
    delay(2000);
  }

  showModeSelection();
}


// -------------------- MAIN GAME LOOP --------------------
// This function keeps running forever and manages the gameplay
void loop() {
  if (lives == 0) showGameOver();

  gameSequence[Level] = random(0, 4);
  Level++;
  if (Level >= MAX_GAME_LENGTH) Level = MAX_GAME_LENGTH - 1;

  playSequence();

// Compare user's button presses to the correct sequence
  if (!checkUserSequence()) {
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print("Wrong!");
    if (lives > 0) {
      playMistakeSound();
      flashAllLEDsSync(1, 200);
    }
    lcd.setCursor(1, 1);
    lcd.print("Lives left: ");
    lcd.print(lives);
    delay(2000);
  } else {
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("Correct!");
    celebrateCorrectSequence();
    delay(1000);
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sequence ");
  lcd.print(Level);
  lcd.setCursor(0, 1);
  lcd.print("Lives: ");
  lcd.print(lives);
  delay(1000);
}