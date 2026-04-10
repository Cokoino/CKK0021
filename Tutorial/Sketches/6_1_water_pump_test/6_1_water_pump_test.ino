/*****************************************************
 
 * This code applies to Cokoino Arduino Timer Alarm & Watering DIY Kit
 * Through this link you can download the source code:
 * https://github.com/Cokoino/CKK0021
 * Company web site:
 * http://www.cokoino.com/
 
*****************************************************/
#define RELAY 9   //Connect the signal pin of the Relay module to the D10 pin of the Arduino UNO

void setup() {
  pinMode(RELAY,OUTPUT); //Define siganl pin D10 of Arduino UNO as output mode
}
void loop() {
  digitalWrite(RELAY,HIGH);//Relay closed, The water pump starts working, start pumping water
  delay(5000);//pumping water 5 seconds
  digitalWrite(RELAY,LOW);//The water pump stops working
  delay(5000);//The water pump stops working 5 seconds
}