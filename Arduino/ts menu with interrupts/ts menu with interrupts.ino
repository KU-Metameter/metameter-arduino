#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>
#include <SPI.h>
#include <Wire.h>

// not working, hanging up after writing a few vals


#define TS_CS  11
#define TFT_DC  9
#define TFT_CS 10
#define TS_IRQ 12
#define BUTTON 4
#define LED_EXT 13

XPT2046_Touchscreen ts(TS_CS);
//XPT2046_Touchscreen ts(TS_CS, TS_IRQ);
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

const int v_off = 240;
const int h_off = 80;

void printMenu(){
  tft.fillRect(h_off*0, v_off, h_off, h_off, ILI9341_BLUE);
  tft.fillRect(h_off*1, v_off, h_off, h_off, ILI9341_RED);  
  tft.fillRect(h_off*2, v_off, h_off, h_off, ILI9341_YELLOW);  
  tft.fillRect(h_off*0, v_off-h_off, h_off, h_off, ILI9341_GREEN);
  tft.fillRect(h_off*1, v_off-h_off, h_off, h_off, ILI9341_ORANGE);
  tft.fillRect(h_off*2, v_off-h_off, h_off, h_off, ILI9341_PURPLE);
  tft.setTextColor(ILI9341_BLACK);
  tft.setCursor(h_off*0+20, v_off-h_off+20);
  tft.setTextSize(6);
  tft.print("V");
  tft.setCursor(h_off*1+20, v_off-h_off+20);
  tft.setTextSize(6);
  tft.print("O"); 
  tft.setCursor(h_off*2+20, v_off-h_off+20);
  tft.setTextSize(6);
  tft.print("A"); 
  tft.setCursor(h_off*0+10, v_off+20);
  tft.setTextSize(5);
  tft.print("BT");       
  tft.setCursor(h_off*1+20, v_off+20);
  tft.setTextSize(6);
  tft.print("D");   
  tft.setCursor(h_off*2+10, v_off+20);
  tft.setTextSize(5);
  tft.print("mA");   
  
}


//manually calibrated, not 100% accurate
const int x_min = 280;
const int x_max = 3893;
const int y_min = 370;
const int y_max = 3916;

int touchXtoTFTx(int touch_x){
  return((touch_x-x_min)*240/(x_max-x_min));
}

int touchYtoTFTy(int touch_y){
  return((touch_y-y_min)*320/(y_max-y_min));
}

/*bool touchInBox(int x, int y, int w, int h){
  TS_Point point = ts.getPoint();  
  if(touchXtoTFTx(point.x) > x && touchXtoTFTx(point.x) < x + w && touchYtoTFTy(point.y) > y && touchYtoTFTy(point.y) < y + h){
    return(true);
  }
  return(false);
}*/

bool touchInBox(int x, int y, int w, int h, TS_Point point){
  if(touchXtoTFTx(point.x) > x && touchXtoTFTx(point.x) < x + w && touchYtoTFTy(point.y) > y && touchYtoTFTy(point.y) < y + h){
    return(true);
  }
  return(false);
}

int mode = 0;
bool tsWasTouched = false;
void interruptHandler() {
  if(ts.touched() && !tsWasTouched){
    digitalWrite(LED_EXT, HIGH);
    TS_Point point = ts.getPoint();
    tft.fillRect(0, 0, 240, v_off-h_off, ILI9341_BLACK);
    tft.setTextSize(2);
    tft.setCursor(0,0);
    tft.print("x = ");
    tft.print(point.x);
    tft.print("y = ");
    tft.print(point.y);
    
    if(touchInBox(h_off*0, v_off, h_off, h_off, point)){
      tft.setCursor(0,20);
      tft.print("BT Mode");
      mode = 1;
    }
    else if(touchInBox(h_off*1, v_off, h_off, h_off, point)){
      tft.setCursor(0,20);
      tft.print("Diode Mode");
      mode = 2;
    }
    else if(touchInBox(h_off*2, v_off, h_off, h_off, point)){
      tft.setCursor(0,20);
      tft.print("Miliamp Mode");
      mode = 3;
    }
    else if(touchInBox(h_off*0, v_off-h_off, h_off, h_off, point)){
      tft.setCursor(0,20);
      tft.print("Voltage Mode");
      mode = 4;
    }
    else if(touchInBox(h_off*1, v_off-h_off, h_off, h_off, point)){
      tft.setCursor(0,20);
      tft.print("Ohm Mode");
      mode = 5;
    }
    else if(touchInBox(h_off*2, v_off-h_off, h_off, h_off, point)){
      tft.setCursor(0,20);
      tft.print("Amp Mode");
      mode = 6;
    }
    tsWasTouched = true;
  }
  else{
    tsWasTouched = false;
  }
}


void setup() {
  pinMode(TS_IRQ, INPUT_PULLUP);
  pinMode(LED_EXT, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(TS_IRQ), interruptHandler, FALLING);  
  
  tft.begin();
  tft.setRotation(2);
  tft.fillScreen(ILI9341_BLACK);
  ts.begin();
  ts.setRotation(0);
  printMenu();
  tft.setTextColor(ILI9341_WHITE);
}


void loop() {
  delay(1000);
  digitalWrite(LED_EXT, LOW);
  tft.setCursor(0,40);
  tft.print("Mode: ");
  tft.print(mode);
}
