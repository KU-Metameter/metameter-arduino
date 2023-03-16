#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>
#include <SPI.h>
#include <Wire.h>


#define BUTTON 4 //on board button, use as necessary
#define V_ADC A0 //voltage and ohm adc positive input
#define COM_ADC A1 //common (negative) adc input
#define OHM_OUT 16 //ohmeter voltage divider supply output 
#define mA_ADC A3 //mA adc positive input
#define A_ADC A4 //A adc positive input
#define TFT_CS D2 //display chip select pin, used in setup call
#define TFT_DC  21 //display disconnect pin, used in setup call
#define TS_CS  22 //touch screen chip select pin, used in setup call
#define TS_IRQ D7 //touch screen interrupt pin, used in setup or standalone (default high)
#define V_OHM_CTL D9 //V/Ohm selector, high for voltmeter
#define UH_CTL D10 //control pin for UH range (1k)
#define H_CTL D11 //control pin for High range (9k)
#define M_CTL D12 //control pin for Medium range (90k)
#define L_CTL D13 //control pin for Low range (900k) (!!Exposes ADC to raw V/ohm In!!) 

//XPT2046_Touchscreen ts(TS_CS);
XPT2046_Touchscreen ts(TS_CS, TS_IRQ);
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

const int v_off = 240; //used for menu spacing. no touchy
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

void simpPrint3(){

}

//manually calibrated touch screen edges, not 100% accurate
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

bool touchInBox(int x, int y, int w, int h){
  TS_Point point = ts.getPoint();  
  if(touchXtoTFTx(point.x) > x && touchXtoTFTx(point.x) < x + w && touchYtoTFTy(point.y) > y && touchYtoTFTy(point.y) < y + h){
    return(true);
  }
  return(false);
}

bool touchInBoxPoint(int x, int y, int w, int h, TS_Point point){
  if(touchXtoTFTx(point.x) > x && touchXtoTFTx(point.x) < x + w && touchYtoTFTy(point.y) > y && touchYtoTFTy(point.y) < y + h){
    return(true);
  }
  return(false);
}



const float adc2v = 3.3/3745; // manually calibrated, only works for 12 bit
const int lowRangeRes = 900000;
const int medRangeRes = 90000;
const int highRangeRes = 9000;
const int uhRangeRes = 1000;
const int totRes = lowRangeRes + medRangeRes + highRangeRes + uhRangeRes;
//copied from voltmeter sketch, needs to be updated with new variable names, probably still gets fucky with negative voltages
//could be more accurate if parallel resistance of adc was accounted for
//could be more accurate if multiple samples were taken and averaged
void voltMeter(){
  int read = 0;
  digitalWrite(H_CTL, HIGH);
  digitalWrite(V_OHM_CTL, HIGH);
  delay(2); //required for relay switching 
  read = analogRead(V_ADC) - (analogRead(COM_ADC));
  if(abs(read) < 400){
    digitalWrite(H_CTL, LOW);
    digitalWrite(M_CTL, HIGH);
    delay(2); //relay switch
    read = analogRead(V_ADC) - (analogRead(COM_ADC));
    if(abs(read) < 400){
      digitalWrite(M_CTL, LOW);
      digitalWrite(L_CTL, HIGH);
      delay(2); //relay switch
      read = analogRead(V_ADC) - (analogRead(COM_ADC));
      simpPrint3("Low Range",read*adc2v*totRes/(lowRangeRes + medRangeRes + highRangeRes + uhRangeRes), read);
      digitalWrite(L_CTL, LOW);
      delay(2); //relay switch
    }
    else{
      simpPrint3("Med Range",read*adc2v*totRes/(medRangeRes + highRangeRes + uhRangeRes), read);
      digitalWrite(M_CTL, LOW);
      delay(2); //relay switch
    }
  }
  else{
    simpPrint3("High Range",read*adc2v*totRes/(highRangeRes + uhRangeRes), read);
    digitalWrite(H_CTL, LOW);
    digitalWrite(V_OHM_CTL, LOW);
    delay(2); //relay switch
  }    
  
}

