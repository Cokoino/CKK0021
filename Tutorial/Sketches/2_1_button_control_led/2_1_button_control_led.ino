/*****************************************************
 
 * This code applies to Cokoino Arduino Timer Alarm&Watering DIY Kit
 * Through this link you can download the source code:
 * https://github.com/Cokoino/CKK0021
 * Company web site:
 * http://www.cokoino.com/
 
*****************************************************/
#define Button_R 2   //Connect the signal pin of the red Button module to the D2 pin of the Arduino UNO
#define Button_G 3  //Connect the signal pin of the green Button module to the D3 pin of the Arduino UNO
#define LED_R 10   //Connect the signal pin of the red LED module to the D11 pin of the Arduino UNO
#define LED_G 12  //Connect the signal pin of the green LED module to the D12 pin of the Arduino UNO

boolean key_1=false,key_2=false;//Define two boolean variables, key_1 and key_2, and initialize them to false

void setup() {
  pinMode(2,INPUT);   //Define siganl pin D2 of Arduino UNO as input mode
  pinMode(3,INPUT);   //Define siganl pin D3 of Arduino UNO as input mode
  pinMode(10,OUTPUT); //Define siganl pin D11 of Arduino UNO as output mode
  pinMode(12,OUTPUT); //Define siganl pin D12 of Arduino UNO as output mode
}
void loop() {
  if(digitalRead(2)==HIGH){             //detect if the red button is pressed.
    delay(10);                         //delay 10 ms
    if(digitalRead(2)==HIGH){         //detect again if the red button is pressed.
      key_1 = !key_1;                //record the mark if the red button is pressed.
      while(digitalRead(2)==HIGH);  //wait for the red button is released.
      }
    }
  if(digitalRead(3)==HIGH){            //detect if the green button is pressed.  
    delay(10);                        //delay 10 ms
    if(digitalRead(3)==HIGH){        //detect again if the green button is pressed.
      key_2 = !key_2;               //record the mark if the green button is pressed.
      while(digitalRead(3)==HIGH); //wait for the green button is released.
      }   
    }
  
  if(key_1==true) {digitalWrite(10,HIGH);} //turn on red LED
  else            {digitalWrite(10,LOW);}  //turn off red LED
  if(key_2==true) {digitalWrite(12,HIGH);} //turn on green LED
  else            {digitalWrite(12,LOW);}  //turn off green LED
}