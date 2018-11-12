
//defined parameters
#define NUM_LEDS 20         // number of leds in the strip
#define BTN_TIMEOUT 800     // button waiting time until pressed processing
#define BRIGHTNESS 200      // maximum
#define SWING 100           // minimum angular velocity
#define STRONG_SWING 200    
#define BLOW 45           // minimum acceleration
#define HARD_BLOW 60      
#define SOUND 9000

#define DEBUG 1      
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
boolean eeprom_flag, strike_flag, swing_flag, lightFlag;
//values of coordinates get from the sensor
int16_t ax, ay, az;
int16_t gx, gy, gz;
unsigned long ACC, GYR;
int gyrX, gyrY, gyrZ, accX, accY, accZ;


void setup(){
  if(DEBUG)
  Serial.begin(9600); 
  Wire.begin();
  
  FastLED.addLeds<WS2811, LED_PIN, GRB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(100);  
  pinMode(BTN, INPUT_PULLUP);
  pinMode(BTN_LED, OUTPUT);
  pinMode(SD_CARD_GND, OUTPUT);
  digitalWrite(SD_CARD_GND, 0);
  digitalWrite(BTN_LED, 1);

  sounds.speakerPin = 9;  //setup read sounds from sd
  sounds.setVolume(5);
  sounds.quality(1);
  if (DEBUG) 
  {
    if (SD.begin(8)) 
      Serial.println(F("SD OK"));
    else 
      Serial.println(F("SD fail"));
  } 
  else {
      SD.begin(8);
  }
  
  sensor.initialize(); //setup sensor
  sensor.setFullScaleGyroRange(MPU6050_GYRO_FS_250);
  sensor.setFullScaleAccelRange(MPU6050_ACCEL_FS_16);

  if (DEBUG) {
    if (sensor.testConnection())  
      Serial.println(F("sensors ok"));
    else
      Serial.println(F("sensors fail"));
  }

  randomSeed(analogRead(2));              // random generator

  if ((EEPROM.read(0) >= 0) && (EEPROM.read(0) <= 5)) {  // if sword was activated
    curColor = EEPROM.read(0);            // reset prev color
  } else {                       
    EEPROM.write(0, 0);                   // set 0 color
    curColor = 0;                
  }
  swordState = 0;
  setColor(curColor);                  
  
  delay(1000);                        
  setAll(0, 0, 0);                        // set black
  FastLED.setBrightness(BRIGHTNESS);      //max brightness
  nowNumber = 0;
  sound_timer = -9000;
}

void blow() {
  if ((ACC > BLOW) && (ACC < HARD_BLOW)) { // if acceleration enough to play blow;
    sounds.play("BL.wav");               // play blow
    if(DEBUG)
        Serial.println(F("BLOW_")); 
    sound_timer = millis() - SOUND + 560;
    strike_flag = 1;
  }
  if (ACC >= HARD_BLOW) {                    //if acceleration enough to play hard blow;
    sounds.play("SBL.wav");               // play strong blow sound
    if(DEBUG)
        Serial.println(F("BLOWS")); 
    sound_timer = millis() - SOUND + 705;
    strike_flag = 1;
  }
}

void swing(){
  if (GYR > 80 && (millis() - swing_timeout > 100)) {
    swing_timeout = millis();
    if (((millis() - swing_timer) > 500) && !strike_flag && swing_flag) {
      if ((GYR > SWING) && (GYR < STRONG_SWING)) { //if velocity was enough to play swing
        sounds.play("SW.wav");                     // play swing sound
        if(DEBUG)
        Serial.println(F("SWINGS")); 
        sound_timer = millis() - SOUND + 390;
        swing_timer = millis();
        swing_flag=0;
      }
      if (GYR >= STRONG_SWING) {                  // if velocity was enough to play strong swing
        sounds.play("SSW.wav");                   //  play strong swing sound
        if(DEBUG)
        Serial.println(F("SWING_")); 
        sound_timer = millis() - SOUND + 360;
        swing_timer = millis();
        swing_flag=0;
      }
    }
  }
}

void loop(){  
  lightEffect();      
  checkSensorState();
  on_off();             //power on/off the sword

  buttonOp();           // operation with control button
  blow();
  swing();
  
}

