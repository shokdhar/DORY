#include <Wire.h>
#include "Adafruit_AS726x.h"
#include<Adafruit_GFX.h>   //  include Graphical Library
#include<Adafruit_SSD1306.h>  // Oled library

const boolean ENABLE_SERIAL=true;
const boolean ENABLE_OLED=false;

//create the object
Adafruit_AS726x ams;
Adafruit_SSD1306 oled(128, 64, &Wire, 4);
//buffer to hold raw values
uint16_t sensorValues[AS726x_NUM_CHANNELS];
uint16_t sensorValuesSpecimen[AS726x_NUM_CHANNELS];
uint16_t dry_sensorValues[AS726x_NUM_CHANNELS];
uint16_t dry_sensorValuesSpecimen[AS726x_NUM_CHANNELS];
uint16_t cf_v, cf_b, sf_v, sf_b; 
char* result;
const int buttonPin = 3;
double margint;
int first=0;
float stain_margin_percent=4;
float humidity_margin_percent=1.2;


void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  Serial.begin(9600);
  if(ENABLE_OLED){
    oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // SSD1306_SWITCHCAPPVCC is generate 3.3volt internaly,  0x3c is I2c slave(oled) address
    oled.clearDisplay();  // Celar display function
  }
  while(!Serial);
  pinMode(A3, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A1, OUTPUT);
  
  delay(1000);
    
  if(!ams.begin()){
    Serial.println("could not connect to sensor! Please check your wiring.");
    while(1);
  }
  
}

void loop() { 
  p("ready");
  digitalWrite(A2, LOW);
  digitalWrite(A3, LOW);
//   testscrolltext();
//   waitForButtonPress();
//   oled.stopscroll();

  if(first==0){
    printOLEDMsg("Starting", "Calibration", " press button");
    waitForButtonPress();
    printOLEDMsg("Insert", "Dry Control", "and press button");
    waitForButtonPress();
    readSpectrometer(dry_sensorValues);
    if(dry_sensorValues[AS726x_VIOLET] < 100){
      printOLEDMsg("Error", "No Strip", "");
      delay(5000);
      return;
    }
  
    printOLEDMsg("Insert", "Dry Test", "and press button");
    waitForButtonPress();
    readSpectrometer(dry_sensorValuesSpecimen);
    if(dry_sensorValuesSpecimen[AS726x_VIOLET] < 100){
      printOLEDMsg("Error", "No Strip", "");
      delay(5000);
      return;
    }
    first++;
    
  }

  printOLEDMsg("Insert", "Sampled Control", "and press button");
  waitForButtonPress();
  readSpectrometer(sensorValues);
  if(sensorValues[AS726x_VIOLET] < 100){
    printOLEDMsg("Error", "No Strip", "");
    delay(5000);
    return;
  }

  
  
  printOLEDMsg("Insert", "Sampled Test", "and press button");
  waitForButtonPress();
  readSpectrometer(sensorValuesSpecimen);
  if(sensorValuesSpecimen[AS726x_VIOLET] < 100){
    printOLEDMsg("Error", "No Strip", "");
    delay(5000);
    return;
  }

  
  delay(100);
  computeTest();
  delay(100);
  printOLEDDirect();
  delay(1000);
  p("done...waiting for button press");
  waitForButtonPress();
}

void readSpectrometer(uint16_t ref[]){
  digitalWrite(A1, HIGH);
//  ams.drvOn();
  ams.startMeasurement();
  while(!ams.dataReady()){
    delay(5);
  }
  ams.readRawValues(ref);
  ams.drvOff();
  digitalWrite(A1, LOW);
  ps(String(ref[AS726x_VIOLET]));
}





//humidity_margin_percent stain_margin_percent
void computeTest(){
  uint16_t v_control=sensorValues[AS726x_VIOLET];
  uint16_t v_specimen=sensorValuesSpecimen[AS726x_VIOLET];
  uint16_t d_control=dry_sensorValues[AS726x_VIOLET];
  uint16_t d_specimen=dry_sensorValuesSpecimen[AS726x_VIOLET];
  int16_t specimen_diff = v_specimen-d_specimen;
  int16_t v_diff = (specimen_diff*100.0)/(d_specimen*1.0);
  uint16_t d_diff =  abs(d_control - v_control)*100.0/d_control;

  delay(1000);
  margint = v_diff;
  
  ps(String(d_control));
  ps(String(d_specimen));
  ps(String(v_control));
  ps(String(v_specimen));
  p("d_diff");
  ps(String(d_diff));
  p("v_diff");
  ps(String(v_diff));    
  if(abs(v_diff) < humidity_margin_percent){
    result = "REPEAT TEST";
    p("inadequate");
    return;
  }
  if(d_diff < humidity_margin_percent*-1){
    p("repeat");
    result = "REPEAT TEST";
    return;
  }
  if(sensorValuesSpecimen[AS726x_VIOLET] < dry_sensorValuesSpecimen[AS726x_VIOLET]){
        p("less");

    if(abs(v_diff) > stain_margin_percent){
          p("positive");

      result="Positive";
      digitalWrite(A3, HIGH);     
    } else {
          p("abmiguous");

      result="Ambiguous";
       digitalWrite(A3, LOW);
       digitalWrite(A2, LOW);      
    }
  } else {
        p("negative");

    result="Negative";
     digitalWrite(A2, HIGH);
  }
}


void waitForButtonPress(){
  while(digitalRead(buttonPin)==HIGH){
    delay(10);
  }
  // buffer the input read
  delay(300);
  return;
}

void printOLEDDirect(){
  Serial.print(result);
  if(!ENABLE_OLED) return;
  oled.clearDisplay();
  oled.setCursor(5,5);
  oled.setTextSize(2);
  oled.print( result);  
  delay(100);
  oled.display();
}

void testscrolltext(void) {
  oled.clearDisplay();

  oled.setTextSize(3); // Draw 2X-scale text
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(10, 10);
  oled.print(F("NEUOME"));
  oled.display();      // Show initial text
  delay(100);
  oled.startscrollleft(0x00, 0x0F);
}

void printOLEDMsg(char *msg, char *msg2, char *msg3){
  p(msg);
  p(msg2);
  p(msg3);

   oled.setTextSize(0);    // first we have to decide font size
   oled.setTextColor(WHITE);     //we have to decide back ground color
   oled.clearDisplay();
   delay(100);
   oled.setCursor(5,5);  
   oled.print(msg);
   oled.setCursor(5,18);  
   oled.setTextSize(2);
   oled.print(msg2);    
   oled.setCursor(5,50);  
   oled.setTextSize(0);
   oled.print(msg3);    
   oled.display();
}


  
void p(char *s){
  if(!ENABLE_SERIAL) return;
  Serial.println(s);
  Serial.flush();
}

void ps(String s){
  if(!ENABLE_SERIAL) return;
  Serial.println(s);
  Serial.flush();
}
