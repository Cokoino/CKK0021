/*****************************************************
 
 * This code applies to Arduino Timer Alarm & Watering Kit
 * Through this link you can download the source code:
 * https://github.com/Cokoino/CKK0021
 * Company web site:
 * http://www.cokoino.com/
 
*****************************************************/
#include <LiquidCrystal_I2C.h> //Import I2C Library
LiquidCrystal_I2C lcd(0x27,16,2); // set the LCD address to 0x27 for a 16 chars and 2 line display
#include <DS1302.h>//Import DS1302 Library
//#include <stdio.h>

const int RSTPin   = 8;  // Chip Enable
const int DATPin   = 6;  // Input/Output
const int CLKPin = 7;  // Serial Clock

DS1302 rtc(RSTPin, DATPin, CLKPin);//Connect DS1302 to UNO R3
String dayAsString(const Time::Day day) {
  switch (day) {
    case Time::kSunday: return "Sunday";//The number 1 represents Sunday
    case Time::kMonday: return "Monday";//The number 2 represents Monday
    case Time::kTuesday: return "Tuesday";
    case Time::kWednesday: return "Wednesday";
    case Time::kThursday: return "Thursday";
    case Time::kFriday: return "Friday";
    case Time::kSaturday: return "Saturday";
  }
  return "(unknown day)";
 }

// Connect the module to UNO R3
const int resetButtonPin = 2;
const int startStopButtonPin = 3;
const int downButtonPin = 4;
const int upButtonPin = 5;
const int relayPin = 9;
const int LED_R = 10;
const int buzzerpin = 11;
const int LED_G = 12;

// Introduce variables
int setMode = 0; // 0:Set the day of the week, 1:set hours, 2:set minutes，3：set seconds
int setupday = 1;
int setupHours = 0;
int setupMinutes = 0;
int setupSeconds = 0; 
int currentday = 1;
int currentHours = 0;
int currentMinutes = 0;
int currentSeconds = 0;
bool flag = false;

int resetButtonState = LOW;
long resetButtonLongPressCounter = 0;
int startStopButtonState = LOW;
int upButtonState = LOW;
int downButtonState = LOW;
int resetButtonPrevState = LOW;
int startStopButtonPrevState = LOW;
int upButtonPrevState = LOW;
int downButtonPrevState = LOW;
bool resetButtonPressed = false;
bool resetButtonLongPressed = false;
bool startStopButtonPressed = false;
bool upButtonPressed = false;
bool downButtonPressed = false;

const int MODE_IDLE = 0;
const int MODE_SETUP = 1;
const int MODE_RUNNING = 2;
const int MODE_Watering = 3;

int currentMode = MODE_IDLE;    // 0=idle 1=setup 2=running 3=Watering
                                // Power up --> idle
                                // Reset --> idle
                                //  Start/Stop --> start or stop counter
                                //  Up / Down --> NOP
                                // Reset (long press) --> enter setup
                                //   Start/Stop --> data select
                                //   Up --> increase current data value
                                //   Down --> decrease current data value
                                //   Reset --> exit setup (idle)

int dataSelection = 0;  // Currently selected data for edit (setup mode, changes with Start/Stop)
                        // 0=hours (00-24) 1=minutes (00-59) 2=seconds (00-59)

void setup() {
  Serial.begin(9600);
  // initialize the lcd 
  lcd.init(); 
  lcd.backlight();
  lcd.clear();

  // Initialize DS1302
  rtc.halt(false);
  rtc.writeProtect(false);

  // Set pin mode
  pinMode(resetButtonPin, INPUT_PULLUP);
  pinMode(startStopButtonPin, INPUT_PULLUP);
  pinMode(upButtonPin, INPUT_PULLUP);
  pinMode(downButtonPin, INPUT_PULLUP);
  pinMode(relayPin, OUTPUT);
  pinMode(buzzerpin, OUTPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);

 //Set the green LED to turn on
  digitalWrite(LED_G,HIGH);
  /*
   * Display initial information
   */
  lcd.setCursor(0, 0);
  lcd.print("Set Your Time:");//Set the desired weekly watering time
  delay(2000);
  lcd.setCursor(0, 1);
  lcd.print("DAY HH:MM:SS");
}

