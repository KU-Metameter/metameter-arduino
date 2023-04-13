#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>
#include <SPI.h>
#include <Wire.h>
#include <bluefruit.h>
//#include <string>
#include "nRF52_PWM.h"

//using std::string;

//these are the pin assignments of the second revision of the multimeter schematic PRONE TO CHANGE (probably not tho)
#define BUTTON 4 //on board button, use as necessary
#define V_ADC A0 //voltage and ohm adc positive input
#define COM_ADC A1 //common (negative) adc input
#define OHM_OUT A2 //ohmeter voltage divider supply output 
#define mA_ADC A3 //mA adc positive input
#define A_ADC A4 //A adc positive input
#define TS_IRQ 2 //touch screen interrupt pin, used in setup or standalone (default high)
#define TS_CS  21 //touch screen chip select pin, used in setup call 
#define TFT_DC  22 //display disconnect pin, used in setup call
#define TFT_CS 7 //display chip select pin, used in setup call
#define L_CTL 9 //control pin for Low range (90k) (!!Exposes ADC to raw V/ohm In!!) 
#define M_CTL 10 //control pin for Medium range (9k)
#define H_CTL 11 //control pin for High range (1k)
#define OHM_GND_CTL 12  //high connects in_com to gnd, used for ohmeter
#define V_OHM_CTL 13 //V/Ohm selector, high for voltmeter
#define SPK A5 //speaker control pin (must be driven by ~4khz pwm signal)

//for speaker PWM output
#define _PWM_LOGLEVEL_       4
#define USING_TIMER       false
nRF52_PWM* PWM_Instance;
float frequencyOn = 4000.0f;
float frequencyOff = 0.0f;
float dutyCycle = 50.0f;  

//XPT2046_Touchscreen ts(TS_CS); //declaration for non interrupting touch screen object (I think its an object)
XPT2046_Touchscreen ts(TS_CS, TS_IRQ);
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

const int v_off = 240; //used for menu spacing. no touchy
const int h_off = 80;

// bluetooth shit idk ask andrew 

