#include <Wire.h>
#include "Adafruit_AS726x.h"
#include<Adafruit_GFX.h>   //  include Graphical Library
#include<Adafruit_SSD1306.h>  // Oled library

const boolean ENABLE_SERIAL=true;
const boolean ENABLE_OLED=true;

//create the object
Adafruit_AS726x ams;
Adafruit_SSD1306 oled(128, 64, &Wire, 4);
//buffer to hold raw values
uint16_t sensorValues[AS726x_NUM_CHANNELS];
uint16_t sensorValuesSpecimen[AS726x_NUM_CHANNELS];
uint16_t dry_sensorValues[AS726x_NUM_CHANNELS];
uint16_t dry_sensorValuesSpecimen[AS726x_NUM_CHANNELS];
uint16_t cf_v, cf_b, sf_v, sf_b; 
String result;
const int buttonPin = 3;
double margint;
int first=0;
float stain_margin_percent=4;
float humidity_margin_percent=0.8;
uint16_t min_reading_threshold=40;


void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  Serial.begin(9600);
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // SSD1306_SWITCHCAPPVCC is generate 3.3volt internaly,  0x3c is I2c slave(oled) address
  oled.clearDisplay();  // Celar display function
  while(!Serial);
  pinMode(A3, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A1, OUTPUT);
  
  delay(1000);
    
  if(!ams.begin()){
    // Serial.println("could not connect to sensor! Please check your wiring.");
    while(1);
  }
  
}

void loop() { 
   Serial.println("ready");
  digitalWrite(A2, LOW);
  digitalWrite(A3, LOW);
  testscrolltext();
  waitForButtonPress();
  oled.stopscroll();

//  if(first==0){
//    printOLEDMsg((char *)"Starting", (char *) "Calibration",  (char *)" press button");
//    waitForButtonPress();
    printOLEDMsg((char *)"Insert", (char *) "Dry Control", (char *) "and press button");
    waitForButtonPress();
    readSpectrometer(dry_sensorValues);
    if(dry_sensorValues[AS726x_VIOLET] < min_reading_threshold){
      printOLEDMsg((char *)"Error", (char *) "No Strip", (char *) "");
      delay(5000);
      return;
    }
  
    printOLEDMsg((char *)"Insert", (char *) "Dry Test", (char *) "and press button");
    waitForButtonPress();
    readSpectrometer(dry_sensorValuesSpecimen);
    if(dry_sensorValuesSpecimen[AS726x_VIOLET] < min_reading_threshold){
      printOLEDMsg((char *)"Error", (char *) "No Strip", (char *) "");
      delay(5000);
      return;
    }
    first++;
    
//  }

  printOLEDMsg((char *)"Insert", (char *) "Sampled Control", (char *) "and press button");
  waitForButtonPress();
  readSpectrometer(sensorValues);
  if(sensorValues[AS726x_VIOLET] < min_reading_threshold){
    printOLEDMsg((char *)"Error", (char *) "No Strip", (char *) "");
    delay(5000);
    return;
  }

  
  
  printOLEDMsg((char *)"Insert", (char *) "Sampled Test", (char *) "and press button");
  waitForButtonPress();
  readSpectrometer(sensorValuesSpecimen);
  if(sensorValuesSpecimen[AS726x_VIOLET] < min_reading_threshold){
    printOLEDMsg((char *)"Error", (char *) "No Strip", (char *) "");
    delay(5000);
    return;
  }

  
  delay(100);
  computeTest();
  delay(100);
  printOLEDDirect();
  delay(100);
  waitForButtonPress();
  
  printOLEDResultsDirect();
  delay(1000);
  // Serial.println("done...waiting for button press");
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
  Serial.println(ref[AS726x_VIOLET]);
}





//humidity_margin_percent stain_margin_percent
void computeTest(){
  uint16_t v_control=sensorValues[AS726x_VIOLET];
  uint16_t v_specimen=sensorValuesSpecimen[AS726x_VIOLET];
  uint16_t d_control=dry_sensorValues[AS726x_VIOLET];
  uint16_t d_specimen=dry_sensorValuesSpecimen[AS726x_VIOLET];
  int16_t specimen_diff = v_specimen-d_specimen;
  int16_t v_diff = (specimen_diff*1000.0)/(d_specimen*1.0);
  int16_t dry_diff = d_control - v_control;
  int16_t d_diff =  (dry_diff*1000.0)/d_control;
  int16_t v_diff2 = abs(v_diff );
  int16_t d_diff2 = abs(d_diff );
  delay(1000);
  margint = v_diff;
  
   Serial.println(d_control);
   Serial.println(d_specimen);
   Serial.println(v_control);
   Serial.println(v_specimen);
//   Serial.println("d_diff");
//   Serial.println(d_diff);
//   Serial.println("v_diff");
//   Serial.println(v_diff);   
   Serial.println("v_diff2");
   Serial.println(v_diff2);
   Serial.println("d_diff2");
   Serial.println(d_diff2);
//  if(v_diff2 < humidity_margin_percent){
//    result = String("REPEAT TEST..");
//    // Serial.println("inadequate");
//    return;
//  }
  if(d_diff2 < humidity_margin_percent*10){
    // Serial.println("repeat");
    result = String("REPEAT TEST");
    return;
  }
  if(sensorValuesSpecimen[AS726x_VIOLET] < dry_sensorValuesSpecimen[AS726x_VIOLET]){
        // Serial.println("less");

    if(abs(v_diff) > stain_margin_percent*10){
          // Serial.println("positive");

      result=String("Positive");
      digitalWrite(A2, HIGH);     
    } else {
          // Serial.println("abmiguous");

      result=String("Ambiguous");
       digitalWrite(A3, LOW);
       digitalWrite(A2, LOW);      
    }
  } else {
        // Serial.println("negative");

    result=String("Negative");
     digitalWrite(A3, HIGH);
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
  // Serial.print(result);
  if(!ENABLE_OLED) return;
  oled.clearDisplay();
  oled.setCursor(5,5);
  oled.setTextSize(2);
  oled.print( result.c_str());  
  delay(100);
  oled.display();
}

void printOLEDResultsDirect(){
  // Serial.print(result);
  if(!ENABLE_OLED) return;
  oled.clearDisplay();
  oled.setTextSize(2);

  oled.setCursor(5,5);
//  oled.print("DC:");  
//  oled.setCursor(25,5);
  oled.print(dry_sensorValues[AS726x_VIOLET]);  

//  oled.setCursor(55,5);
//  oled.print("DT:");  
  oled.setCursor(75,5);
  oled.print(dry_sensorValuesSpecimen[AS726x_VIOLET]);  

  oled.setCursor(5,25);
//  oled.print("SC:");  
//  oled.setCursor(25,25);
  oled.print(sensorValues[AS726x_VIOLET]);  

//  oled.setCursor(55,25);
//  oled.print("ST:");  
  oled.setCursor(75,25);
  oled.print(sensorValuesSpecimen[AS726x_VIOLET]);  

  oled.setTextSize(1);

  oled.setCursor(5,43);
  oled.print(dry_sensorValues[AS726x_RED]);  
  oled.setCursor(35,43);
  oled.print(dry_sensorValuesSpecimen[AS726x_RED]);  
  oled.setCursor(65,43);
  oled.print(sensorValues[AS726x_RED]);  
  oled.setCursor(95,43);
  oled.print(sensorValuesSpecimen[AS726x_RED]);  
 
  oled.setCursor(5,55);
  oled.print(result);  

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
  Serial.println(msg);
  Serial.println(msg2);
  Serial.println(msg3);

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
