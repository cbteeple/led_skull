
#include <EEPROM.h>
#include <EEPROMAnything.h>

// This is a demonstration on how to use an input device to trigger changes on your neo pixels.
// You should wire a momentary push button to connect from ground to a digital IO pin.  When you
// press the button it will change to a new pixel animation.  Note that you need to press the
// button once to start the first animation!

#include <Adafruit_NeoPixel.h>
//#include "PinChangeInterrupt.h"

//#define JULES
//#define DEBUG

#define BUTTON_PIN 2    // Digital IO pin connected to the button.  This will be
                          // driven with a pull-up resistor so the switch should
                          // pull the pin to ground momentarily.  On a high -> low
                          // transition the button press logic will execute.


#define PIXEL_PIN    6    // Digital IO pin connected to the NeoPixels.
#define PIXEL_PIN_1    5    // Digital IO pin connected to the NeoPixels.

  


#define PIXEL_COUNT 12
#define EYE_COUNT 6

#ifdef JULES
  #define PIXEL_COUNT_1 5
#else
  #define PIXEL_COUNT_1 4
#endif

const uint8_t BRIGHTNESS_DATA_START = 0;
const uint8_t SHOW_DATA_START = 4;

const int numBrightLevels = 6;
const float BRIGHT_LEVELS[numBrightLevels]={0.08,0.18,0.24,0.50,0.75,1.0};
uint8_t BrightnessIDX=0;

// Parameter 1 = number of pixels in strip,
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

#ifdef JULES
Adafruit_NeoPixel strip_hair = Adafruit_NeoPixel(PIXEL_COUNT_1, PIXEL_PIN_1, NEO_GRB + NEO_KHZ800);
#else
Adafruit_NeoPixel strip_hair = Adafruit_NeoPixel(PIXEL_COUNT_1, PIXEL_PIN_1, NEO_RGB + NEO_KHZ800);
#endif



bool oldState = HIGH;
int showType = 0;
int numShows = 6;
bool switchShows = true;
float Brightness = BRIGHT_LEVELS[numBrightLevels-1];
bool requiresLoop=0;
bool newButton=false;


unsigned long transitionTime = 0.5;
unsigned long timeBetweenPresses = 10000;  // the last time the output pin was toggled
unsigned long timeHeld =0;
unsigned long lastPressTime = 0;    // the debounce time; increase if the output flickers
unsigned long lastPressDownTime=0;
unsigned long brightPressTime=1000;
unsigned long brightInitTime=250;
unsigned long brightEndTime=3000;
unsigned long startBrightCurr=0;
unsigned long debouncing_time = 400; // [ms] Debouncing time
volatile unsigned long last_micros;


//Set the resting colors of things:
#ifdef JULES
uint8_t hair_r = 255;
uint8_t hair_g = 100;
uint8_t hair_b = 0;

uint8_t head_r = 255;
uint8_t head_g = 20;
uint8_t head_b = 0;

#else
uint8_t hair_r = 20;
uint8_t hair_g = 40;
uint8_t hair_b = 255;

uint8_t head_r = 255;
uint8_t head_g = 20;
uint8_t head_b = 0;
#endif


bool setBright = false;
bool firstcall=true;


void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(PIXEL_PIN, OUTPUT);
  pinMode(PIXEL_PIN_1, OUTPUT);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  strip_hair.begin();
  strip_hair.show(); // Initialize all pixels to 'off'

  #ifdef DEBUG
    Serial.begin(115200);
  #endif
  
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), debounceInterrupt, CHANGE);
  
}

void loop() {
  if(firstcall){
    readSettings();
    firstcall=false;
  }
  else{
    if (setBright){
      startBlink(150);
      startBlink(150);
      showColor(128,128,128);
      while ((millis()-startBrightCurr)<brightEndTime){
        showColor(128,128,128);
      }
      setBright=false;
      switchShows=true;
      }
    else{
      if (switchShows || requiresLoop){
        switchShows=false;
        startShow();
      }
    }
  }
}

