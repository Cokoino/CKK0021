/*****************************************************
 
 * This code applies to Cokoino Arduino Timer Alarm & Watering Kit
 * Through this link you can download the source code:
 * https://github.com/Cokoino/CKK0021
 * Company web site:
 * http://www.cokoino.com/
 
*****************************************************/
#include <LiquidCrystal_I2C.h>//Import I2C Library
LiquidCrystal_I2C lcd(0x27,16,2); // set the LCD address to 0x27 for a 16 chars and 2 line display
#include <DS1302.h>//Import DS1302 Library

//Connect DS1302 to Arduino UNO 
const int RST   = 8;  // Chip Enable
const int DAT   = 6;  // Input/Output
const int CLK = 7;  // Serial Clock
DS1302 rtc(RST, DAT, CLK);

String dayAsString(const Time::Day day) {
  switch (day) {
    case Time::kSunday: return "Sunday";
    case Time::kMonday: return "Monday";
    case Time::kTuesday: return "Tuesday";
    case Time::kWednesday: return "Wednesday";
    case Time::kThursday: return "Thursday";
    case Time::kFriday: return "Friday";
    case Time::kSaturday: return "Saturday";
  }
  return "(unknown day)";
 }
 
// // Connect the module toArduino UNO
const int resetButtonPin = 2;
const int startStopButtonPin = 3;
const int downButtonPin = 4;
const int upButtonPin = 5;
const int relayPin = 9;
const int LED_R = 10;
const int buzzerpin = 11;
const int LED_G = 12;

// Introduce variables
int setMode = 0; //0:set hours, 1:set minutes，2：set seconds
int setupHours = 0;
int setupMinutes = 0;
int setupSeconds = 0; 
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
const int MODE_BUZZING = 3;

int currentMode = MODE_IDLE;    // 0=idle 1=setup 2=running 3=buzzing
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
                        // 0=hours (00-23) 1=minutes (00-59) 2=seconds (00-59)

void setup() {
  Serial.begin(9600);
  // initialize the lcd 
  lcd.init(); 
  lcd.backlight();
  lcd.clear();

  // initialize the DS1302
  rtc.halt(false);
  rtc.writeProtect(false);

  // Set the button as input
  pinMode(resetButtonPin, INPUT_PULLUP);
  pinMode(startStopButtonPin, INPUT_PULLUP);
  pinMode(upButtonPin, INPUT_PULLUP);
  pinMode(downButtonPin, INPUT_PULLUP);
  //Set relay and LED as outputs
  pinMode(relayPin, OUTPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  //Set the green LED to turn on
  digitalWrite(LED_G,HIGH);
}

void printTime() {
  // Get the current time and date from the chip.
  Time t = rtc.time();

  // Name the day of the week.
  const String day = dayAsString(t.day);

  // Format the time and date and insert into the temporary buffer.
  char buf[50];
  snprintf(buf, sizeof(buf), "%s %04d-%02d-%02d %02d:%02d:%02d",
           day.c_str(),
           t.yr, t.mon, t.date,
           t.hr, t.min, t.sec);

  // Print the formatted string to serial so we can see the time.
  Serial.println(buf);
  lcd.setCursor(0, 0);//Display starting from the first row and first grid of 1602 LCD
  //Display real-time time
  if((t.hr) < 10) lcd.print("0");
  lcd.print(t.hr);
  lcd.print(":");
  if((t.min) < 10) lcd.print("0");
  lcd.print(t.min);
  lcd.print(":");
  if((t.sec) < 10) lcd.print("0");
  lcd.print(t.sec);
}
void alarm_clock(){
  Time t = rtc.time();// Read the current time
  startStopButtonPressed = false;
  upButtonPressed = false;
  downButtonPressed = false;
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
      if(resetButtonLongPressCounter == 50)//Press and hold the reset button for 5 seconds
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
      Serial.println(flag);
      if(startStopButtonPressed || flag)
      {
        currentMode = currentMode == MODE_IDLE ? MODE_RUNNING : MODE_IDLE;
        flag = false;
      }
      break;

    case MODE_SETUP:
      if(resetButtonPressed)
      {
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
        if(dataSelection == 3)
        {
          dataSelection = 0;
        }
      }
      if(downButtonPressed)
      {
        switch(dataSelection)
        {
          case 0: // hours
            setupHours--;
            if(setupHours == -1)
            {
              setupHours = 23;
            }
            break;
          case 1: // minutes
            setupMinutes--;
            if(setupMinutes == -1)
            {
              setupMinutes = 59;
            }
            break;
          case 2: // seconds
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
          case 0: // hours
            setupHours++;
            if(setupHours == 24)
            {
              setupHours = 0;
            }
            break;
          case 1: // minutes
            setupMinutes++;
            if(setupMinutes == 60)
            {
              setupMinutes = 0;
            }
            break;
          case 2: // seconds
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

    case MODE_BUZZING:
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
      if(t.hr == setupHours && t.min == setupMinutes)//Set alarm time, hours and minutes
       {
        currentMode = MODE_BUZZING;//Enter buzzer working mode
         }
      break;
    case MODE_BUZZING:
      digitalWrite(LED_R,HIGH);//The red LED is on
      digitalWrite(LED_G,LOW);//The green LED is off
      //The buzzer sounds for one minute, and you can modify the parameters yourself to change the duration of the buzzer sound
      for(int i = 0;i < 30; i++)
      {
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
      }
      digitalWrite(LED_R,LOW);//The red LED is off
      digitalWrite(LED_G,HIGH);//The green LED is on
      currentMode = MODE_IDLE;
      break;
  }

  /*
   * LCD management
   */
  //lcd.clear();
  lcd.setCursor(9, 0);
  switch(currentMode)
  {
    case MODE_IDLE:
      lcd.print("Ready  ");
      break;
    case MODE_SETUP:
      lcd.print("M:");
      switch(dataSelection)
      {
        case 0:
          lcd.print("HRS ");
          break;
        case 1:
          lcd.print("MINS");
          break;
        case 2:
          lcd.print("SECS");
          break;
      }
      break;
    case MODE_RUNNING:
      lcd.print("Runing ");
      break;
    case MODE_BUZZING:
      lcd.print("BUZZING");
      flag = true;
      break;
  }
lcd.setCursor(0, 1);
  switch(currentMode)
  {
    case MODE_IDLE:
      if(currentHours < 10) lcd.print("0");
      lcd.print(currentHours);
      lcd.print(":");
      if(currentMinutes < 10) lcd.print("0");
      lcd.print(currentMinutes);
      lcd.print(":");
      if(currentSeconds < 10) lcd.print("0");
      lcd.print(currentSeconds);
      break;
    case MODE_SETUP:
      lcd.setCursor(0, 1);
      if(setupHours < 10) lcd.print("0");
      lcd.print(setupHours);
      lcd.print(":");
      if(setupMinutes < 10) lcd.print("0");
      lcd.print(setupMinutes);
      lcd.print(":");
      if(setupSeconds < 10) lcd.print("0");
      lcd.print(setupSeconds);
      break;
  }

}

void Reset()
{
  currentMode = MODE_IDLE;
  currentHours = setupHours;
  currentMinutes = setupMinutes;
  currentSeconds = setupSeconds;
}

void loop()
 {
 printTime();
 alarm_clock();
}


