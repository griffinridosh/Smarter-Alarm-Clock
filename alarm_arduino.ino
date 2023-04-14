#include <Adafruit_SSD1306.h>
#include <Wire.h>
Adafruit_SSD1306 lcd(128, 64); // create display object
#define BUTTON0 34
#define BUTTON1 0
#define BUTTON2 35
#define BAUD_RATE 230400
uint8_t *buffer;
int Time[3];
bool stateChange = false;
String state = "clock";
int alarmTimes[] = {12,0,0};
int settingIndex = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(BUTTON0, INPUT_PULLUP);
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);    
  lcd.begin(SSD1306_SWITCHCAPVCC, 0x3C); // init
  lcd.clearDisplay();
  lcd.setTextColor(WHITE);
  lcd.setCursor(0, 28);
  lcd.print("Waiting for data...");
  lcd.display();
  Serial.begin(BAUD_RATE);
  buffer = lcd.getBuffer();
}

#define WIDTH 128
byte data[WIDTH];
byte frame_header[4];
int buttonTimeout = 100;
byte prev0 = 0;
byte prev1 = 0;
byte prev2 = 0;

void loop() {
  // put your main code here, to run repeatedly:
  if(state.equals("clock")){
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
        printTime(Time);
        /*lcd.setCursor(10, 28);
        String suffix = "AM";
        for (int n = 0; n <= 2; n++) {

          //check if AM or PM
          if(n==0 && Time[n] > 12){
            suffix = "PM";
            Time[n] -= 12;
          }
          if (Time[n] < 10) {
          lcd.print("0");
          }
          lcd.print(Time[n]);
          if (n < 2) {
            lcd.print(":");
          }
        }
        lcd.print(" " + suffix);*/
        delay(500);//FIX THIS
      }
      else{
        lcd.clearDisplay();
        lcd.setTextColor(WHITE);
        lcd.setCursor(0, 28);
        lcd.print("Didn't work");
      }
      lcd.display();
      lcd.clearDisplay();
    }


  }

  else if(state.equals("setalarm")){
    

    printTime(alarmTimes);




  }

  //BUTTON CONTROLLER
  byte curr0 = digitalRead(BUTTON0);
  byte curr1 = digitalRead(BUTTON1);
  byte curr2 = digitalRead(BUTTON2);

  if(millis() > buttonTimeout){

    if(curr0 == 0 && prev0 == 1){

      if(state.equals("clock")){//volume up


      }
      else if(state.equals("setalarm")){//increase alarm h/m/s by 1

      }

    }

    else if(curr1 == 0 && prev1 == 1){
      
      if(state.equals("clock")){//volume down

          
      }
      else if(state.equals("setalarm")){//swap between setting h/m/s

      }

    }

    else if(curr2 == 0 && prev2 == 1){

      if(state.equals("clock")){//set alarm
        state = "setalarm";
          
      }
      else if(state.equals("setalarm")){
        state = "clock";
      }

    }

    buttonTimeout = millis() + 100;
  }
}

void printTime(int timeArray[]){

    lcd.setCursor(10, 28);
    String suffix = "AM";
    for (int n = 0; n <= 2; n++) {
      int displayTime = timeArray[n];
      if(n==0 && displayTime > 12){
        suffix = "PM";
        displayTime -= 12;
      }
      if (displayTime < 10) {
        lcd.print("0");
      }
      lcd.print(displayTime);
      if (n < 2) {
        lcd.print(":");
      }
    }
    lcd.print(" " + suffix);

}