void ohmMeter(){
  int read = 0;
  digitalWrite(H_CTL, HIGH);
  digitalWrite(V_OHM_CTL, LOW);
  digitalWrite(OHM_OUT, HIGH);
  delay(2); //required for relay switching 
  read = analogRead(V_ADC) - (analogRead(COM_ADC));
  if(abs(read) < 400){
    digitalWrite(H_CTL, LOW);
    digitalWrite(M_CTL, HIGH);
    delay(2); //relay switch
    read = analogRead(V_ADC) - (analogRead(COM_ADC));
    if(abs(read) < 400){
      digitalWrite(M_CTL, LOW);
      digitalWrite(L_CTL, HIGH);
      delay(2); //relay switch
      read = analogRead(V_ADC) - (analogRead(COM_ADC));
      simpPrint3("Low Range",read*adc2v*totRes/(lowRangeRes + medRangeRes + highRangeRes + uhRangeRes), read);
      digitalWrite(L_CTL, LOW);
      delay(2); //relay switch
    }
    else{
      simpPrint3("Med Range",read*adc2v*totRes/(medRangeRes + highRangeRes + uhRangeRes), read);
      digitalWrite(M_CTL, LOW);
      delay(2); //relay switch
    }
  }
  else{
    simpPrint3("High Range",read*adc2v*totRes/(highRangeRes + uhRangeRes), read);
    digitalWrite(H_CTL, LOW);
    digitalWrite(V_OHM_CTL, HIGH);  //may need to change, refer to call in setup
    digitalWrite(OHM_OUT, LOW);
    delay(2); //relay switch
  }    
}

void ampMeter(){

}

void miliampMeter(){
  
}

void setup() {
  analogReadResolution(12);
  pinMode(V_ADC, INPUT);
  pinMode(COM_ADC, INPUT);
  pinMode(mA_ADC, INPUT);
  pinMode(A_ADC, INPUT);
  pinMode(BUTTON, INPUT);

  digitalWrite(OHM_OUT, LOW);
  digitalWrite(V_OHM_CTL, HIGH);  //best start condition to prevent damage?
  digitalWrite(UH_CTL, LOW);
  digitalWrite(H_CTL, LOW);
  digitalWrite(M_CTL, LOW);
  digitalWrite(L_CTL, LOW);

  pinMode(OHM_OUT, OUTPUT);
  pinMode(V_OHM_CTL, OUTPUT);
  pinMode(UH_CTL, OUTPUT);
  pinMode(H_CTL, OUTPUT);
  pinMode(M_CTL, OUTPUT);
  pinMode(L_CTL, OUTPUT);

  tft.begin();
  tft.setRotation(2);
  tft.fillScreen(ILI9341_BLACK);
  ts.begin();
  ts.setRotation(0);
  printMenu();
  tft.setTextColor(ILI9341_WHITE);
}


int curTime = 0;
int lastTime = 0;
int mode = 0;  //
void loop() {  //utilizing zoomy loops to avoid using interrupts (double triggering, never felt responsive enough) keep everything in the main loop fast (or you die)
  curTime = milis();
  if(curTime >= lastTime + 500){  //run this section of the loop every half second 
    if(mode == 1){
      //BTmenu();
    }
    else if(mode == 2){
      //diodeMeter();
    }
    else if(mode == 3){
      miliampMeter();
    }
    else if(mode == 4){
      voltMeter();  //take approx 4 to 8 ms, could possibly be improved with better relay characterization
    }
    else if(mode == 5){
      ohmMeter();
    }
    else if(mode == 6){
      ampMeter();
    }


    
    lastTime = milis(); //run at end of loop for funsies
  }

  if (ts.tirqTouched()) { //check for touch, probably needs to be run rapidly to feel responsive  //TODO: Check how long it takes to run
    if (ts.touched()) {  //not sure if this nest is necessary 
      TS_Point p = ts.getPoint();
      
      if(touchInBox(h_off*0, v_off, h_off, h_off, point)){
        mode = 1;
      }
      else if(touchInBox(h_off*1, v_off, h_off, h_off, point)){
        mode = 2;
      }
      else if(touchInBox(h_off*2, v_off, h_off, h_off, point)){
        mode = 3;
      }
      else if(touchInBox(h_off*0, v_off-h_off, h_off, h_off, point)){
        mode = 4;
      }
      else if(touchInBox(h_off*1, v_off-h_off, h_off, h_off, point)){
        mode = 5;
      }
      else if(touchInBox(h_off*2, v_off-h_off, h_off, h_off, point)){
        mode = 6;
      }
    }
  }
  
}