void startShow() {
  switch (showType){
      case 0: {
        showColorAnimateHead(head_r, head_g, head_b,50);
        showColorAnimateHair(hair_r,hair_g,hair_b,60);
        requiresLoop=0;
        } //Red
        break;
      case 1: {
          genPulse(head_r, head_g,head_b,  hair_r,hair_g,hair_b,  9);
          requiresLoop=1;
        } //Red
        break;
      case 2: {genPulse(255, 255, 255,  255,255,255,  7);
          requiresLoop=1;
          }
          break;
      case 3: {rainbowCycleDim(10,1,0,true); //DISCO SKULL!!!!!
        requiresLoop=1;
        }
        break;
      case 4: {rainbowCycleDim(10,1,70,false); //DISCO SKULL CHOPPY!!!!!
        requiresLoop=1;
        }
        break;
      case 5: { showColor(255,0,0);
          rainbowCycleHair(3,1); //Rave Hair/Spikes!!!!
          requiresLoop=1;
        }
        break;
      default:{
        showColor(255,0,0);
        rainbowCycleHair(3,1); //Rave Hair/Spikes!!!!
      }
      break;
  }
}


//================================

// Same as RainbowCycle, except Dimmer
void rainbowCycleDim(uint8_t wait, uint8_t reps, uint8_t wait_hair, bool smooth) {
  uint16_t i, j;
  bool FullColor=true;

  for(j=0; j<256*reps; j++) { // 5 cycles of all colors on wheel
    if (setBright || switchShows){
      break ;
    }

    if (j%32==0 || smooth ){
      // Set the hair color in a nice wave back-to-front
      for(int k=strip_hair.numPixels()-1; k>=0; k--) {
        if (FullColor){
          strip_hair.setPixelColor(k, Wheel(j & 255));
          }
        else{
          strip_hair.setPixelColor(k, Wheel(((i * 256 / strip_hair.numPixels()) + j) & 255));
        }
        strip_hair.show();
        delay(wait_hair);
      }

      // Set the inside colors
      for(i=0; i< strip.numPixels(); i++) {
        if (FullColor){
          strip.setPixelColor(i, Wheel(j & 255));
          }
        else{
          strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
        }
      }
      
    }
    
    strip.show();
    delay(wait);
  }
}




void halfAndHalf(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2,uint16_t wait){
  uint16_t i;
    for(i=0; i< strip_hair.numPixels(); i++) {
      if (setBright || switchShows){
        break ;
      }
      strip_hair.setPixelColor(i, strip_hair.Color(r2*Brightness,g2*Brightness,b2*Brightness));
      strip_hair.show(); 
      delay(wait); 
    }
    
    for(i=0; i< strip.numPixels(); i++) {
      if (setBright || switchShows){
        break ;
      }
      if (i<strip.numPixels()/2){
        strip.setPixelColor(i, strip.Color(r1*Brightness,g1*Brightness,b1*Brightness));
      }
      else if ((i>=strip.numPixels()/2)){
        strip.setPixelColor(i, strip.Color(r2*Brightness,g2*Brightness,b2*Brightness));
      } 
      strip.show(); 
      delay(wait);   
    }
}



void rainbowCycleHair(uint8_t wait, uint8_t reps) {
  uint16_t i, j;

  for(j=0; j<256*reps; j++) { // 5 cycles of all colors on wheel
    if (setBright || switchShows){
      break ;
    }
    for(i=0; i< strip_hair.numPixels()+1; i++) {
      strip_hair.setPixelColor(i, Wheel_hair(((i * 256 / strip_hair.numPixels()+1) + j) & 255));

      // Set the whole head color as the last "LED"
      if (i==strip_hair.numPixels()){
        for (int k = 0; k< strip.numPixels(); k++){
          strip.setPixelColor(k, Wheel_hair(((i * 256 / strip_hair.numPixels()+1) + j) & 255));
        }
      }
    }
    strip_hair.show();
    strip.show();
    delay(wait);
  }
}

void pingPongHair(uint8_t r_base, uint8_t g_base, uint8_t b_base, uint8_t r_dot, uint8_t g_dot, uint8_t b_dot,uint16_t wait){
  
  
}



void halfAndHalfAnimated(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2,uint16_t wait){
  uint16_t i, j, k;
    //Set inital configuration
    halfAndHalf(r1, g1, b1, r2, g2, b2, 30);
    
    //Slowly rotate   
    for (k=0; k<strip.numPixels(); k++){
      for (j=0; j<128; j++){
        if (setBright || switchShows){
          return;
        }
        float ratio=float(j/128.0);
        strip.setPixelColor((k+strip.numPixels()/2)%(strip.numPixels()), strip.Color(((ratio*r1)+((1.0-ratio)*r2))*Brightness,((ratio*g1)+((1.0-ratio)*g2))*Brightness,((ratio*b1)+((1.0-ratio)*b2))*Brightness));
        strip.setPixelColor(k, strip.Color(((ratio*r2)+((1.0-ratio)*r1))*Brightness,((ratio*g2)+((1.0-ratio)*g1))*Brightness,((ratio*b2)+((1.0-ratio)*b1))*Brightness));
        strip.show(); 
        delay(wait);   
      }
    }
    
}




