#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <ESP32Servo.h>
#include <SPI.h>
#include <MFRC522.h>
Adafruit_SSD1306 lcd(128, 64); // create display object
#define BUTTON0 34
#define BUTTON1 0
#define BUTTON2 35

#define SERVO1 27
#define SERVO_L 33
#define SERVO_R 32
#define BUZZER 17

#define BAUD_RATE 230400
#define SS_PIN 2
#define RST_PIN 4
MFRC522 mfrc522(SS_PIN, RST_PIN);

Servo catapult;
Servo leftServo;
Servo rightServo;
uint8_t *buffer;
int Time[3];
bool stateChange = false;
String state = "clock";
int alarmTimes[] = {12,0,0};
int settingIndex = 0;
double volume = .5;
int volumeTimer = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(BUTTON0, INPUT_PULLUP);
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);
  catapult.attach(SERVO1);
  leftServo.attach(SERVO_L);
  rightServo.attach(SERVO_R);
  catapult.write(55);
  leftServo.write(0);
  rightServo.write(180);

  SPI.begin();
  mfrc522.PCD_Init();
  lcd.begin(SSD1306_SWITCHCAPVCC, 0x3C); // init
  lcd.clearDisplay();
  lcd.setTextColor(WHITE);
  lcd.setCursor(0, 28);
  lcd.setTextSize(1);
  lcd.print("Waiting for data...");
  lcd.display();
  Serial.begin(BAUD_RATE);
  buffer = lcd.getBuffer();
}

#define WIDTH 128
byte data[WIDTH];
byte frame_header[4];
int clockTimeout = 500;
int blinkTimeout = 500;
int buttonTimeout = 100;
int rfidTimeout = 500;
int timeOfAlarm = 0;
int triggerTimeout = 500;
int buzzerTimeout = 0;
bool firstAlarmInter = true;
bool blink = false;
byte prev0 = 1;
byte prev1 = 1;
byte prev2 = 1;