void on_off(){                    
  if (cngRequest) {                
    if (!swordState && (voltage() > 10 ))                 //sword is on
    {                 

        sounds.play("On.wav");       // play on sound
        delay(200); 
        lightOn();  
        if (DEBUG) Serial.println(F("ON"));                
        delay(200);  
        soundEnable = 1;
        swordState = 1;          //change state to power on
    } 
    else 
    {                        
      sounds.play("Off.wav");         // play off souns
      delay(300); 
      lightOff();  
      soundEnable = 0;
      if (DEBUG) Serial.println(F("OFF")); 
      delay(300); 
      sounds.disable();                                
      swordState = 0;             
      if (eeprom_flag) 
      {                              // if color was changed
        eeprom_flag = 0;
        EEPROM.write(0, curColor);   // save to the eeprom
      }
    }
    cngRequest = 0;                //reset change state request
  }
  if (soundEnable && ((millis() - sound_timer) >= SOUND) ) {           // play main sound 6 seconds
    //if( DEBUG)
    //Serial.println(F("HUM"));
    //sounds.play("SW.wav");
    sound_timer = millis(); 
    swing_flag = 1;
    strike_flag = 0;                                        
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

    if (!btnState && btn_flag) {     // button was released
    btn_flag = 0;
    hold_flag = 0;           
  }
  
  // button still pressed
  if (btn_flag && btnState && (millis() - btn_timer > BTN_TIMEOUT) && !hold_flag) {
    cngRequest = 1;              // set request to change state of the sword
    hold_flag = 1;
    btn_counter = 0;
  }

  
 // button was pressed more than 1 time
  if ((millis() - btn_timer > BTN_TIMEOUT) && (btn_counter != 0)) {
    if (swordState) {
      if (btn_counter == 3) {               
        curColor++;                         
        if (curColor >= 6) curColor = 0;    
        setColor(curColor);                 
        setAll(red, green, blue);           
        eeprom_flag = 1;    
        lightFlag=0;                
      }
      if (btn_counter == 5) {               
        setRGB();       
        lightFlag=1;            
      }
    }
    btn_counter = 0;
  }
}

void checkSensorState(){
  if (swordState) {                                               
    if (millis() - sensor_timer > 600) {             
                    
      sensor.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);       // every 300ms save checked values

      gyrX = abs(gx / 100);
      gyrY = abs(gy / 100);
      gyrZ = abs(gz / 100);
      accX = abs(ax / 100);
      accY = abs(ay / 100);
      accZ = abs(az / 100);

      
      if(DEBUG){
         Serial.print(F("acc "));
         Serial.print(accX);
         Serial.print(" ");
         Serial.print(accY);
         Serial.print(" ");
         Serial.print(accZ);
         Serial.println(" ");

         Serial.print("gyr ");
         Serial.print(gyrX);
         Serial.print(" ");
         Serial.print(gyrY);
         Serial.print(" ");
         Serial.print(gyrZ);
         Serial.println(" ");
         
    }

      ACC = sq((long)accX) + sq((long)accY) + sq((long)accZ);
      ACC = sqrt(ACC);
      GYR = sq((long)gyrX) + sq((long)gyrY) + sq((long)gyrZ);
      GYR = sqrt((long)GYR);
    if(DEBUG){
         Serial.print("ACC = ");
         Serial.print(ACC);
         Serial.print("GYR = ");
         Serial.println(GYR);
    }

    sensor_timer = micros(); 
  }
}
}

void lightEffect(){
  if (swordState && ((millis() - effect_timer) >= 50) & !lightFlag) {           // show 
    if(nowNumber == NUM_LEDS / 4)
    {
      nowNumber = 0;
      setPixel(NUM_LEDS / 4 - 1 , red, green, blue);                   //set prev pixels to the main color
      setPixel(NUM_LEDS / 4, red, green, blue);
      setPixel((3*NUM_LEDS) / 4 -1, red, green, blue);
      setPixel((3*NUM_LEDS) / 4, red, green, blue);
    }
    
      char i0 = nowNumber;
      char i1 = NUM_LEDS / 2 - nowNumber - 1;
      char i2 = NUM_LEDS / 2 + nowNumber;
      char i3 = NUM_LEDS - nowNumber - 1;

      if(nowNumber){
      setPixel(i0-1, red, green, blue);                              //set prev pixels to the main color
      setPixel(i1+1, red, green, blue);
      setPixel(i2-1, red, green, blue);
      setPixel(i3+1, red, green, blue);
      }
      //set 3 next pixels to an another color  
          setPixel(i0, abs(red - 80), abs(green- 80),abs( blue- 80));
          setPixel(i1, abs(red - 80), abs(green- 80),abs( blue- 80));
          setPixel(i2, abs(red - 80), abs(green- 80),abs( blue- 80));
          setPixel(i3, abs(red - 80), abs(green- 80),abs( blue- 80));
  
    FastLED.show();
    nowNumber++;
  }
}


