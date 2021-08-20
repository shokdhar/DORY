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
uint16_t dry_sensorValues[AS726x_NUM_CHANNELS];
uint16_t dry_sensorValuesSpecimen[AS726x_NUM_CHANNELS];
uint16_t cf_v, cf_b, sf_v, sf_b; 
uint8_t c_temp, s_temp;
String result;
const int buttonPin = 3;
float margint, ratio;
int first=0;
float stain_margin_percent=4;
float humidity_margin_percent=0.5;


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
    Serial.println("could not connect to sensor! Please check your wiring.");
    while(1);
  }
  
}

void loop() { 
  digitalWrite(A2, LOW);
  digitalWrite(A3, LOW);
//  if(first==0){
    testscrolltext();
    first++;
    waitForButtonPress();
    oled.stopscroll();
//  }


  printOLEDMsg((char *)"Insert", (char *)"Control strip", (char *)"and press button");
  waitForButtonPress();
  dryReadControl();
  printOLEDMsg((char *)"Insert", (char *)"Specimen strip", (char *)"and press button");
  waitForButtonPress();
  dryReadSpecimen();

  printOLEDMsg((char *)"Insert", (char *)"Blown Control strip", (char *)"and press button");
  waitForButtonPress();
  readControl();
  printOLEDMsg((char *)"Insert", (char *)"Blown Specimen strip", (char *)"and press button");
  waitForButtonPress();
  readSpecimen();
  delay(100);
  printOLEDDirect();
  waitForButtonPress();
}

void printOLEDMsg(char *msg, char *msg2, char *msg3){
  oled.setTextSize(2);    // first we have to decide font size
  oled.setTextColor(WHITE);     //we have to decide back ground color
  oled.clearDisplay();
  delay(100);
  oled.setCursor(5,5);  
  oled.print(msg);
  oled.setCursor(5,25);  
  oled.setTextSize(0);
  oled.print(msg2);    
  oled.setCursor(5,40);  
  oled.setTextSize(0);
  oled.print(msg3);    
  oled.display();
}




void readControl(){
  digitalWrite(A1, HIGH);
//  ams.drvOn();
  ams.startMeasurement();
  while(!ams.dataReady()){
    delay(5);
  }
  ams.readRawValues(sensorValues);
  ams.drvOff();
  digitalWrite(A1, LOW);
}


void readSpecimen(){
  digitalWrite(A1, HIGH);
//  ams.drvOn();
  ams.startMeasurement();
  while(!ams.dataReady()){
    delay(5);
  }
  ams.readRawValues(sensorValuesSpecimen);
  ams.drvOff();
  digitalWrite(A1, LOW);

}


void dryReadControl(){
  digitalWrite(A1, HIGH);
//  ams.drvOn();
  ams.startMeasurement();
  while(!ams.dataReady()){
    delay(5);
  }
  ams.readRawValues(dry_sensorValues);
  ams.drvOff();
  digitalWrite(A1, LOW);
}


void dryReadSpecimen(){
  digitalWrite(A1, HIGH);
//  ams.drvOn();
  ams.startMeasurement();
  while(!ams.dataReady()){
    delay(5);
  }
  ams.readRawValues(dry_sensorValuesSpecimen);
  ams.drvOff();
  digitalWrite(A1, LOW);

}


/*
void computeTest(){
//  float margint;
  if(sensorValuesSpecimen[AS726x_VIOLET] > sensorValues[AS726x_VIOLET]){
    margint=(((float)sensorValuesSpecimen[AS726x_VIOLET]-(float)sensorValues[AS726x_VIOLET])*100)/(float)sensorValues[AS726x_VIOLET];
    result="Negative";
    ratio=(float)sensorValuesSpecimen[AS726x_VIOLET]/(float)sensorValues[AS726x_VIOLET];
    digitalWrite(A3, HIGH);
  } else {
    margint=(((float)sensorValues[AS726x_VIOLET] - (float)sensorValuesSpecimen[AS726x_VIOLET])*100)/(float)sensorValues[AS726x_VIOLET];
    ratio=((float)sensorValuesSpecimen[AS726x_VIOLET]/(float)sensorValues[AS726x_VIOLET])-1;
    result="Positive";
    digitalWrite(A2, HIGH);
  }
  if(margint < 20) {
    result="Ambiguous";
    digitalWrite(A3, LOW);
    digitalWrite(A2, LOW);
  }
}

*/
//humidity_margin_percent stain_margin_percent