//andrew puts spaces at the start of his comments 
/*
⢎⣽⣭⠿⣶⣅⣶⠴⠤⠦⠤⡄⡤⠤⣤⡤⠤⠤⠤⡄⣠⠤⠠⠤⡀⠀⠀⠀⡤⠤⠤⠤⣤⠤⠤⠤⠤⡄⢀⡤⠤⠤⢄⠀⡤⠤⠤⡄⢠⠤⠤⢤⠀⠀⠀⢀⡤⠄⠤⢄⠠⡤⠤⠤⠤⢄⢠⠤⠤⡤⠤⢤⣠⠤⢤⢀⡤⠤⠤⢄⠀⡤⠤⠤⠤⡄⣀⡤⢤⢀⠀
⢭⣿⣍⡁⣯⣽⣯⠀⢰⢦⠀⢹⠅⠀⡽⡇⠀⡖⠐⠧⡃⠠⣶⣀⣹⠀⠀⠀⡇⠀⣆⠲⣹⠀⢰⢢⠀⢸⡎⠀⢴⠄⠈⣶⡃⠀⡀⢱⡜⠀⠀⢸⠀⠀⠀⡞⠀⣰⡄⠈⣹⠄⠀⣶⡀⠈⣾⠀⠀⡇⠀⠘⣭⠀⢸⡞⠀⡰⣄⠈⣏⡇⠀⡖⠒⠟⣯⠙⢘⡭⠗
⢎⣷⢶⣃⣷⠼⣧⠀⢸⢼⠀⢸⡃⠀⡽⡇⠀⠉⠙⡆⠣⡀⠈⠲⡄⠀⠀⠀⡇⠀⢉⣙⣻⠀⠈⡉⠠⣜⡇⠀⢸⠃⠀⣷⡁⠀⡇⠸⠇⢸⠀⢸⠀⠀⠀⡇⠀⢎⠑⠂⢹⠀⠀⡉⠠⢴⢓⠀⢀⡇⠀⣃⢫⠀⢸⠇⠀⣽⠛⠛⣷⡃⠀⠉⠙⡏⠧⣎⣱⠦⠏
⢎⡼⣭⢏⢧⡃⡷⠀⢸⢼⠀⢸⡅⠀⡽⡇⠀⡏⠉⢱⠒⠛⣧⠀⢸⡄⠀⠀⡇⠀⡏⢈⢸⠀⢸⣽⠀⢸⡇⠀⣼⡁⠀⣿⠆⠀⣷⠀⠀⣸⠁⢸⠀⠀⠀⡇⠀⢯⡋⠉⣿⠀⠀⣿⠀⠸⢨⠀⠀⡆⠀⢽⠈⠀⢸⡇⠀⣙⡇⠀⡷⡃⠀⡏⠉⠁⠀⠀⠀⠀⠀
⢮⠵⣎⡟⢮⣓⡿⠀⠈⠁⣀⠞⡇⠀⣳⡇⠀⠉⠉⢿⢇⡀⠉⢀⡜⠀⠀⠀⡇⠀⡗⢂⣻⠀⢸⣿⠀⢸⣷⡀⠈⠀⣰⢉⡇⠀⣟⡄⠀⣿⠀⢸⠀⠀⠀⠱⣀⠈⠁⣠⢺⡀⠀⣿⠀⢸⠰⠀⠰⡁⠀⢸⣇⠀⢸⢰⡀⠈⢁⡀⣟⡇⠀⠉⠉⡇⠀⠀⠀⠀⠀
⢧⢻⡼⣭⠳⣌⠯⣍⠩⣍⠱⣌⠩⠍⠁⠉⠉⠉⠉⠁⠀⠉⠉⠁⠀⠀⠀⠀⣩⣽⣿⡿⠟⠉⠉⠈⠉⠉⠈⠙⠛⢿⣿⣿⣭⡫⢟⣿⣿⣿⣿⣿⣧⠀⠀⠀⠈⠉⠉⠀⠀⠉⠉⠉⠉⠉⠈⠉⠉⠉⠉⠉⠈⠉⠉⠀⠈⠉⠁⠉⠁⠉⠉⠉⠉⠁⠀⠀⠀⠀⠀
⣭⢳⡟⢶⣛⣬⢳⣉⠳⢬⠓⣌⠣⡁⠀⢀⠢⡐⠤⠐⠀⠀⠀⠀⠀⣈⣶⣿⣿⠟⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠻⣿⣿⣭⢿⣿⣿⣿⣿⣿⣧⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⠀⠀⠀⠀⠀⠀⢀⠠⠀⠀⡀⠆⡄⠀⠀⢀⠂⡀⠀⠀⠀⠀
⣎⢷⣫⠷⣭⢖⣣⡜⡣⢏⣝⢢⡓⢤⢀⢂⡳⠌⡄⢣⢤⣾⠿⢷⣴⣿⣿⣿⢃⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠹⣿⣿⡽⣿⣿⣿⣿⣿⣧⢀⡀⠀⠀⠀⢀⠰⡈⢆⠄⠀⢀⠒⠤⡙⠈⠆⡍⢊⠅⠂⠄⠁⢂⠜⡌⡐⠠⠐⢂⠡⠐⠀⠀⠀⡀
⢾⣹⣎⠿⣜⡯⢶⣍⠳⣏⢎⢧⡙⣆⠣⢎⡜⣡⡘⢆⡛⣣⣴⣿⣿⣿⣿⣿⣿⣿⣿⣶⡄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⢻⣿⣯⢿⣿⣿⣿⣿⣿⣷⣄⠀⠀⢄⠃⡱⢊⠀⡐⢀⡉⢆⠑⡈⠠⠜⡀⠎⡡⢀⠡⢈⢢⢱⢠⠁⡌⢂⠄⡀⠀⠀⠠⠅
⣟⡶⣯⠟⠾⠽⠧⠞⠽⠚⠮⠷⣻⣼⣻⣶⣝⣦⣝⡲⣽⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⢾⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠹⣿⣷⣿⣿⣿⣿⣿⣿⣟⣶⡌⢤⠓⡔⢣⠎⠴⡡⠜⡌⢖⠡⡑⢭⡘⡱⢌⡣⡜⣂⠧⣋⠶⣙⢬⣋⠾⡡⠍⣌⠓⡬
⣯⣟⣧⣀⣀⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⡠⣤⣼⡷⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣻⡅⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢻⣿⣿⣿⣿⣿⣿⣿⣿⣾⣿⠢⡍⡜⣡⠚⢤⠣⡝⡜⡌⢆⠱⢢⢱⡘⢦⡱⣚⢔⣫⢒⡽⡘⢶⣩⠞⣥⠓⣌⡚⡔
⣿⣾⢯⣿⣽⡾⣟⣿⣛⢶⡱⢶⡶⣶⢿⣷⣿⣿⣿⣟⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡷⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡀⠤⡘⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⡼⣜⡥⣛⢦⠳⡜⡧⡝⢮⣙⢧⢧⣛⠶⣹⡜⢮⢖⡯⢶⣹⢳⣎⠿⣜⡛⢶⡹⢞
⣿⣿⣾⣽⣻⣷⠻⠉⠋⠈⠁⠈⠑⠋⠚⣿⣿⣿⣿⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⡃⠀⠀⠀⠀⠀⠀⠀⠀⡀⠄⡠⣀⢦⠱⣤⢓⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⣿⣵⣯⣜⢶⡱⣃⠷⣘⡻⡼⣌⠟⣴⢫⡕⡞⣼⣱⣎⠿⣬⢛⡴⣫⠗⣎⣳⢎
⣯⣽⣧⣯⣽⡏⠀⠀⠀⠀⠀⠀⠀⠀⠀⢻⣿⣿⣽⣻⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⢠⠄⡤⣀⠒⣄⢣⡙⢦⣹⠜⠀⢘⡮⢷⣚⣯⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣾⣿⣷⣛⡾⣥⢳⣑⠎⣉⢀⢋⡈⣑⣁⣋⣈⢓⣈⢋⣘⣁⢛⣈⡙⣊
⣿⣿⣿⣿⣿⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⣿⣿⣟⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣟⣮⣟⣶⣭⢿⣼⣳⣻⠟⣀⣠⡐⣧⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡻⣿⣿⣿⣿⣿⣿⣿⣿⣯⣿⣾⣥⣏⣮⢵⣫⣞⡵⣎⡷⢎⣏⢶⡹⣎⡳⣝⢮
⠿⡿⢿⢿⡿⡀⠀⠀⠀⠀⠀⠀⠀⢀⡀⣀⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⢫⣾⣵⣳⣿⣿⣱⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠟⡁⠹⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣯⣷⣿⣾⢿⣽⢾⣹⢎⡷⣽⢺⣵⣛⣮
⠀⠀⠀⠀⠀⠁⠈⠀⠀⠀⠀⠀⠀⠀⠀⠀⠘⣿⣿⢫⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⣋⢭⡻⢿⣿⣿⣿⣿⣿⣾⣿⣿⣿⣿⣿⣳⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣻⡝⡆⢼⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡎⠀⠉⠁⠈⠁⠈
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠐⣿⣿⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⣿⣾⣽⣿⣿⣿⣿⡿⣿⠿⣿⠿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣽⡳⢌⢿⣿⣿⣿⣿⣿⡟⠿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡟⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠠⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣭⣟⣿⣿⣿⣿⣿⣿⣷⣟⢧⣺⣿⣿⣟⣷⣿⡷⢶⠾⣽⣿⣿⣿⣿⣿⣿⡿⠋⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠉⣙⢻⢻⡿⣿⣯⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⢿⣻⢯⣟⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠃⠀⠀⠀⠀⠀⠀⠀⠈⠉⠉⠉⠁⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠐⡈⣬⢣⡟⣵⡿⣯⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⣿⣟⣯⢿⣽⣻⠿⡿⡿⣿⢿⡿⣿⠿⠿⠿⠿⠿⠿⠛⠛⠒⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⠡⢖⡴⢣⡛⣷⢻⡽⣿⣻⢿⡿⢿⠿⣿⠿⡿⢿⡟⣿⠻⡟⢯⠛⠧⠛⠜⠑⠃⠘⠈⠒⠈⠀⠉⠀⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠐⠀⠁⠀⠀⠁⠀⠀⠁⠀⠁⠀⠃⠈⠈⠘⠀⠁⠀⠁⠈⠀⠁⠀⠀⠈⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
*/
const uint8_t UUID128_SVC_MULTIMETER[] = {0x3b, 0xe7, 0xc3, 0x85, 0x95, 0xc0, 0xd4, 0x90, 0x61, 0x42, 0xaa, 0xa0, 0x6c, 0x92, 0x83, 0x3e};
const uint8_t UUID128_CHR_VOLTAGE[] = {0x81, 0xfe, 0xb9, 0x16, 0xbf, 0x03, 0x64, 0xbf, 0x51, 0x41, 0xd8, 0x9b, 0x5b, 0x5c, 0xf8, 0x86};

