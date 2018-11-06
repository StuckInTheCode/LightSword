
//defined parameters
#define NUM_LEDS 30         // number of leds in the strip
#define BTN_TIMEOUT 500     // button waiting time until pressed processing
#define BRIGHTNESS 255      // maximum
#define SWING 100           // minimum angular velocity
#define STRONG_SWING 200    
#define BLOW 100            // minimum acceleration
#define HARD_BLOW 200      

#define DEBUG 0      
#define LED_PIN 6           // led din pin
#define BTN 3               // button pin
#define BTN_LED 4           // control button led
#define SD_CARD_GND A0           


// library
#include "Wire.h"           // вспомогательная библиотека для работы с акселерометром
#include "I2Cdev.h"         // вспомогательная библиотека для работы с акселерометром
#include "MPU6050.h"        // accel-gyroscope library
#include "FastLED.h"        // library for the strip
#include <EEPROM.h>         // memory
#include <SD.h>
#include <TMRpcm.h>         // play audio

//object for sounds
TMRpcm sounds;

// arrray of leds
CRGB leds[NUM_LEDS];

//instance of the accel-gyroscope
MPU6050 sensor;

boolean cngRequest, swordState;         //change state request, sword state 
boolean btnState, btn_flag, hold_flag;  //button state
boolean soundEnable;
byte btn_counter;                       //count of pressing
unsigned long btn_timer;                //time of button pressing
unsigned long sound_timer;              //time of main sound playing
unsigned long effect_timer;             //time of light effect
unsigned long sensor_timer,swing_timer, swing_timeout;             //time of light effect
byte nowNumber;
byte LEDcolor; 
byte curColor, red, green, blue, redOffset, greenOffset, blueOffset;
boolean eeprom_flag, strike_flag;
//values of coordinates get from the sensor
int16_t ax, ay, az;
int16_t gx, gy, gz;
unsigned long ACC, GYR;
int gyrX, gyrY, gyrZ, accX, accY, accZ;


