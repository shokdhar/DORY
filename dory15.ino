
#include <Wire.h>
#include "Adafruit_AS726x.h"
#include<Adafruit_GFX.h>   //  include Graphical Library
#include<Adafruit_SSD1306.h>  // Oled library



//create the object
Adafruit_AS726x ams;
Adafruit_SSD1306 oled(128, 64, &Wire, 4);
//buffer to hold raw values
uint16_t sensorValues[AS726x_NUM_CHANNELS];
uint16_t sensorValuesSpecimen[AS726x_NUM_CHANNELS];
// int c_r, s_r,c_y, s_y,c_o, s_o,c_g, s_g,c_b, s_b,c_v, s_v;
uint16_t cf_v, cf_b, sf_v, sf_b; 
uint8_t c_temp, s_temp;
String result;
const int buttonPin = 3;
float margint;
const int pwm = A1; 

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  Serial.begin(9600);
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // SSD1306_SWITCHCAPPVCC is generate 3.3volt internaly,  0x3c is I2c slave(oled) address
  oled.clearDisplay();  // Celar display function
  while(!Serial);
  pinMode(A3, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(pwm, OUTPUT);
  
  delay(1000);
    
  if(!ams.begin()){
    Serial.println("could not connect to sensor! Please check your wiring.");
    while(1);
  }
  
}

void loop() {  
   digitalWrite(A1, LOW);
   digitalWrite(A2, LOW);
   digitalWrite(A3, LOW);
   
  printOLEDMsg((char *)"          Insert the  Control");
  waitForButtonPress();
  readControl();
  cooloff();
  printOLEDMsg((char *)"          Insert the Specimen");
  waitForButtonPress();
  readSpecimen();
  delay(100);
  printOLEDDirect();
  waitForButtonPress();
}

void printOLEDMsg(char *msg){
  oled.setTextSize(2);    // first we have to decide font size
  oled.setTextColor(WHITE);     //we have to decide back ground color
  oled.clearDisplay();
  oled.invertDisplay(true);
  delay(100);
  oled.setCursor(5,5);  
  oled.print(msg);
  oled.display();
}

void readControl()
{
  digitalWrite(pwm,25);
  ams.startMeasurement();
  while(!ams.dataReady())
  {
    delay(5);
  }
  ams.readRawValues(sensorValues);  
  digitalWrite(A1, LOW);
}


void readSpecimen(){
  digitalWrite(pwm,25);
  ams.startMeasurement();
  while(!ams.dataReady())
  {
    delay(5);
  }
  ams.readRawValues(sensorValuesSpecimen);
 
  digitalWrite(pwm,25);

}

void computeTest(){
//  float margint;
  if(sensorValuesSpecimen[AS726x_VIOLET] < sensorValues[AS726x_VIOLET]){
    margint=(((float)sensorValuesSpecimen[AS726x_VIOLET]-(float)sensorValuesSpecimen[AS726x_VIOLET])*100)/(float)sensorValues[AS726x_VIOLET];
    result="+ve";
  } else {
    margint=(((float)sensorValues[AS726x_VIOLET] - (float)sensorValuesSpecimen[AS726x_VIOLET])*100)/(float)sensorValues[AS726x_VIOLET];
    result="-ve";
  }
}

void cooloff(){
  printOLEDMsg((char *)"Processing.....", (char *) "", (char *) "pls wait");
  delay(5000);
 
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

void waitForButtonPress()

{
 // Serial.println("Before loop");
  while(digitalRead(buttonPin)==HIGH)
  {
   // Serial.println(digitalRead(buttonPin));
    delay(10);
  }
 //Serial.println("After loop");
  // buffer the input read
  delay(300);
  return;
}

void printOLEDDirect(){
  int i=5;
  oled.setTextSize(0);    // first we have to decide font size
  oled.clearDisplay();
  delay(100);
 
  oled.setCursor(5,i); i+=8;
  oled.setCursor(5,i); i+=8;
  oled.print("V: "+ String(sensorValues[AS726x_VIOLET]) + "->"+String(sensorValuesSpecimen[AS726x_VIOLET]));
  Serial.print("Control  =  ");
  Serial.println(sensorValues[AS726x_VIOLET]);
  Serial.print("Test     =  ");
   Serial.println(sensorValuesSpecimen[AS726x_VIOLET]);
 /* oled.setCursor(5,i); i+=8;
  oled.print("B: "+ String(sensorValues[AS726x_BLUE]) + "->"+String(sensorValuesSpecimen[AS726x_BLUE]));
  oled.setCursor(5,i); i+=8;
  oled.print("G: "+ String(sensorValues[AS726x_GREEN]) + "->"+String(sensorValuesSpecimen[AS726x_GREEN]));
  oled.setCursor(5,i); i+=8;
  oled.print("Y: "+ String(sensorValues[AS726x_YELLOW]) + "->"+String(sensorValuesSpecimen[AS726x_YELLOW]));
  oled.setCursor(5,i); i+=8;
   oled.print("O: "+ String(sensorValues[AS726x_ORANGE]) + "->"+String(sensorValuesSpecimen[AS726x_ORANGE]));
   oled.setCursor(5,i); i+=8;
   oled.print("R: "+ String(sensorValues[AS726x_RED]) + "->"+String(sensorValuesSpecimen[AS726x_RED]));
      oled.setCursor(5,i); i+=8;
  */
 //   oled.print(temp);
  oled.setCursor(5,i); i+=8;
  oled.setCursor(5,i); i+=8; 
 computeTest();
 // oled.print("RESULT     =  "+ result);  
  oled.setCursor(5,i); i+=8;
 // oled.print("Percentage =  ");
 // oled.print(margint*-1);
  //oled.print("%");
  oled.display();

}