BLEService        multimeter_svc = BLEService(UUID128_SVC_MULTIMETER);
BLECharacteristic voltage_chr = BLECharacteristic(UUID128_CHR_VOLTAGE);

BLEDis bledis;    // DIS (Device Information Service) helper class instance
// BLEBas blebas;    // BAS (Battery Service) helper class instance

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

void simpPrint4(char* Mode, float Value, char* Units, float Read){
  tft.fillRect(0, 0, 240, v_off-h_off, ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setCursor(0,0);
  tft.print(Mode);
  tft.setCursor(0,30);
  tft.setTextSize(4);
  tft.print(Value);
  tft.setCursor(0,80);
  tft.setTextSize(4);
  tft.print(Units);
  tft.setTextSize(2);
  tft.setCursor(0,140);
  tft.print("Raw ADC:");
  tft.print(Read);

  // transmit the value over bluetooth
  voltage_chr.notify32(Value);
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



const float adc2v = 3.3/3745; // manually calibrated, only works for 12 bit //used for all data input functions //different for each itsybitsy?
const int lowRangeRes = 90000; // values also used for ohmmeter //holy shit $15 buys a good resistor network
const int medRangeRes = 9000;
const int highRangeRes = 1000;
const int totRes = lowRangeRes + medRangeRes + highRangeRes;
//copied from voltmeter sketch,  probably still gets fucky with negative voltages
//could be more accurate if parallel resistance of ADC was accounted for
//could be more accurate if multiple samples were taken and averaged
//modified for use with 100k input impedance, may need tweaking
void voltMeter(){
  int read = 0;
  digitalWrite(H_CTL, HIGH);
  digitalWrite(M_CTL, LOW); //redundancy, setting defaults
  digitalWrite(L_CTL, LOW);
  digitalWrite(V_OHM_CTL, HIGH);
  digitalWrite(OHM_GND_CTL, LOW);
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
      simpPrint4("Low Range",read*adc2v*totRes/(lowRangeRes + medRangeRes + highRangeRes),"Volts" , read);
      digitalWrite(L_CTL, LOW);
      //delay(2); //relay switch
    }
    else{
      simpPrint4("Med Range",read*adc2v*totRes/(medRangeRes + highRangeRes),"Volts" , read);
      digitalWrite(M_CTL, LOW);
      //delay(2); //relay switch
    }
  }
  else{
    simpPrint4("High Range",read*adc2v*totRes/(highRangeRes),"Volts" , read);
    digitalWrite(H_CTL, LOW);
    digitalWrite(V_OHM_CTL, LOW);
    //delay(2); //relay switch
  }    

  digitalWrite(L_CTL, LOW); //resetting to defaults
  digitalWrite(M_CTL, LOW); 
  digitalWrite(H_CTL, LOW);
  digitalWrite(OHM_GND_CTL, LOW);  
  digitalWrite(V_OHM_CTL, HIGH);  //may need to change, refer to call in setup
  digitalWrite(OHM_OUT, LOW);
  delay(2); //relay switch
  
}