void setPixel(int index, byte red, byte green, byte blue) {
  leds[index].r = red;
  leds[index].g = green;
  leds[index].b = blue;
}

void setRGB() {
  char i0 = 0;
  char i1 = NUM_LEDS / 2 - 1;
  char i2 = NUM_LEDS / 2;
  char i3 = NUM_LEDS - 1;

  redOffset = 255;
  greenOffset = 0;
  blueOffset = 0;
  
    setPixel(i0++, redOffset, greenOffset, blueOffset);
    setPixel(i1--, redOffset, greenOffset, blueOffset);
    setPixel(i2++, redOffset, greenOffset, blueOffset);
    setPixel(i3--, redOffset, greenOffset, blueOffset);

  redOffset = 205;
  greenOffset = 0;
  blueOffset = 50;

    setPixel(i0++, redOffset, greenOffset, blueOffset);
    setPixel(i1--, redOffset, greenOffset, blueOffset);
    setPixel(i2++, redOffset, greenOffset, blueOffset);
    setPixel(i3--, redOffset, greenOffset, blueOffset);

  redOffset = 255;
  greenOffset = 0;
  blueOffset = 255;

    setPixel(i0++, redOffset, greenOffset, blueOffset);
    setPixel(i1--, redOffset, greenOffset, blueOffset);
    setPixel(i2++, redOffset, greenOffset, blueOffset);
    setPixel(i3--, redOffset, greenOffset, blueOffset);

  redOffset = 0;
  greenOffset = 0;
  blueOffset = 255;

    setPixel(i0++, redOffset, greenOffset, blueOffset);
    setPixel(i1--, redOffset, greenOffset, blueOffset);
    setPixel(i2++, redOffset, greenOffset, blueOffset);
    setPixel(i3--, redOffset, greenOffset, blueOffset);

  redOffset = 100;
  greenOffset = 255;
  blueOffset = 0;

    setPixel(i0, redOffset, greenOffset, blueOffset);
    setPixel(i1, redOffset, greenOffset, blueOffset);
    setPixel(i2, redOffset, greenOffset, blueOffset);
    setPixel(i3, redOffset, greenOffset, blueOffset);
      FastLED.show();
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
    setPixel(i0, 0, 0, 0);
    setPixel(i1, 0, 0, 0);
    setPixel(i2, 0, 0, 0);
    setPixel(i3, 0, 0, 0);
    //setPixel((NUM_LEDS - 1 - i), red, green, blue);
    FastLED.show();
    delay(10);
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
    case 1://pink
      red = 255;
      green = 255;
      blue = 0;
      break;
    case 2://blue
      red = 0;
      green = 255;
      blue = 0;
      break;
    case 3://green
      red = 0;
      green = 0;
      blue = 255;
      break;

    case 4://sky
      red = 0;
      green = 255;
      blue = 255;
      break;
      
    case 5://yellow
      red = 255;
      green = 0;
      blue = 255;
      break;

  }
}


byte voltage() {
  
  float volts = 0;
  float k = 5;    // correlation coefficient to the real current value of the voltage of the batteries
  for (int i = 0; i < 5; i++) {    
    volts += ((float)analogRead(VOLT_PIN)) * k / 1024 * ((R1 + R2) / R2); //get the voltage of 5 in 10ns
  }
       
  int measure = 100 * (volts / (3*5)) ;   //get the voltage*100 for 1 battery

  if (measure > 380)                      //full charge
  return map(measure, 420, 380, 100, 80);
  else if ((measure <= 380) && (measure > 375) )
    return map(measure, 380, 375, 80, 50);
  else if ((measure <= 375) && (measure > 360) )
    return map(measure, 375, 360, 50, 25);
  else if ((measure <= 360) && (measure > 340) )
    return map(measure, 360, 340, 25, 8);
  else                                    //low charge
  return map(measure, 340, 260, 8, 0);
}