void loop() {
  // put your main code here, to run repeatedly:
  //if(state.equals("clock")){
  int mils = millis();
    if(millis() > clockTimeout){//this has to run every iteration or it loses time in other states
      if(Serial.available()) {
        Serial.readBytes(frame_header, 4);
        byte i;
        for(i=0;i<4;i++) {
          if(frame_header[i]!=i+61) break;  // verify frame header, should be 61,62,63,64
        }
        if(i==4) {  // valid frame header received
          for(int i=0; i<=2; i++){
            Time[i] = Serial.read();
          }

          if(state.equals("clock")){
            printTime(Time, -1);
            lcd.display();
            lcd.clearDisplay();
          }

        }
        else{
          lcd.clearDisplay();
          lcd.setTextColor(WHITE);
          lcd.setCursor(0, 28);
          lcd.setTextSize(1);
          lcd.print("Error receiving data");
        }

      }
      clockTimeout = millis() + 500;
    }

  if(state.equals("volume")){
    if(volumeTimer > mils){
      lcd.setCursor(50, 10);
      lcd.setTextSize(1);
      lcd.print("Volume:");
      lcd.drawTriangle(0, 64, 128, 64, 128, 32, WHITE);
      int xVal = 128 * volume;
      int yVal = 64 - 32*volume;
      lcd.fillTriangle(0, 64, xVal, 64, xVal, yVal, WHITE);
      lcd.display();
      lcd.clearDisplay();
    }
    if(volumeTimer <= mils){
      state = "clock";
    }
  }
  //}

  if(state.equals("setalarm")){
    lcd.setCursor(30, 10);
    lcd.setTextSize(1);
    lcd.print("Set Alarm:");
    if(millis() > blinkTimeout){ 
      //allows the time being set to blink, indicating it to user
      if(blink){
        printTime(alarmTimes, settingIndex);
        lcd.display();
        lcd.clearDisplay();
      }
      else{
        printTime(alarmTimes, -1);
        lcd.display();
        lcd.clearDisplay();
      }
      blinkTimeout = millis() + 500;
      blink = !blink;
    }

  }

  //CHECK IF ALARM MATCH
  if(!state.equals("alarm") && !state.equals("setalarm")){
    bool timesMatch = true;
    for(int i=0; i<=2; i++){
      if(Time[i] != alarmTimes[i]){
        timesMatch = false;
      }
    }
    if(timesMatch){
      state = "alarm";
      timeOfAlarm = mils;
      rfidTimeout = 500;
    }
  }

  //ALARM STATE
  if(state.equals("alarm")){
    //chaos ensues
    lcd.setCursor(0, 28);
    lcd.setTextSize(2);
    lcd.print("WAKE UP!!!");
    lcd.display();
    lcd.clearDisplay();
    lcd.setTextSize(1);
    
    //volume is between 0 and 1. If 0, off, otherwise scale
    //the timeout is half the period. Timeout of 1ms = T = 2
    //T=2ms means 2 ms cycle, freq = 1/T = 500 ms
    //want T to go down for freq to go up
    //use us
    //10kHz = 100 us = 50 us timeout
    //100 Hz = 10000 us = 5000 us timeout
    //vol = 1 -> 50 us
    //vol = .1 -> 5000 us0
    int freq = (-5000)*volume + 5550;
    if(volume > 0){
      if(micros() > buzzerTimeout){
        digitalWrite(BUZZER, 1-digitalRead(BUZZER));
        buzzerTimeout = micros() + freq;
      }

    }

    int catapultTimeout = 50;
    int catapultAngle = 180;
    if(mils < timeOfAlarm + triggerTimeout){
      catapult.write(catapultAngle);
      leftServo.write(90);
      rightServo.write(90);
    }
    else{
      if(mils > catapultTimeout){
        catapultAngle -= 5;
        catapultTimeout = mils + 50;
      }
      catapult.write(55);
      leftServo.write(0);
      rightServo.write(180);
    }

    //wait rfidTimeout milliseconds after alarm goes off
    if(mils > timeOfAlarm + rfidTimeout){
      //Serial.println("aaa");
      //if rfid present, turn alarm off
      if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
        // If a card is present, read its UID (unique identifier)
        String cardID = "";
        for (byte i = 0; i < mfrc522.uid.size; i++) {
          cardID += String(mfrc522.uid.uidByte[i], HEX);
        }

        // Print the card ID to the serial monitor
        Serial.println("Card detected: " + cardID);
        catapult.write(55);
        leftServo.write(0);
        rightServo.write(180);
        digitalWrite(BUZZER, 0);
        state = "clock";
      }
      rfidTimeout += 100;
    }
  }



  //BUTTON CONTROLLER
  byte curr0 = digitalRead(BUTTON0);
  byte curr1 = digitalRead(BUTTON1);
  byte curr2 = digitalRead(BUTTON2);

  if(millis() > buttonTimeout){
    //BUTTON 1
    if(curr0 == 0 && prev0 == 1){
      Serial.print("button0 pushed");
      if(state.equals("clock") || state.equals("volume")){//volume up
        if(volume < 1){
          volume += .1;
        }
        volumeTimer = mils + 1000;
        state = "volume";
      }
      else if(state.equals("setalarm")){//increase alarm h/m/s by 1
        alarmTimes[settingIndex]++;
        if(settingIndex == 0)
          alarmTimes[settingIndex] %= 24;
        else
          alarmTimes[settingIndex] %= 60;
              
        printTime(alarmTimes, -1);
        blinkTimeout = millis() + 500;
        lcd.display();
        lcd.clearDisplay();
      }

      buttonTimeout = millis() + 150;
    }
    //BUTTON 2
    else if(curr1 == 0 && prev1 == 1){
      Serial.print("button1 pushed");
      if(state.equals("clock") || state.equals("volume")){//volume down
        if(volume > 0){
          volume -= .1;
        }
        volumeTimer = mils + 1000;
        state = "volume";
      }
      else if(state.equals("setalarm")){//swap between setting h/m/s
        settingIndex++;
        settingIndex %= 3;
      }

      buttonTimeout = millis() + 150;
    }
    //BUTTON 3
    else if(curr2 == 0 && prev2 == 1){
      Serial.print("button2 pushed");
      if(state.equals("clock")){//set alarm
        state = "setalarm";
        settingIndex = 0;
      }
      else if(state.equals("setalarm")){
        state = "clock";
      }

      buttonTimeout = millis() + 150;
    }

  
  }
  prev0 = curr0;
  prev1 = curr1;
  prev2 = curr2;
}

void printTime(int timeArray[], int blinkIndex){

    lcd.setCursor(0, 28);
    lcd.setTextSize(2);
    String suffix = "AM";
    for (int n = 0; n <= 2; n++) {
      int displayTime = timeArray[n];
      if(n==0 && displayTime > 12){
        suffix = "PM";
        displayTime -= 12;
      }
      if(blinkIndex != n){
        if (displayTime < 10) {
          lcd.print("0");
        }
        lcd.print(displayTime);
      }
      else{
        lcd.print("  ");
      }
      if (n < 2) {
        lcd.print(":");
      }
    }
    lcd.setTextSize(1);
    lcd.print(" " + suffix);

}