//used voltage divider equation solved for R2 from https://en.wikipedia.org/wiki/Voltage_divider#Resistive_divider
//could probably add a fudge factor for series resistance of 3 relays and probes, maybe just subtract from final value?
//modified for use with 100k input impedance, may need tweaking
void ohmMeter(){
  int read = 0;
  digitalWrite(L_CTL, HIGH); // connect v/ohm in to adc for all of ohmeter reading
  digitalWrite(V_OHM_CTL, LOW); //disconnect bottom of r ladder from in common
  digitalWrite(OHM_GND_CTL, HIGH); //connects in common to ground
  digitalWrite(OHM_OUT, HIGH); //ohm v source
  digitalWrite(M_CTL, LOW); //setting defaults for robustness
  digitalWrite(H_CTL, LOW);
  delay(2); //required for relay switching 
  read = analogRead(V_ADC) - (analogRead(COM_ADC)); //read with full resistor ladder (100k)
  if(read < 400){
    //digitalWrite(, LOW);
    digitalWrite(M_CTL, HIGH); //bypass 90k
    delay(2); //relay switch
    read = analogRead(V_ADC) - (analogRead(COM_ADC)); //read with  9k and 1k (10k)
    if(read < 400){
      //digitalWrite(, LOW);
      digitalWrite(H_CTL, HIGH); //bypass 9k
      delay(2); //relay switch
      read = analogRead(V_ADC) - (analogRead(COM_ADC)); //read with 1k
      simpPrint4("1k range",((1/((3.3/(read*adc2v))-1))*(highRangeRes)),"Ohms" , read);
    }
    else{
      simpPrint4("10k range",((1/((3.3/(read*adc2v))-1))*(highRangeRes + medRangeRes)),"Ohms" , read);
    }
  }
  else{
    simpPrint4("100k range",((1/((3.3/(read*adc2v))-1))*(highRangeRes + medRangeRes + lowRangeRes)),"Ohms" , read);
  }    
  
  digitalWrite(L_CTL, LOW); //refer to beginning of function 
  digitalWrite(M_CTL, LOW); //resetting to defaults
  digitalWrite(H_CTL, LOW);
  digitalWrite(OHM_GND_CTL, LOW);  
  digitalWrite(V_OHM_CTL, HIGH);  //may need to change, refer to call in setup
  digitalWrite(OHM_OUT, LOW);
  delay(2); //relay switch
}

