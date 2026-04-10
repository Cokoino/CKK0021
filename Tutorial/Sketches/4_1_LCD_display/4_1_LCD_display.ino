/*****************************************************
 
 * This code applies to Cokoino Arduino Timer Alarm&Watering DIY Kit
 * Through this link you can download the source code:
 * https://github.com/Cokoino/CKK0021
 * Company web site:
 * http://www.cokoino.com/
 
*****************************************************/

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2); // set the LCD address to 0x27 for a 16 chars and 2 line display

void setup()
{
  lcd.init(); // initialize the lcd 
  lcd.backlight();
  lcd.clear();
  
}

void loop()
{
  lcd.setCursor(2,0);//set the line and the char begaining for the print information
  lcd.print("Hello World!");
  lcd.setCursor(1,1); 
  lcd.print("Hello Cokoino!");                             
}
