/*****************************************************
 
 * This code applies to Cokoino Arduino Timer Alarm&Watering DIY Kit
 * Through this link you can download the source code:
 * https://github.com/Cokoino/CKK0021
 * Company web site:
 * http://www.cokoino.com/
 
*****************************************************/
#define RELAY 9   //Connect the signal pin of the Relay module to the D10 pin of the Arduino UNO
#define LED_G 12  //Connect the signal pin of the green LED module to the D12 pin of the Arduino UNO

void setup() {
  pinMode(10,OUTPUT); //Define siganl pin D10 of Arduino UNO as output mode
  pinMode(12,OUTPUT); //Define siganl pin D12 of Arduino UNO as output mode
}
void loop() {
  digitalWrite(LED_G,HIGH);
  digitalWrite(RELAY,HIGH);//Relay closed, LED lit up
  delay(2000);//LED turns on for 2 seconds
  digitalWrite(RELAY,LOW);//Relay disconnected, LED off
  delay(2000);//LED turns off for 2 seconds
}