void genPulse(uint8_t red, uint8_t green,uint8_t blue,uint8_t red_h, uint8_t green_h,uint8_t blue_h,uint16_t wait){
  
  bool flip = true;
  float bright1=Brightness;
  float bright2=Brightness/4;
  float bright_tmp = bright1;
  int maxSteps=127;
  float progress=0.0;

  showColorAnimateHead(red/float(4), green/float(4), blue/float(4),50);
  showColorAnimateHair(red_h/float(4), green_h/float(4), blue_h/float(4),50);

  for (uint8_t k=0;k<2;k++){
    for (uint16_t i=0; i<maxSteps+1; i++){
      if (setBright || switchShows){
        break;
      }      
      progress=BezierBlend(float(i)/float(maxSteps));
       
      //Calculate the brightness to use;
      if (flip){
        bright_tmp=(progress)*bright1+(1-progress)*bright2;
      }
      else{
        bright_tmp=(progress)*bright2+(1-progress)*bright1;
      }
  
      //Apply that brightness to all pixels
      for (uint8_t j=0; j<strip.numPixels(); j++){
          strip.setPixelColor(j, strip.Color(red*bright_tmp,green*bright_tmp,blue*bright_tmp));
      }
      for (uint8_t j=0; j<strip_hair.numPixels(); j++){
          strip_hair.setPixelColor(j, strip_hair.Color(red_h*bright_tmp,green_h*bright_tmp,blue_h*bright_tmp));
      }
  
      //update values to al pixels
      strip.show(); 
      strip_hair.show(); 
  
      //Wait the correct amount of time
      if (flip){
        delay(wait);
      }
      else{
      delay(wait*2.5);  
      }
      
    }
    flip=!flip;
  }
  delay(wait*20); 
}



void showColor(uint8_t red,uint8_t green,uint8_t blue){
  for(int i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(red*Brightness,green*Brightness,blue*Brightness));
  }

  for(int i=0; i< strip_hair.numPixels(); i++) {
      strip_hair.setPixelColor(i, strip_hair.Color(red*Brightness,green*Brightness,blue*Brightness));
  }
    strip.show();  
    strip_hair.show();  
}


void showColorAnimate(uint8_t red,uint8_t green,uint8_t blue,uint16_t wait){
  for(int i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(red*Brightness,green*Brightness,blue*Brightness));
      strip.show(); 

      if (i < strip_hair.numPixels()){
        strip_hair.setPixelColor(i, strip.Color(red*Brightness,green*Brightness,blue*Brightness));
        strip_hair.show();
      }
      
      delay(wait);
  }
     
}


void showColorAnimateHead(uint8_t red,uint8_t green,uint8_t blue,uint16_t wait){
  for(int i=0; i< strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(red*Brightness,green*Brightness,blue*Brightness));
    strip.show();      
    
    delay(wait);
  }
     
}


void showColorAnimateHair(uint8_t red,uint8_t green,uint8_t blue,uint16_t wait){
  for(int i=0; i< strip_hair.numPixels(); i++) {
    strip_hair.setPixelColor(i, strip_hair.Color(red*Brightness,green*Brightness,blue*Brightness));
    strip_hair.show();      
    
    delay(wait);
  }
     
}


void showRainbow(uint16_t wait){
  uint16_t j=0;
  uint16_t i=0;
  for(i=0; i< strip.numPixels(); i++) {
    if (setBright || switchShows){
      return;
    }
    strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    strip.show(); 
    delay(wait); 
  }
}


void startBlink(uint16_t wait){
  showColor(128,128,128);
  delay(wait);
  showColor(0,0,0);
  delay(wait);
}



