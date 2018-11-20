#define LED_PIN 6           // led din pin
#define BTN 3               // button pin
#define BTN_LED 4           // control button led
#define SD_CARD_GND A0      // sd card ground    
#define VOLT_PIN A6
#define R1 100000             
#define R2 51000            
#include "Light.h"      //work with ledstrip
#include "AccelGyro.h"    //work with mpu6050
#include "Sound.h"      //sounds
#include "Wire.h"           
#include "I2Cdev.h"         
#include <EEPROM.h>         // memory
#include <toneAC.h>         // generate sound

boolean cngRequest, swordState;         //change state request, sword state 
boolean btnState, btn_flag, hold_flag;  //button state
boolean tone_flag;            //generation flag
byte btn_counter;                       //count of pressing
unsigned long btn_timer;                //time of button pressing
unsigned long toneTimer;                //time of sound generation
boolean eeprom_flag;

Light lights;
Sound sounds;
AccelGyro sensor;

void setup() {
  Wire.begin();
  //set pins
  pinMode(BTN, INPUT_PULLUP);
  pinMode(BTN_LED, OUTPUT);
  pinMode(SD_CARD_GND, OUTPUT);
  digitalWrite(SD_CARD_GND, 0);
  digitalWrite(BTN_LED, 1);

  if ((EEPROM.read(0) >= 0) && (EEPROM.read(0) <= 5)) {  
                     // if sword was activated
    lights.curColor = EEPROM.read(0); // reset prev color
  }
  else {
    EEPROM.write(0, 0);         // set 0 color
    lights.curColor = 0;
  }
  swordState = 0;
  lights.setColor(lights.curColor);
  lights.showVoltage(voltage());      //show voltage
  delay(1000);
  lights.setAll(0, 0, 0);         // set black
  lights.setMaxBrightness();
  lights.nowNumber = 0;
}

void loop() {
  lights.lightEffect(swordState);
  sensor.checkSensorState();
  on_off();               //power on/off the sword
  buttonOp();               // operation with control button
  sounds.blow(sensor.ACC);
  sounds.swing(sensor.GYR);
}

void buttonOp() {

  btnState = !digitalRead(BTN);    // true if button was pressed
  if (btnState && !btn_flag) {
    btn_flag = 1;
    btn_counter++;               // increase count of pressing
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
        lights.curColor++;
        if (lights.curColor >= 6) lights.curColor = 0;
        lights.setColor(lights.curColor);
        lights.setAll(lights.red, lights.green, lights.blue);
        eeprom_flag = 1;
        lights.lightFlag = 0;
      }
      if (btn_counter == 5) {
        lights.setRGB();
        lights.lightFlag = 1;
      }
    }
    btn_counter = 0;
  }
}

void on_off() {
  if (cngRequest) {
    if (!swordState && (voltage() > 10))//sword is on
    {

      sounds.On();          //play on sounds
      delay(200);
      lights.lightOn();
      delay(200);
      sounds.soundEnable = 1;
      swordState = 1;         //change state to power on
    }
    else
    {
      sounds.Off();         // play off souns
      delay(300);
      lights.lightOff();
      sounds.soundEnable = 0;
      delay(300);
      sounds.Mute();
      swordState = 0;
      if (eeprom_flag)
      {                              // if color was changed
        eeprom_flag = 0;
        // save to the eeprom
        EEPROM.write(0, lights.curColor);   
      }
    }
    cngRequest = 0;           //reset change state request
  }
  long delta = millis() - toneTimer;
  if ((delta > 3) && tone_flag) {     // generate main sound
    if (strike_flag) {
      tmrpcm.disable();             
      strike_flag = 0;
    }
    toneAC(freq_f);                   
    toneTimer = millis();                          
  }
}

byte voltage() {

  float volts = 0;
  float k = 5;              // correlation coefficient to the real current value of the voltage of the batteries
  for (int i = 0; i < 5; i++) {
    volts += ((float)analogRead(VOLT_PIN)) * k / 1024 * ((R1 + R2) / R2); //get the voltage of 5 in 10ns
  }

  int measure = 100 * (volts / (3 * 5));   //get the voltage*100 for 1 battery

  if (measure > 380)                      //full charge
    return map(measure, 420, 380, 100, 80);
  else if ((measure <= 380) && (measure > 375))
    return map(measure, 380, 375, 80, 50);
  else if ((measure <= 375) && (measure > 360))
    return map(measure, 375, 360, 50, 25);
  else if ((measure <= 360) && (measure > 340))
    return map(measure, 360, 340, 25, 8);
  else                                    //low charge
    return map(measure, 340, 260, 8, 0);
}
