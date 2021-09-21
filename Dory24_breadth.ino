//---------------------------------------------------------- 
#include <Wire.h>
#include "Adafruit_AS726x.h"
#include<Adafruit_GFX.h>   //  include Graphical Library
#include<Adafruit_SSD1306.h>  // Oled library
//---------------------------------------------------------- 
const boolean ENABLE_SERIAL=true;
const boolean ENABLE_OLED=true;

//create the object
Adafruit_AS726x ams;
Adafruit_SSD1306 oled(128, 64, &Wire, 4);
//buffer to hold raw values
//---------------------------------------------------------- 
uint16_t sensorValues[AS726x_NUM_CHANNELS];
uint16_t sensorValuesSpecimen[AS726x_NUM_CHANNELS];
uint16_t dry_sensorValues[AS726x_NUM_CHANNELS];
uint16_t dry_sensorValuesSpecimen[AS726x_NUM_CHANNELS];
uint16_t cf_v, cf_b, sf_v, sf_b; 
String result;
const int buttonPin = 3;
double margint;
uint8_t board_temp;
int first=0;
//---------------------------------------------------------- 

//////////// SETTINGS ////////////////////
float stain_margin_percent=2;
float max_control_change_percent=4;
int16_t max_negative_stain_margin_percent=20;


uint16_t min_reading_threshold=40;
//////////// END SETTINGS ////////////////


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
  //---------------------------------------------------------- 
    result = '\0';
   Serial.println("ready");
  digitalWrite(A2, LOW);
  digitalWrite(A3, LOW);
  board_temp = ams.readTemperature();
  Serial.println("temp");
  Serial.println(board_temp);
  testscrolltext();
  waitForButtonPress();
  oled.stopscroll();
//---------------------------------------------------------- 
    printOLEDMsg((char *)"Insert", (char *) "Dry       Control", (char *) "and press button");
    waitForButtonPress();
    readSpectrometer(dry_sensorValues);
    if(dry_sensorValues[AS726x_VIOLET] < min_reading_threshold){
      printOLEDMsg((char *)"Error", (char *) "No Strip", (char *) "");
      delay(5000);
      return;
    }
    //---------------------------------------------------------- 
    cooloff();
    //---------------------------------------------------------- 
    printOLEDMsg((char *)"Insert", (char *) "Dry Test", (char *) "and press button");
    waitForButtonPress();
    readSpectrometer(dry_sensorValuesSpecimen);
    if(dry_sensorValuesSpecimen[AS726x_VIOLET] < min_reading_threshold){
      printOLEDMsg((char *)"Error", (char *) "No Strip", (char *) "");
      delay(5000);
      return;
    }
    first++;
    //---------------------------------------------------------- 
    cooloff();
    //---------------------------------------------------------- 
  printOLEDMsg((char *)"Insert", (char *) "Sampled    Control", (char *) "and press button");
  waitForButtonPress();
  readSpectrometer(sensorValues);
  if(sensorValues[AS726x_VIOLET] < min_reading_threshold){
    printOLEDMsg((char *)"Error", (char *) "No Strip", (char *) "");
    delay(5000);
    return;
  }
  //---------------------------------------------------------- 
    cooloff();
  //---------------------------------------------------------- 
   
  printOLEDMsg((char *)"Insert", (char *) "Sampled    Test", (char *) "and press button");
  //---------------------------------------------------------- 
  waitForButtonPress();
  //---------------------------------------------------------- 
  readSpectrometer(sensorValuesSpecimen);
  if(sensorValuesSpecimen[AS726x_VIOLET] < min_reading_threshold){
    printOLEDMsg((char *)"Error", (char *) "No Strip", (char *) "");
    delay(5000);
    return;
  }

  
  delay(100);
  computeTest();
  delay(100);
 
  delay(100);
  //---------------------------------------------------------- 
  waitForButtonPress();
  //---------------------------------------------------------- 

  //---------------------------------------------------------- 
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
  Serial.print("reading=");
  Serial.println(ref[AS726x_VIOLET]);
}





