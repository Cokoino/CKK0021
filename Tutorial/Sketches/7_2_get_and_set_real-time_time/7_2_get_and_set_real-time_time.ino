 /*****************************************************
 
 * This code applies to Cokoino Arduino Timer Alarm & Watering DIY Kit
 * Through this link you can download the source code:
 * https://github.com/Cokoino/CKK0021
 * Company web site:
 * http://www.cokoino.com/
 
*****************************************************/


// CONNECTIONS:
// DS1302 CLK/SCLK --> 7
// DS1302 DAT/IO --> 6
// DS1302 RST/CE --> 8
// DS1302 VCC --> 5v
// DS1302 GND --> GND
 
#include <RtcDS1302.h> //import Rtc_by_Makuna library
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2); // set the LCD address to 0x27 for a 16 chars and 2 line display
 
ThreeWire myWire(6,7,8); // DAT, CLK, RST
RtcDS1302<ThreeWire> Rtc(myWire);
 
void setup () 
{
  Serial.begin(9600);
  DS1302_init();// initialize the DS1302
  lcd.init(); // initialize the lcd 
  lcd.backlight();
  lcd.clear();
}
 
void loop () 
{
    ds1302_loop();
}
 
#define countof(a) (sizeof(a) / sizeof(a[0]))
 
void printDateTime(const RtcDateTime& dt)
{
    char datestring[26];
 
    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
            dt.Month(),
            dt.Day(),
            dt.Year(),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
    Serial.print(datestring);//Extract parameters such as month, day, year, hour, minute, and second, and convert them into string format
    lcd.setCursor(0, 0);//Set the display to start from the first row and first grid of the 1602 LCD
    lcd.print(dt.Month());//Display 'Month'
    lcd.print("-");
    lcd.print(dt.Day());//Display 'Day'
    lcd.print("-");
    lcd.print(dt.Year());//Display 'Year'
    lcd.setCursor(0, 1);
    if((dt.Hour()) < 10) lcd.print("0");
    lcd.print(dt.Hour());//Display 'Hour'
    lcd.print(":");
    if((dt.Minute()) < 10) lcd.print("0");
    lcd.print(dt.Minute());//Display 'Minute'
    lcd.print(":");
    if((dt.Second()) < 10) lcd.print("0");
    lcd.print(dt.Second());////Display 'Second'
}
 
 
void DS1302_init()
{
   Serial.print("compiled: ");
    Serial.print(__DATE__);
    Serial.println(__TIME__);//Printing system compilation time
    Rtc.Begin();
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__); //Store the system compilation time in ds1302
    printDateTime(compiled);
    Serial.println();
    if (!Rtc.IsDateTimeValid()) 
    {
        //  Common reasons:
        //    1) The device was not running during the first run
        //    2) The battery on the device is low or even dead
        Serial.println("RTC lost confidence in the DateTime!");
        Rtc.SetDateTime(compiled);
    }
 
    if (Rtc.GetIsWriteProtected())//Check if write protection is enabled, if so, turn off write protection
    {
        Serial.println("RTC was write protected, enabling writing now");
        Rtc.SetIsWriteProtected(false);
    }
 
    if (!Rtc.GetIsRunning())//Check if RTC is running from now on
    {
        Serial.println("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
    }
 
    RtcDateTime now = Rtc.GetDateTime(); //Assign the normal time of the last calibration run to now
    if (now < compiled) //If the internal time of the module is slower than the burning time of the module, it proves that time is needed to update
    {
        Serial.println("RTC is older than compile time!  (Updating DateTime)");
        Rtc.SetDateTime(compiled);//Calibrate the burning time to the internal time of the module
    }
    else if (now > compiled) 
    {
        Serial.println("RTC is newer than compile time. (this is expected)");//RTC is faster than the corresponding time during code burning. (This is a normal situation)
    }
    else if (now == compiled) 
    {
        Serial.println("RTC is the same as compile time! (not expected but all is fine)");//This is the best scenario
    }
}
 
void ds1302_loop()
{
  RtcDateTime now = Rtc.GetDateTime();//Get the current time 
  printDateTime(now);//print time
  Serial.println();//Enter and wrap
  if (!now.IsValid())//Check if the device is online
  {
      // Common reasons:
      //    1) The battery on the device is low or even lost, and the power cord is disconnected
      Serial.println("RTC lost confidence in the DateTime!");
  }
  delay(10); //delay 10 ms
}