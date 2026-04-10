/*****************************************************
 
 * This code applies to Cokoino Arduino Timer Alarm&Watering DIY Kit
 * Through this link you can download the source code:
 * https://github.com/Cokoino/CKK0021
 * Company web site:
 * http://www.cokoino.com/
 
*****************************************************/
#define LED_R 10  //Connect the signal pin of the red LED module to the D11 pin of the Arduino UNO
#define LED_G 12  //Connect the signal pin of the green LED module to the D12 pin of the Arduino UNO

void setup() {
  pinMode(LED_R,OUTPUT);  //Define siganl pins D11 and D12 of Arduino UNO as output mode
  pinMode(LED_G,OUTPUT);  //Define siganl pins D11 and D12 of Arduino UNO as output mode
}
void loop() {
digitalWrite(LED_R,HIGH);//turn on red LED
digitalWrite(LED_G,HIGH);//turn on green LED
delay(1000);
digitalWrite(LED_R,LOW);//turn off red LED
digitalWrite(LED_G,LOW);//turn off green LED
delay(1000);
}
