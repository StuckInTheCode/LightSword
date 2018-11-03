//defined parameters
#define NUM_LEDS 30         // number of leds in the strip
#define BTN_TIMEOUT 500     // button waiting time until pressed processing
#define BRIGHTNESS 255      // maximum

#define DEBUG 0      
#define LED_PIN 6           // led din pin
#define BTN 3               // button pin
#define BTN_LED 4           // control button led

// library
#include "FastLED.h"        // library for the strip
#include <EEPROM.h>         // memory

// arrray of leds
CRGB leds[NUM_LEDS];

boolean cngRequest, swordState;         //change state request, sword state 
boolean btnState, btn_flag, hold_flag;  //button state
byte btn_counter;                       //count of pressing
unsigned long btn_timer;                //time of button pressing
byte nowNumber;
byte LEDcolor; 
byte curColor, red, green, blue, redOffset, greenOffset, blueOffset;
boolean eeprom_flag;


void setup() {
  FastLED.addLeds<WS2811, LED_PIN, GRB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(100);  
  pinMode(BTN, INPUT_PULLUP);
  pinMode(BTN_LED, OUTPUT);
  digitalWrite(BTN_LED, 1);


  randomSeed(analogRead(2));    // random generator

  if ((EEPROM.read(0) >= 0) && (EEPROM.read(0) <= 5)) {  // if sword was activated
    curColor = EEPROM.read(0);   // reset prev color
  } else {                       
    EEPROM.write(0, 0);          // set 0 color
    curColor = 0;                
  }

  setColor(curColor);                  
  
  delay(1000);                        
  setAll(0, 0, 0);                     // set black
  FastLED.setBrightness(BRIGHTNESS);   //max brightness
}

void loop() {
  lightEffect();      
  on_off();             //power on/off the sword
  buttonOp();          // operation with control button
}

void on_off() {                    
  if (cngRequest) {                
    if (!swordState)               //sword is on
    {                 
        lightOn();  
        if (DEBUG) Serial.println(F("ON"));                
        delay(200);  
        swordState = true;          //change state to power on
    } else 
    {                        
      lightOff();   
      if (DEBUG) Serial.println(F("OFF"));              
      delay(300);                   
      swordState = false;             
      if (eeprom_flag) 
      {                              // if color was changed
        eeprom_flag = 0;
        EEPROM.write(0, curColor);   // save to the eeprom
      }
    }
    cngRequest = 0;                //reset change state request
  }
}

void buttonOp() {
  btnState = !digitalRead(BTN);    // true if button was pressed
  if (btnState && !btn_flag) {
    if (DEBUG) Serial.println(F("BTN PRESS"));
    btn_flag = 1;
    btn_counter++;                 // increase count of pressing
    btn_timer = millis();
  }

  // button still pressed
  if (btn_flag && btnState && (millis() - btn_timer > BTN_TIMEOUT) && !hold_flag) {
    cngRequest = 1;              // set request to change state of the sword
    hold_flag = 1;
    btn_counter = 0;
  }
//////////////////////////////////////////////////////////////////
    if (!btnState && btn_flag) {     // button was released
    btn_flag = 0;
    hold_flag = 0;           
  }
}

void lightEffect() {
}


void setPixel(int index, byte red, byte green, byte blue) {
  leds[index].r = red;
  leds[index].g = green;
  leds[index].b = blue;
}

// set all pixels to the inputed color
void setAll(byte red, byte green, byte blue) {
  for (int i = 0; i < NUM_LEDS; i++ ) {
    setPixel(i, red, green, blue);
  }
  FastLED.show();
}

//animation of power on
void lightOn() {
  char i0 = 0;
  char i1 = NUM_LEDS / 4;
  char i2 = NUM_LEDS / 2;
  char i3 = (NUM_LEDS * 3) / 4;
  for (i0,i1,i2,i3; i0 <= (NUM_LEDS / 4 - 1); i0++,i1++,i2++,i3++) {  //set every 4th led to the current color
    setPixel(i0, red, green, blue);
    setPixel(i1, red, green, blue);
    setPixel(i2, red, green, blue);
    setPixel(i3, red, green, blue);
    //setPixel((NUM_LEDS - 1 - i), red, green, blue);
    FastLED.show();
    delay(25);
  }
}

// animation of power off
void lightOff() {
  char i0 = NUM_LEDS / 4 -1;
  char i1 = NUM_LEDS / 2 - 1;
  char i2 = (NUM_LEDS * 3) / 4 -1 ;
  char i3 = NUM_LEDS ;
  for (i0,i1,i2,i3; i0 >=0; i0--, i1--, i2--, i3--) {         
    setPixel(i0, red, green, blue);
    setPixel(i1, red, green, blue);
    setPixel(i2, red, green, blue);
    setPixel(i3, red, green, blue);
    //setPixel((NUM_LEDS - 1 - i), red, green, blue);
    FastLED.show();
    delay(25);
  }
}


void setColor(byte c) {
  switch (c) {
    // 0 - red, 1 - yellow, 2 - green, 3 - sky, 4 - blue, 5 - puple
    case 0:
      red = 255;
      green = 0;
      blue = 0;
      break;
    case 1:
      red = 255;
      green = 255;
      blue = 0;
      break;
    case 2:
      red = 0;
      green = 255;
      blue = 0;
      break;
    case 3:
      red = 0;
      green = 0;
      blue = 255;
      break;

    case 4:
      red = 0;
      green = 255;
      blue = 255;
      break;
      
    case 5:
      red = 255;
      green = 0;
      blue = 255;
      break;

  }
}
