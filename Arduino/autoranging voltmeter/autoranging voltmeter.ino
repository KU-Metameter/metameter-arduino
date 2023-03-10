#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeMono9pt7b.h>


// DOES NOT WORK WITH NEGATIVE INPUT


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define A_POS A0
#define A_NEG A1
#define HI_EN 11
#define MED_EN 10
#define LOW_EN 9


void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();   
  display.setTextColor(SSD1306_WHITE);
  
  analogReadResolution(12);
  pinMode(A_POS, INPUT);
  pinMode(A_NEG, INPUT);

  digitalWrite(HI_EN, LOW);
  digitalWrite(MED_EN, LOW);
  digitalWrite(LOW_EN, LOW);

  pinMode(HI_EN, OUTPUT);
  pinMode(MED_EN, OUTPUT);
  pinMode(LOW_EN, OUTPUT);
}



void simpPrint(float toPrint){
  display.clearDisplay();
  display.setCursor(0,17);
  display.setTextSize(2);
  display.println(toPrint); 
  display.display(); 
}
void simpPrint2(float toPrint, float toPrint2){
  display.clearDisplay();
  display.setCursor(0,17);
  display.setTextSize(2);
  display.println(toPrint); 
  display.setCursor(0,33);
  display.println(toPrint2); 
  display.display(); 
}

void simpPrint3(char* toPrint, float toPrint2, float toPrint3){
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0,0);
  display.println(F(toPrint));
  display.setCursor(0,17);
  display.println(toPrint2); 
  display.setCursor(0,33);
  display.println(toPrint3); 
  display.display(); 
}

void voltMeter(){
  int read = 0;
  const float adc2v = 3.3/3745;
  const int highRes = 1001000;
  const int medRes = 99200;
  const int lowRes = 9920;
  const int totRes = highRes + medRes + lowRes;
  while (1){
    digitalWrite(HI_EN, HIGH);
    delay(2);
    read = analogRead(A0) - (analogRead(A1));
    if(abs(read) < 400){
      digitalWrite(HI_EN, LOW);
      digitalWrite(MED_EN, HIGH);
      delay(2);
      read = analogRead(A0) - (analogRead(A1));
      if(abs(read) < 400){
        digitalWrite(MED_EN, LOW);
        digitalWrite(LOW_EN, HIGH);
        delay(2);
        read = analogRead(A0) - (analogRead(A1));
        simpPrint3("Low Range",read*adc2v*totRes/(highRes+medRes+lowRes), read);
        digitalWrite(LOW_EN, LOW);
        delay(2);
      }
      else{
        simpPrint3("Med Range",read*adc2v*totRes/(medRes+lowRes), read);
        digitalWrite(MED_EN, LOW);
        delay(2);
      }
    }
    else{
      simpPrint3("High Range",read*adc2v*totRes/lowRes, read);
      digitalWrite(HI_EN, LOW);
      delay(2);
    }    
    delay(500);
  }
  
}


void loop() {
  voltMeter();
}