void diodeMeter(){
  int read = 0;
  
  digitalWrite(V_OHM_CTL, LOW); //disconnect bottom of r ladder from in common
  digitalWrite(OHM_GND_CTL, HIGH); //connect in common to ground
  digitalWrite(OHM_OUT, HIGH); //connect bottom of r ladder to 3.3v source 
  digitalWrite(L_CTL, HIGH); //const 10k output resistance
  digitalWrite(M_CTL, HIGH); 
  digitalWrite(H_CTL, LOW);
  delay(2); //required for relay switching

  read = analogRead(V_ADC) - (analogRead(COM_ADC));

  if(abs(read) < 400){
    simpPrint4("Probably Continuous",((1/((3.3/(read*adc2v))-1))*(highRangeRes + medRangeRes)), "Ohms", read);
    PWM_Instance->setPWM(SPK, frequencyOn, dutyCycle);
  }
  else if(abs(read) < 3000){ //needs tweaking, thinking if voltage is significantly less than source, there is a conductive path or something 
    //simpPrint4("Probably a Diode",read*adc2v*totRes/(medRangeRes + highRangeRes),"Volts" , read);
    simpPrint4("Probably a Diode",read*adc2v,"Volts" , read);
    PWM_Instance->setPWM(SPK, frequencyOff, dutyCycle);
  }
  else{
    //simpPrint4("Probably not a Diode",read*adc2v*totRes/(medRangeRes + highRangeRes),"Volts" , read);
    simpPrint4("Probably not a Diode",read*adc2v,"Volts" , read);
    PWM_Instance->setPWM(SPK, frequencyOff, dutyCycle);
  }

  digitalWrite(V_OHM_CTL, HIGH); //set defaults
  digitalWrite(OHM_GND_CTL, LOW); 
  digitalWrite(OHM_OUT, LOW);  
  digitalWrite(L_CTL, LOW); 
  digitalWrite(M_CTL, LOW); 
  digitalWrite(H_CTL, LOW);
  delay(2); //required for relay switching

}


const float ampRes = 0.05; 
void ampMeter(){
  int read = 0;
  read = analogRead(A_ADC) - (analogRead(COM_ADC));
  simpPrint4("10A Range", read*adc2v/ampRes,"Amps" , read);  //ohms law, bitch
}

const int miliampRes = 5;
void miliampMeter(){
  int read = 0;
  read = analogRead(mA_ADC) - (analogRead(COM_ADC));
  simpPrint4("200mA Range", read*adc2v/miliampRes*1000,"Miliamps" , read);  //ohms law, bitch part 2
}

void connect_callback(uint16_t conn_handle)
{
  // Get the reference to current connection
  BLEConnection* connection = Bluefruit.Connection(conn_handle);

  char central_name[32] = { 0 };
  connection->getPeerName(central_name, sizeof(central_name));

  // TODO later
}

/**
 * Callback invoked when a connection is dropped
 * @param conn_handle connection where this event happens
 * @param reason is a BLE_HCI_STATUS_CODE which can be found in ble_hci.h
 */
void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  // TODO later
}