void computeTest(){
  float v_control=(float)sensorValues[AS726x_VIOLET];
  float v_specimen=(float)sensorValuesSpecimen[AS726x_VIOLET];
  float vb_specimen=(float)sensorValuesSpecimen[AS726x_BLUE];
  float d_control=(float)dry_sensorValues[AS726x_VIOLET];
  float d_specimen=(float)dry_sensorValuesSpecimen[AS726x_VIOLET];
  float db_specimen=(float)dry_sensorValuesSpecimen[AS726x_BLUE];
  float d_variance = (d_control - v_control);
  d_variance = d_variance*100/d_control;
  float vv_diff = (abs(v_specimen-d_specimen)/d_specimen)*100;
  float vb_diff = (abs(vb_specimen-db_specimen)/d_specimen)*100;
  float v_diff = 0.9 * vv_diff + 0.1 * vb_diff;

  Serial.println("v_specimen");
  Serial.println(v_specimen);

  Serial.println("d_specimen");
  Serial.println(d_specimen);

  Serial.println("vb_specimen");
  Serial.println(vb_specimen);
  
  Serial.println("db_specimen");
  Serial.println(db_specimen);
  
  Serial.println("d_control");
  Serial.println(d_control);
  
  Serial.println("v_control");
  Serial.println(v_control);
  
//  Serial.println("d_variance");
//  Serial.println(d_variance);
  
  
  
  Serial.println("v_diff");
  Serial.println(v_diff);
  margint = v_diff;
  float d_diff = (abs(d_control - v_control)/d_control)*100;
//    Serial.println("d_diff");
//  Serial.println(d_diff);

  if(v_diff < humidity_margin_percent){
    result = "INADEQUATE";
    return;
  }
  
  if(d_diff <= (stain_margin_percent * -1)){
        result="Repeat";
        return;                  
  }
  
  if(v_specimen < d_specimen){
    if(v_diff > stain_margin_percent){

        result="Positive";
        digitalWrite(A2, HIGH);     
    } else {
      result="Ambiguous";
       digitalWrite(A3, LOW);
       digitalWrite(A2, LOW);      
    }
  } else {
    Serial.println("Negative");
    result="Negative";
     digitalWrite(A3, HIGH);
  }
    Serial.println("Result");
  Serial.println(result);
    Serial.println("Done Result");


}

/*
void computeTest(){
  float v_control=(float)sensorValues[AS726x_VIOLET];
  float v_specimen=(float)sensorValuesSpecimen[AS726x_VIOLET];
  float d_control=(float)dry_sensorValues[AS726x_VIOLET];
  float d_specimen=(float)dry_sensorValuesSpecimen[AS726x_VIOLET];
  float d_variance = abs(d_control - v_control)/d_control;
  float v_diff = (abs(v_specimen-d_specimen)/d_specimen)*100;
   Serial.println("control dry");
  Serial.println(dry_sensorValues[AS726x_VIOLET]);

  Serial.println("specimen dry");
  Serial.println(dry_sensorValuesSpecimen[AS726x_VIOLET]);


  Serial.println("control ");
  Serial.println(sensorValues[AS726x_VIOLET]);

  Serial.println("specimen ");
  Serial.println(sensorValuesSpecimen[AS726x_VIOLET]);




  margint = v_diff;
  float d_diff = (abs(d_control - v_control)/d_control)*100;

  Serial.println("v_diff");
  Serial.println(v_diff);
  Serial.println("d_diff");
  Serial.println(d_diff);

 
 
  if(v_diff < humidity_margin_percent){
    result = "INADEQUATE";
    return;
  }
  if(v_specimen < d_specimen){
    if(v_diff > stain_margin_percent){
      result="Positive";
      digitalWrite(A2, HIGH);    
    } else {
      result="Ambiguous";
       digitalWrite(A3, LOW);
       digitalWrite(A2, LOW);      
    }
  } else {
    result="Negative";
     digitalWrite(A3, HIGH);
  }

  Serial.println(result);

 
}
*/

void waitForButtonPress(){
  while(digitalRead(buttonPin)==HIGH)
  {
    delay(10);
  }
  // buffer the input read
  delay(300);
  return;
}

void printOLEDDirect(){
  int i=5;
  oled.clearDisplay();
  computeTest();
  oled.setCursor(5,i);
  oled.setTextSize(2);
  oled.print( result);  
  i+=20;
  oled.setCursor(5,i);
//  oled.setTextSize(0);    // first we have to decide font size
//  oled.print( "Confidence: " + String(margint) + "%" );  
  delay(100);
//  i+=15;
//  oled.setCursor(5,i); i+=10; 
//  oled.print("V: "+ String(sensorValues[AS726x_VIOLET]) + "->"+String(sensorValuesSpecimen[AS726x_VIOLET]));
//  oled.setCursor(5,i);
//  oled.print("B: "+ String(sensorValues[AS726x_BLUE]) + "->"+String(sensorValuesSpecimen[AS726x_BLUE]));
//  oled.setCursor(5,i); i+=8;
//  oled.print("G: "+ String(sensorValues[AS726x_GREEN]) + "->"+String(sensorValuesSpecimen[AS726x_GREEN]));
//  oled.setCursor(5,i); i+=8;
//  oled.print(" Y: "+ String(sensorValues[AS726x_YELLOW]) + "->"+String(sensorValuesSpecimen[AS726x_YELLOW]));
//  oled.setCursor(5,i); i+=8;
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
