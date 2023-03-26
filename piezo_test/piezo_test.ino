

#define _PWM_LOGLEVEL_       4
#define USING_TIMER       false   //true

#include "nRF52_PWM.h"
#define SPK       13

//creates pwm instance
nRF52_PWM* PWM_Instance;

float frequency = 1000.0f;

float dutyCycle = 0.0f;

void setup(){
  PWM_Instance = new nRF52_PWM(SPK, frequency, dutyCycle);
}

void playFreq(float freq, float sec){
  frequency = freq;
  dutyCycle = 50.0f;
  PWM_Instance->setPWM(SPK, frequency, dutyCycle);
  delay(sec);
}



void loop(){
  for(int i = 3000; i < 5000; i++){
    playFreq(i,5);
  }
}