void roboCop(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r_hair, uint8_t g_hair, uint8_t b_hair,uint16_t wait){
  uint8_t r2 = 0;
  uint8_t g2 = 0;
  uint8_t b2 = 0;
  
  uint16_t i, j, k;
    //Set inital configuration
    showColorAnimateHead(r1,g1,b1,60);
    showColorAnimateHair(r_hair,g_hair,b_hair,60);
      
    for(i=0; i< EYE_COUNT; i++) {
      if (i<EYE_COUNT/2){
        strip.setPixelColor(i, strip.Color(r1*Brightness,g1*Brightness,b1*Brightness));
      }
      else if ((i>=EYE_COUNT/2)){
        strip.setPixelColor(i, strip.Color(r2*Brightness,g2*Brightness,b2*Brightness));
      } 
    }
    strip.show();   
    
    //Slowly rotate
    bool flip = false;
    while(!(setBright || switchShows)){
      for (k=0; k<EYE_COUNT; k++){
        for (j=0; j<128; j++){
          if (setBright || switchShows){
            return;
          }
          float ratio=float(j/128.0);
          strip.setPixelColor((k+EYE_COUNT/2)%(EYE_COUNT), strip.Color(((ratio*r1)+((1.0-ratio)*r2))*Brightness,((ratio*g1)+((1.0-ratio)*g2))*Brightness,((ratio*b1)+((1.0-ratio)*b2))*Brightness));
          strip.setPixelColor(k, strip.Color(((ratio*r2)+((1.0-ratio)*r1))*Brightness,((ratio*g2)+((1.0-ratio)*g1))*Brightness,((ratio*b2)+((1.0-ratio)*b1))*Brightness));
          strip.show(); 
          delay(wait);   
        }
      }
    }
    
}




//================================
//HELPER FUNCTIONS

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color((255 - WheelPos * 3)*Brightness, 0, (WheelPos * 3)*Brightness);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, (WheelPos * 3)*Brightness, (255 - WheelPos * 3)*Brightness);
  }
  WheelPos -= 170;
  return strip.Color((WheelPos * 3)*Brightness, (255 - WheelPos * 3)*Brightness, 0);
}


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel_hair(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip_hair.Color((255 - WheelPos * 3)*Brightness, 0, (WheelPos * 3)*Brightness);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip_hair.Color(0, (WheelPos * 3)*Brightness, (255 - WheelPos * 3)*Brightness);
  }
  WheelPos -= 170;
  return strip_hair.Color((WheelPos * 3)*Brightness, (255 - WheelPos * 3)*Brightness, 0);
}



/* InOutQuadBlend takes in elements of a vector of times [0;1]
 *  and returns wait times. 
 */

float InOutQuadBlend(float t)
{
    if(t <= 0.5f)
        return (2.0f * t*t);
    t -= 0.5;
    return (2.0f * t * (1.0f - t) + 0.5);
}




float BezierBlend(float t)
{
    return (t*t) * (3.0f - 2.0f * t);
}




//ISR
void debounceInterrupt() {
  if((long)(micros() - last_micros) >= debouncing_time * 1000) {
    bool reading = digitalRead(BUTTON_PIN);
    if(reading==LOW){ //Falling
      lastPressDownTime = millis();
    }
    else{   
      switchShow();
      last_micros = micros();
    }
  }
}



boolean switchShow(){
  unsigned long currTime = millis();

  if(setBright){
    incrementBrightness();
    startBrightCurr=millis();
  }
  else{
    if((currTime-lastPressDownTime) > brightPressTime){
      setBright=true;
      //Serial.println("Set Brightness");
      startBrightCurr=millis();
    }
    else{
      showType++;
      if (showType >= numShows){
        showType=0;
      }
      switchShows=true;
      //Serial.println("Switch Shows");
    }

    lastPressTime=currTime;
  }
  saveSettings();
}


void incrementBrightness(){
//Update the brightness
  BrightnessIDX++;
  if (BrightnessIDX>=numBrightLevels){
    BrightnessIDX=0;
  }
  Brightness=BRIGHT_LEVELS[BrightnessIDX];

}




void saveSettings(){
  //EEPROM_writeAnything(BRIGHTNESS_DATA_START, BrightnessIDX);
  EEPROM.write(BRIGHTNESS_DATA_START, BrightnessIDX);
  EEPROM.write(SHOW_DATA_START, showType);
  //EEPROM_writeAnything(, showType);
}

void readSettings(){
    //Read old settings from EEPROM
  //EEPROM_readAnything(BRIGHTNESS_DATA_START, BrightnessIDX);
  uint8_t tmpShow= EEPROM.read(SHOW_DATA_START);
  uint8_t tmpBright= EEPROM.read(BRIGHTNESS_DATA_START);

  
  //If show number is unreasonable, go back to default
  if (tmpShow<numShows & tmpShow>=0){
    showType = tmpShow;
  }

  if (tmpBright>0 & tmpBright<(numBrightLevels-1)){
    BrightnessIDX = tmpBright;
    Brightness=BRIGHT_LEVELS[BrightnessIDX];
  }
}