void setup(){
  FastLED.addLeds<WS2811, LED_PIN, GRB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(100);  
  pinMode(BTN, INPUT_PULLUP);
  pinMode(BTN_LED, OUTPUT);
  pinMode(SD_CARD_GND, OUTPUT);
  digitalWrite(SD_CARD_GND, 0);
  digitalWrite(BTN_LED, 1);

  sounds.speakerPin = 9;  //setup read sounds from sd
  sounds.setVolume(3); sounds.quality(1);
  if (DEBUG) 
  {
    if (SD.begin(8)) 
      Serial.println(F("SD OK"));
    else 
      Serial.println(F("SD fail"));
  } 
  else 
      SD.begin(8);

  sensor.initialize(); //setup sensor
  sensor.setFullScaleGyroRange(MPU6050_GYRO_FS_250);
  sensor.setFullScaleAccelRange(MPU6050_ACCEL_FS_16);

  if (DEBUG) {
    if (sensor.testConnection())  
      Serial.println(F("sensors ok"));
    else
      Serial.println(F("sensors fail"));
  }

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

void blow() {
  if ((ACC > BLOW) && (ACC < HARD_BLOW)) {      // если ускорение превысило порог;
    sounds.play("Blow.wav");               // воспроизвести звук удара
    sound_timer = millis() - 9000 + 550;
    strike_flag = 1;
  }
  if (ACC >= HARD_BLOW) {           // если ускорение превысило порог
    sounds.play("StrBlow.wav");               // воспроизвести звук удара
    sound_timer = millis() - 9000 + 700;
    strike_flag = 1;
  }
}

void swing(){
  if (GYR > 80 && (millis() - swing_timeout > 100)) {
    swing_timeout = millis();
    if (((millis() - swing_timer) > 100) && !strike_flag) {
      if (GYR >= STRONG_SWING) {      // если ускорение превысило порог
        sounds.play("Swing.wav");               // воспроизвести звук взмаха
        sound_timer = millis() - 9000 + 350;
        swing_timer = millis();
      }
      if ((GYR > SWING) && (GYR < STRONG_SWING)) {
        sounds.play("StrSwing.wav");               // воспроизвести звук взмаха
        sound_timer = millis() - 9000 + 390;
        swing_timer = millis();
      }
    }
  }
}

void loop(){
  
  lightEffect();      
  on_off();             //power on/off the sword
  if (soundEnable && ((millis() - sound_timer) >= 6000) ) {           // play main sound 6 seconds
    sounds.play("MainSound.wav");
    sound_timer = millis(); 
    strike_flag = 0;                                        
  }
  buttonOp();           // operation with control button
  blow();
  swing();
  
}

void on_off(){                    
  if (cngRequest) {                
    if (!swordState)                 //sword is on
    {                 
        lightOn();  
        soundEnable = 1;
        sounds.play("On.wav");       // play on sound
        if (DEBUG) Serial.println(F("ON"));                
        delay(200);  
        swordState = true;          //change state to power on
    } 
    else 
    {                        
      lightOff();   
      sounds.play("Off.wav");         // play off souns
      soundEnable = 0;
      if (DEBUG) Serial.println(F("OFF")); 
      sounds.disable();              
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

void buttonOp(){
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

void checkSensorState(){
  if (swordState) {                                               
    if (millis() - sensor_timer > 300) {             
                    
      sensor.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);       // every 300ms save checked values

      // найти абсолютное значение, разделить на 100
      gyrX = abs(gx / 100);
      gyrY = abs(gy / 100);
      gyrZ = abs(gz / 100);
      accX = abs(ax / 100);
      accY = abs(ay / 100);
      accZ = abs(az / 100);

      
      if(DEBUG){
         Serial.print("ACC ");
         Serial.print(accX);
         Serial.print(" ");
         Serial.print(accY);
         Serial.print(" ");
         Serial.print(accZ);
         Serial.println(" ");

         Serial.print("GYR ");
         Serial.print(gyrX);
         Serial.print(" ");
         Serial.print(gyrY);
         Serial.print(" ");
         Serial.print(gyrZ);
         Serial.println(" ");
         
    }

      // найти среднеквадратичное (сумма трёх векторов в общем)
      ACC = sq((long)accX) + sq((long)accY) + sq((long)accZ);
      ACC = sqrt(ACC);
      GYR = sq((long)gyrX) + sq((long)gyrY) + sq((long)gyrZ);
      GYR = sqrt((long)GYR);

    sensor_timer = millis();
  }
}
}

void lightEffect(){
  if (swordState && ((millis() - effect_timer) >= 50) ) {           // show 
    if(nowNumber == 11)
    nowNumber = 0;
    
      char i0 = nowNumber;
      char i1 = NUM_LEDS / 2 - nowNumber - 1;
      char i2 = NUM_LEDS / 2 + nowNumber;
      char i3 = NUM_LEDS - nowNumber - 1;
      
      setPixel(i0++, red, green, blue);                              //set prev pixels to the main color
      setPixel(i1++, red, green, blue);
      setPixel(i2++, red, green, blue);
      setPixel(i3++, red, green, blue);
      
      for (i0,i1,i2,i3; i0 < nowNumber + 4; i0++,i1--,i2++,i3--){   //set 3 next pixels to an another color  
          setPixel(i0, red + 20, green+ 20, blue+ 20);
          setPixel(i1, red+ 20, green+ 20, blue+ 20);
          setPixel(i2, red+ 20, green+ 20, blue+ 20);
          setPixel(i3, red+ 20, green+ 20, blue+ 20);
    }
    FastLED.show();
    nowNumber++;
  }
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
  char i1 = NUM_LEDS / 2 - 1;
  //char i1 = NUM_LEDS / 4;
  char i2 = NUM_LEDS / 2;
  char i3 = NUM_LEDS - 1;
  //char i3 = (NUM_LEDS * 3) / 4;
  for (i0,i1,i2,i3; i0 <= (NUM_LEDS / 4 - 1); i0++,i1--,i2++,i3--) {  //set every 4th led to the current color
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
  //char i1 = NUM_LEDS / 2 - 1;
  char i1 = NUM_LEDS / 4;
  char i2 = (NUM_LEDS * 3) / 4 -1 ;
  //char i3 = NUM_LEDS - 1 ;
  char i3 = (NUM_LEDS * 3) / 4;
  for (i0,i1,i2,i3; i0 >=0; i0--, i1++, i2--, i3++) {         
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
