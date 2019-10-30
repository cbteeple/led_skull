
#include <EEPROM.h>
#include <EEPROMAnything.h>

// This is a demonstration on how to use an input device to trigger changes on your neo pixels.
// You should wire a momentary push button to connect from ground to a digital IO pin.  When you
// press the button it will change to a new pixel animation.  Note that you need to press the
// button once to start the first animation!

#include <Adafruit_NeoPixel.h>
//#include "PinChangeInterrupt.h"

//#define JULES

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

const uint8_t SHOW_DATA_START = 4;

// Parameter 1 = number of pixels in strip,  neopixel stick has 8
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream, correct for neopixel stick
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip), correct for neopixel stick
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

#ifdef JULES
Adafruit_NeoPixel strip_hair = Adafruit_NeoPixel(PIXEL_COUNT_1, PIXEL_PIN_1, NEO_GRB + NEO_KHZ800);
#else
Adafruit_NeoPixel strip_hair = Adafruit_NeoPixel(PIXEL_COUNT_1, PIXEL_PIN_1, NEO_RGB + NEO_KHZ800);
#endif



bool oldState = HIGH;
int showType = 0;
int numShows = 8;
bool switchShows = false;
float Brightness = 1.0;
bool requiresLoop=0;
bool newButton=false;


unsigned long transitionTime = 0.5;
unsigned long timeBetweenPresses = 10000;  // the last time the output pin was toggled
unsigned long timeHeld =0;
unsigned long lastPressTime = 0;    // the debounce time; increase if the output flickers
unsigned long brightPressTime=1000;
unsigned long brightInitTime=250;
unsigned long brightEndTime=1500;
unsigned long startBrightCurr=0;

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
  
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), debounce, CHANGE);
  
}

void loop() {
  if(firstcall){
    readSettings();
    firstcall=false;
  }
  else{
    switchShows=false;
    startShow();
  }
}

void startShow() {
  switch (showType){
      case 0: {showColorAnimate(255, 20, 0,50);
          requiresLoop=0;
        } //Red
        break;
      case 1: {
          //halfAndHalf(0, 255, 0,255, 255, 0,20);
          genPulse(255, 20, 0,15);
          requiresLoop=0;
        } //Red
        break;
      case 2:{
          halfAndHalf(0, 164, 255,63, 0, 255,1);
          //showColorAnimate(0, 164, 255,30);
          requiresLoop=0;
        }//Yellow
        break;
     case 3:{
          halfAndHalfAnimated(0, 164, 255,63, 0, 255,5);
          //showColorAnimate(0, 164, 255,30);
          requiresLoop=0;
        }//Yellow
        break;
      case 4: {
          //genPulse(255, 255, 0,20);
          genPulse(0, 164, 255,10);
          requiresLoop=0;
        } //Red
        break;
      case 41:{showColorAnimate(0, 255, 0,30);
          requiresLoop=0;
        }//Green
        break;
      case 51: {genPulse(0, 255, 0,20);
          requiresLoop=0;
        } //Red
        break;
      case 61:{ showColorAnimate(0, 30, 255,30); //Blue
          requiresLoop=0;
        }
        break;
      case 71: {genPulse(0, 30, 255,20);
          requiresLoop=0;
        } //Red
        break;
      case 81:{ showColorAnimate(120, 0, 255,30); //Purple
          requiresLoop=0;
        }
        break;
      case 91: {genPulse(120, 0, 255,20);
          requiresLoop=0;
        } //Red
        break;
      case 101:{ showColorAnimate(255, 0, 255,30); //Pink
          requiresLoop=0;
        }
        break;
      case 111: {genPulse(255, 0, 255,20);
          requiresLoop=0;
        } //Red
        break;
      case 121: {showRainbow(30); //Rainbow
        requiresLoop=0;
        }
        break;
        //Michigan
      case 141: halfAndHalfAnimated(255,255,0,0,31,173,20);
        break;
        //Michigan State
      case 131: halfAndHalfAnimated(255,255,255,50,200,100,20);
        break;
      case 5: rainbowCycleDim(10,1,true); //DISCO SKULL!!!!!
        break;
      case 6: {roboCop(255, 10, 0,0, 0, 0,1);
          //showColorAnimate(0, 164, 255,30);
          requiresLoop=0;
      }
        break;
      case 7: { showColor(255,0,0);
          rainbowCycleHair(5,1);
          requiresLoop=1;
      }
        break;
      case 161: rainbowCycleDim(5,1,false);
        break;
      case 171: rainbowCycleDim(20,1,false);
        break;
  }
}