void loop() {
  Time t = rtc.time();// Read real-time time
  startStopButtonPressed = false;
  upButtonPressed = false;
  downButtonPressed = false;
  /*
   * Reset button management
   */
  resetButtonPressed = false;
  resetButtonLongPressed = false;
  resetButtonState = digitalRead(resetButtonPin);
  delay(20);
  if(resetButtonState != resetButtonPrevState)
  {
    resetButtonPressed = resetButtonState == HIGH;
    resetButtonPrevState = resetButtonState;
  }
  else  // Long press management...
  {
    if(resetButtonState == HIGH)
    {
      resetButtonLongPressCounter++;
      if(resetButtonLongPressCounter == 50)
      {
        resetButtonPressed = false;
        resetButtonLongPressed = true;
        resetButtonLongPressCounter = 0;
      }
    }
    else
    {
      resetButtonLongPressCounter = 0;
      resetButtonPressed = false;
      resetButtonLongPressed = false;
    }
  }

  /*
   * Start/Stop button management
   */
  startStopButtonPressed = false;
  startStopButtonState = digitalRead(startStopButtonPin);
  delay(20);
  if(startStopButtonState != startStopButtonPrevState)
  {
    startStopButtonPressed = startStopButtonState == HIGH;
    startStopButtonPrevState = startStopButtonState;
  }

  /*
   * Down button management
   */
  downButtonPressed = false;
  downButtonState = digitalRead(downButtonPin);
  delay(20);
  if(downButtonState != downButtonPrevState)
  {
    downButtonPressed = downButtonState == HIGH;
    downButtonPrevState = downButtonState;
  }

  /*
   * Up button management
   */
  upButtonPressed = false;
  upButtonState = digitalRead(upButtonPin);
  delay(20);
  if(upButtonState != upButtonPrevState)
  {
    upButtonPressed = upButtonState == HIGH;
    upButtonPrevState = upButtonState;
  }

  /*
   * Mode management
   */
  switch(currentMode)
  {
    case MODE_IDLE:
      if(resetButtonPressed)
      {
        Reset();
      }
      if(resetButtonLongPressed)
      {
        currentMode = MODE_SETUP;
      }
      //Serial.println(flag);
      if(startStopButtonPressed || flag)
      {
        currentMode = currentMode == MODE_IDLE ? MODE_RUNNING : MODE_IDLE;
        flag = false;
      }
      break;

    case MODE_SETUP:
      if(resetButtonPressed)
      {
        currentday = setupday;
        currentHours = setupHours;
        currentMinutes = setupMinutes;
        currentSeconds = setupSeconds;
        dataSelection = 0;
        currentMode = MODE_IDLE;
      }
      if(startStopButtonPressed)
      {
        // Select next data to adjust
        dataSelection++;
        if(dataSelection == 4)
        {
          dataSelection = 0;
        }
      }
      if(downButtonPressed)
      {
        switch(dataSelection)
        {
           case 0: // day
            setupday--;
            if(setupday == 0)
            {
              setupday = 7;//Saturday
            }
            break;
          case 1: // hours
            setupHours--;
            if(setupHours == -1)
            {
              setupHours = 23;
            }
            break;
          case 2: // minutes
            setupMinutes--;
            if(setupMinutes == -1)
            {
              setupMinutes = 59;
            }
            break;
          case 3: // seconds
            setupSeconds--;
            if(setupSeconds == -1)
            {
              setupSeconds = 59;
            }
            break;
        }
      }
      if(upButtonPressed)
      {
        switch(dataSelection)
        {
          case 0: // week
            setupday++;
            if(setupday == 8)
            {
              setupday = 1;//Sunday
            }
            break;
          case 1: // hours
            setupHours++;
            if(setupHours == 24)
            {
              setupHours = 0;
            }
            break;
          case 2: // minutes
            setupMinutes++;
            if(setupMinutes == 60)
            {
              setupMinutes = 0;
            }
            break;
          case 3: // seconds
            setupSeconds++;
            if(setupSeconds == 60)
            {
              setupSeconds = 0;
            }
            break;
        }
      }
      break;
    
    case MODE_RUNNING:
      if(startStopButtonPressed)
      {
        currentMode = MODE_IDLE;
      }
      if(resetButtonPressed)
      {
        Reset();
        currentMode = MODE_IDLE;
      }
      break;

    case MODE_Watering:
      if(resetButtonPressed || startStopButtonPressed || downButtonPressed || upButtonPressed)
      {
        currentMode = MODE_IDLE;
      }
      break;
  }

  /*
   * Time management
   */
  switch(currentMode)
  {
    case MODE_IDLE:
    case MODE_SETUP:
      // NOP
      break;
    case MODE_RUNNING:
       //When the real-time time reaches the set weekly watering time, the running mode switches to watering mode
      if(t.day ==setupday && t.hr == setupHours && t.min == setupMinutes && t.sec == setupSeconds)
       {
        currentMode = MODE_Watering;
         }
      break;
    case MODE_Watering:
      digitalWrite(LED_G,LOW);//The green LED is off
      digitalWrite(LED_R,HIGH);//The red LED is on
      tone(buzzerpin, 262);  //output the sound with frequency of 262Hz
      delay(250);   //delay in 250ms
      tone(buzzerpin, 294);;   //output the sound with frequency of 294Hz
      delay(250);   //delay in 250ms
      tone(buzzerpin, 330);
      delay(250);
      tone(buzzerpin, 349);
      delay(250);
      tone(buzzerpin, 392);
      delay(250);
      tone(buzzerpin, 440);
      delay(250);
      tone(buzzerpin, 494);
      delay(250);
      tone(buzzerpin, 532);
      delay(250);
      noTone(buzzerpin);   //stop sound output
      digitalWrite(relayPin,HIGH);//Relay closed, power supply to water pump
      delay(30000);//The water pump works for 30 seconds, you can customize this time
      digitalWrite(relayPin,LOW);//Relay disconnected, water pump stops working
      digitalWrite(LED_R,LOW);//The red LED is off
      digitalWrite(LED_G,HIGH);//The green LED is on
      currentMode = MODE_IDLE;
      flag = true;
      break;
  }

  /*
   * LCD management
   */
  lcd.setCursor(0, 0);
  //Display real-time time
  switch(currentMode)
  {
    case MODE_IDLE:
      lcd.print(t.day);
      lcd.print(" ");
      if(t.hr < 10) lcd.print("0");
      lcd.print(t.hr);
      lcd.print(":");
      if(t.min < 10) lcd.print("0");
      lcd.print(t.min);
      lcd.print(":");
      if(t.sec < 10) lcd.print("0");
      lcd.print(t.sec);
      lcd.print(" ");
      break;
    case MODE_RUNNING:
      lcd.print(t.day);
      lcd.print(" ");
      if(t.hr < 10) lcd.print("0");
      lcd.print(t.hr);
      lcd.print(":");
      if(t.min < 10) lcd.print("0");
      lcd.print(t.min);
      lcd.print(":");
      if(t.sec < 10) lcd.print("0");
      lcd.print(t.sec);
      lcd.print(" ");
      break;
  }
 
  lcd.setCursor(11, 0);
  switch(currentMode)
  {
    case MODE_IDLE:
      lcd.print("Ready");
      break;
    case MODE_SETUP:
      lcd.print("M:");
      switch(dataSelection)
      {
        case 0:
          lcd.print("DAY ");
          break;
        case 1:
          lcd.print("HRS ");
          break;
        case 2:
          lcd.print("MINS");
          break;
        case 3:
          lcd.print("SECS");
          break;
      }
      break;
    case MODE_RUNNING:
      lcd.print("Run  ");
      break;
    case MODE_Watering:
      lcd.print("Rain ");
      flag = true;
      break;
  }

  lcd.setCursor(0, 1);
  //Display the set weekly watering time
  switch(currentMode)
  {
    case MODE_IDLE:
      break;
    case MODE_SETUP:
      lcd.print(setupday);
      lcd.print(" ");
      if (setupHours <10) lcd.print("0");
      lcd.print(setupHours);
      lcd.print(":");
      if (setupMinutes <10) lcd.print("0");
      lcd.print(setupMinutes);
      lcd.print(":");
      if (setupSeconds <10) lcd.print("0");
      lcd.print(setupSeconds);
      lcd.print("  ");
      break;
    case MODE_RUNNING:
      lcd.print(currentday);
      lcd.print(" ");
      if (currentHours <10) lcd.print("0");
      lcd.print(currentHours);
      lcd.print(":");
      if (currentMinutes <10) lcd.print("0");
      lcd.print(currentMinutes);
      lcd.print(":");
      if (currentSeconds <10) lcd.print("0");
      lcd.print(currentSeconds);
      lcd.print("  ");
      break;
  }
}
void Reset()
{
  currentMode = MODE_IDLE;
  currentday = setupday;
  currentHours = setupHours;
  currentMinutes = setupMinutes;
  currentSeconds = setupSeconds;
}