void setupBluetooth(void)
{
  // BLUEFRUIT INITIALIZATION
  // Initialise the Bluefruit module
  Bluefruit.begin();
  // Set name
  Bluefruit.setName("Metameter");
  // Set the connect/disconnect callback handlers
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

  // DEVICE INFORMATION SERVICE
  // Configure and Start the Device Information Service
  bledis.setManufacturer("Metameter");
  bledis.setModel("Metameter");
  bledis.begin();

  // MULTIMETER SERVICE
  // Setup the Multimeter service using
  // BLEService and BLECharacteristic classes
  multimeter_svc.begin();

  // VOLTAGE CHARACTERISTIC
  // Note: You must call .begin() on the BLEService before calling .begin() on
  // any characteristic(s) within that service definition.. Calling .begin() on
  // a BLECharacteristic will cause it to be added to the last BLEService that
  // was 'begin()'ed!
  // Setup voltage characteristic
  voltage_chr.setProperties(CHR_PROPS_NOTIFY);
  voltage_chr.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  voltage_chr.setFixedLen(4);
  voltage_chr.setPresentationFormatDescriptor(0x14, 0, 0x2728); // Set interpretation to float32
  voltage_chr.begin();
  voltage_chr.write32(0); // init voltage to 0

  // ADVERTISING
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  // Include Multimeter Service UUID
  Bluefruit.Advertising.addService(multimeter_svc);
  // Include Name
  Bluefruit.ScanResponse.addName();
  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   * 
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html   
   */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
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
  digitalWrite(OHM_GND_CTL, LOW);
  digitalWrite(H_CTL, LOW);
  digitalWrite(M_CTL, LOW);
  digitalWrite(L_CTL, LOW);

  pinMode(OHM_OUT, OUTPUT);
  pinMode(V_OHM_CTL, OUTPUT);
  pinMode(OHM_GND_CTL, OUTPUT);
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
  PWM_Instance = new nRF52_PWM(SPK, frequencyOff, dutyCycle);

  setupBluetooth();
}


int curTime = 0;
int lastTime = 0;
int mode = 0;  //current mode
void loop() {  //utilizing zoomy loops to avoid using interrupts (double triggering, never felt responsive enough) keep everything in the main loop fast (or you die)
  curTime = millis();
  if(curTime >= lastTime + 500){  //run this section of the loop every half second 
    if(mode == 1){

      PWM_Instance->setPWM(SPK, frequencyOff, dutyCycle);
      //BTmenu(); //TODO Yell at andrew (nicely)
    }
    else if(mode == 2){
      //testing speaker
      //PWM_Instance->setPWM(SPK, frequencyOn, dutyCycle);
      diodeMeter(); //TODO How the fuck do diodes work
    }
    else if(mode == 3){
      PWM_Instance->setPWM(SPK, frequencyOff, dutyCycle);
      miliampMeter(); //takes approx 0 ms, no relays
    }
    else if(mode == 4){
      PWM_Instance->setPWM(SPK, frequencyOff, dutyCycle);
      voltMeter();  //take approx 4 to 8 ms, could possibly be improved with better relay characterization
    }
    else if(mode == 5){
      PWM_Instance->setPWM(SPK, frequencyOff, dutyCycle);
      ohmMeter(); //takes approx 4 to 10 ms, see voltmeter call
    }
    else if(mode == 6){
      PWM_Instance->setPWM(SPK, frequencyOff, dutyCycle);
      ampMeter(); //takes approx 0 ms, no relay switching
    }


    
    lastTime = millis(); //run at end of loop for funsies
  }

  if (ts.tirqTouched()) { //check for touch, probably needs to be run rapidly to feel responsive  //TODO: Check how long it takes to run
    //if (ts.touched()) {  //not sure if this nest is necessary (nestecarry ehehuhehuhe)
      TS_Point point = ts.getPoint();
      
      if(touchInBoxPoint(h_off*0, v_off, h_off, h_off, point)){
        mode = 1;
      }
      else if(touchInBoxPoint(h_off*1, v_off, h_off, h_off, point)){
        mode = 2;
      }
      else if(touchInBoxPoint(h_off*2, v_off, h_off, h_off, point)){
        mode = 3;
      }
      else if(touchInBoxPoint(h_off*0, v_off-h_off, h_off, h_off, point)){
        mode = 4;
      }
      else if(touchInBoxPoint(h_off*1, v_off-h_off, h_off, h_off, point)){
        mode = 5;
      }
      else if(touchInBoxPoint(h_off*2, v_off-h_off, h_off, h_off, point)){
        mode = 6;
      }
    //}
  }
  
}