//================================

// Same as RainbowCycle, except Dimmer
void rainbowCycleDim(uint8_t wait, uint8_t reps, bool FullColor) {
  uint16_t i, j;

  for(j=0; j<256*reps; j++) { // 5 cycles of all colors on wheel
    if (setBright || switchShows){
      break ;
    }
    
    for(i=0; i< strip.numPixels(); i++) {
      if (FullColor){
        strip.setPixelColor(i, Wheel(j & 255));
        }
      else{
        strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
      }
    }

    for(i=0; i< strip_hair.numPixels(); i++) {
      if (FullColor){
        strip_hair.setPixelColor(i, Wheel(j & 255));
        }
      else{
        strip_hair.setPixelColor(i, Wheel(((i * 256 / strip_hair.numPixels()) + j) & 255));
      }
    }
    
    strip.show();
    strip_hair.show();
    delay(wait);
  }
}




void halfAndHalf(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2,uint16_t wait){
  uint16_t i;
    for(i=0; i< strip.numPixels(); i++) {
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
    for(i=0; i< strip_hair.numPixels(); i++) {
      strip_hair.setPixelColor(i, Wheel_hair(((i * 256 / strip_hair.numPixels()) + j) & 255));
    }
    strip_hair.show();
    delay(wait);
  }
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




void genPulse(uint8_t red, uint8_t green,uint8_t blue,uint16_t wait){
  
  bool flip = true;
  float bright1=Brightness;
  float bright2=Brightness/3;
  float bright_tmp = bright1;
  int maxSteps=127;
  float progress=0.0;

  showColorAnimate(red/float(3), green/float(3), blue/float(3),50);

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
          strip_hair.setPixelColor(j, strip_hair.Color(red*bright_tmp,green*bright_tmp,blue*bright_tmp));
      }
  
      //update values to al pixels
      strip.show(); 
      strip_hair.show(); 
  
      //Wait the correct amount of time
      delay(wait);
    }
    flip=!flip;
  } 
}






//================================
//HELPER FUNCTIONS

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


void showRainbow(uint16_t wait){
  uint16_t j=0;
  uint16_t i=0;
  for(i=0; i< strip.numPixels(); i++) {
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


void hairOnly(uint16_t wait){
  uint16_t j=0;
  uint16_t i=0;
  for(i=0; i< strip_hair.numPixels(); i++) {
    strip_hair.setPixelColor(i, Wheel(((i * 256 / strip_hair.numPixels()) + j) & 255));
    strip_hair.show(); 
    delay(wait); 
  }
}


void roboCop(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2,uint16_t wait){
  uint16_t i, j, k;
    //Set inital configuration
    showColor(r1,g1,b1);
    
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



//ISR
boolean debounce(){
cli();
delay(20);
int reading = digitalRead(BUTTON_PIN);

if (reading==LOW){ 
  timeBetweenPresses=millis()-lastPressTime;
  lastPressTime=millis();
   
  }
  else{
    showType++;
      if (showType >= numShows){
        showType=0;
      }
      switchShows=true;  
  }
  saveSettings();
  sei();
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



void saveSettings(){
  //EEPROM_writeAnything(BRIGHTNESS_DATA_START, Brightness);
  EEPROM.write(SHOW_DATA_START, showType);
  //EEPROM_writeAnything(, showType);
}

void readSettings(){
    //Read old settings from EEPROM
  //EEPROM_readAnything(BRIGHTNESS_DATA_START, Brightness);
  uint8_t tmpShow= EEPROM.read(SHOW_DATA_START);

  
  //If show number is unreasonable, go back to default
  if (tmpShow<numShows & tmpShow>=0){
    showType = tmpShow;
  }
}