void computeTest(){
  //---------------------------------------------------------- 
  int16_t specimen_diff = dry_sensorValuesSpecimen[AS726x_VIOLET]-sensorValuesSpecimen[AS726x_VIOLET];
  int16_t test_variance = (specimen_diff*1000.0)/(dry_sensorValuesSpecimen[AS726x_VIOLET]*1.0);
  int16_t dry_diff = dry_sensorValues[AS726x_VIOLET] - sensorValues[AS726x_VIOLET];
  int16_t control_variance =  (dry_diff*1000.0)/dry_sensorValues[AS726x_VIOLET]*1.0;
  //---------------------------------------------------------- 
  delay(100);
  margint = test_variance;
  
  Serial.println(dry_sensorValues[AS726x_VIOLET]);
  Serial.println(dry_sensorValuesSpecimen[AS726x_VIOLET]);
  Serial.println(sensorValues[AS726x_VIOLET]);
  Serial.println(sensorValuesSpecimen[AS726x_VIOLET]);
  Serial.println("control_variance");
  Serial.println(control_variance);
  Serial.println("test_variance");
  Serial.println(test_variance);   

//---------------------------------------------------------- 

  // in case control increases and is more than 1 percent
  if(control_variance < -10)
  {
    if(sensorValues[AS726x_VIOLET]>dry_sensorValues[AS726x_VIOLET])
{ result = String("REPEAT TEST (pnb)");
      oled.clearDisplay();
  oled.setCursor(5,5);
  oled.setTextSize(2);
  oled.print("REPEAT TEST ");  
  Serial.print("REPEAT TEST "); 
  delay(100);
  oled.display();
    return;
 
  }

 if(sensorValuesSpecimen[AS726x_VIOLET]>dry_sensorValuesSpecimen[AS726x_VIOLET])
{
  result = String("REPEAT TEST (pnb)");
      oled.clearDisplay();
  oled.setCursor(5,5);
  oled.setTextSize(2);
  oled.print("REPEAT TEST ");  
  Serial.print("REPEAT TEST "); 
  delay(100);
  oled.display();
    return;

  }


  }
//---------------------------------------------------------- 
  if(abs(control_variance) > max_control_change_percent*10){
    // Serial.println("repeat");
    result = String("REPEAT TEST (nb)");
      oled.clearDisplay();
  oled.setCursor(5,5);
  oled.setTextSize(2);
  oled.print("REPEAT TEST ");  
  Serial.print("REPEAT TEST "); 
  delay(100);
  oled.display();
    return;
  }
  //---------------------------------------------------------- 

  if(sensorValuesSpecimen[AS726x_VIOLET] < dry_sensorValuesSpecimen[AS726x_VIOLET]*0.99)
  {
    if(abs(test_variance) >= stain_margin_percent*10)
    {
      result=String("Positive");
        oled.clearDisplay();
  oled.setCursor(5,5);
  oled.setTextSize(2);
  oled.print("Positive");  
  Serial.print("Positive"); 
  delay(100);
  oled.display();
      digitalWrite(A3, HIGH);     
    } else 
    {
      result=String("Ambiguous");
      
        oled.clearDisplay();
  oled.setCursor(5,5);
  oled.setTextSize(2);
  oled.print("Ambiguous");  
  Serial.print("Ambiguous"); 
  delay(100);
  oled.display();
      
       digitalWrite(A3, LOW);
       digitalWrite(A2, LOW);      
    }
  } 
  else 
  {
    if(abs(test_variance) >= max_negative_stain_margin_percent*10)
    {
      result = String("REPEAT TEST (t inc)");   
   oled.clearDisplay();
  oled.setCursor(5,5);
  oled.setTextSize(2);
   oled.print("REPEAT TEST ");  
  Serial.print("REPEAT TEST "); 
  delay(100);
  oled.display();
    }
    else 
    {
     // result=String("Negative");
        oled.clearDisplay();
  oled.setCursor(5,5);
  oled.setTextSize(2);
  oled.print("Negative");  
  Serial.print("Negative");
  delay(100);
  oled.display();
       digitalWrite(A2, HIGH);
      
    }
  }
}

void waitForButtonPress(){
  int i=0;
  while(digitalRead(buttonPin)==HIGH){
    delay(10);
  }
  while(digitalRead(buttonPin)==LOW){
    delay(100);
    i++;
  }
  delay(300);
  return;
}


void printOLEDResultsDirect(){
  // Serial.print(result);
  if(!ENABLE_OLED) return;
  oled.clearDisplay();
  oled.setTextSize(2);

  oled.setCursor(5,5);
  oled.print(dry_sensorValues[AS726x_VIOLET]);  

  oled.setCursor(75,5);
  oled.print(dry_sensorValuesSpecimen[AS726x_VIOLET]);  

  oled.setCursor(5,25);
  oled.print(sensorValues[AS726x_VIOLET]);  

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
//---------------------------------------------------------- 
void cooloff()
{
  printOLEDMsg((char *)"Processing.....", (char *) "", (char *) "pls wait");
  delay(5000);
  uint8_t t = ams.readTemperature();
  Serial.println("temp");
    Serial.println(t);
  
  while(ams.readTemperature() > board_temp+3)
  {
    delay(1000);
  }
 
}
//---------------------------------------------------------- 
