/*****************************************************
 
 * This code applies to Arduino Timer Alarm & Watering DIY Kit
 * Through this link you can download the source code:
 * https://github.com/Cokoino/CKK0021
 * Company web site:
 * http://www.cokoino.com/
 
*****************************************************/
#include <TimeLib.h> 
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2); // set the LCD address to 0x27 for a 16 chars and 2 line display

const int resetButtonPin = 2;
const int startStopButtonPin = 3;
const int downButtonPin = 4;
const int upButtonPin = 5;
const int relay_pin = 9;
const int LED_R = 10;
const int buzzer_pin = 11;
const int LED_G = 12;

int setupHours = 0;     // How many hours will count down when started
int setupMinutes = 0;   // How many minutes will count down when started
int setupSeconds = 0;   // How many seconds will count down when started
time_t setupTime = 0;
int currentHours = 0;
int currentMinutes = 0;
int currentSeconds = 0;
time_t currentTime = 0;
bool flag = false;

time_t startTime = 0;
time_t elapsedTime = 0;

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

int currentMode = MODE_IDLE;    // 0=idle 1=setup 2=running 3=watering
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
                        // 0=hours (00-09) 1=minutes (00-59) 2=seconds (00-59)

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  lcd.init(); // initialize the lcd 
  lcd.backlight();
  lcd.clear();
  pinMode(resetButtonPin, INPUT_PULLUP);
  pinMode(startStopButtonPin, INPUT_PULLUP);
  pinMode(upButtonPin, INPUT_PULLUP);
  pinMode(downButtonPin, INPUT_PULLUP);
  pinMode(relay_pin, OUTPUT);
  pinMode(buzzer_pin, OUTPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  //Set the green LED to turn on
  digitalWrite(LED_G,HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:
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
  //delay(20);
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
  //delay(20);
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
        if(currentMode == MODE_RUNNING)
        {
          // STARTING TIMER!
          startTime = now();
          //unsigned long startTime = now();
        }
        flag = false;
      }
      break;

    case MODE_SETUP:
      if(resetButtonPressed)
      {
        // Exit setup mode
        setupTime = setupSeconds + (60 * setupMinutes) + (3600 * setupHours);
        //unsigned long  setupTime = setupSeconds + (60 * setupMinutes) + (3600 * setupHours);
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
              setupHours = 9;
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
            if(setupHours == 10)
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
      break;
    case MODE_RUNNING:
      currentTime = setupTime - (now() - startTime);
      if(currentTime <= 0)
      {
        currentMode = MODE_Watering;
      }
      break;
    case MODE_Watering:
      buzzer_ring();
      digitalWrite(LED_G,LOW);//The green LED is off
      digitalWrite(LED_R,HIGH);//The red LED is on
      digitalWrite(relay_pin,HIGH);//Relay closed, power supply to water pump
      delay(5000);//The water pump works for 5 seconds, you can customize this time
      digitalWrite(relay_pin,LOW);//Relay disconnected, water pump stops working
      digitalWrite(LED_R,LOW);//The red LED is off
      digitalWrite(LED_G,HIGH);////The green LED is on
      currentMode = MODE_IDLE;
      flag = true;
      break;
  }

  /*
   * LCD management
   */
  //lcd.clear();
  lcd.setCursor(0, 0);
  switch(currentMode)
  {
    case MODE_IDLE:
      lcd.print("Timer ready     ");
      break;
    case MODE_SETUP:
      lcd.print("Setup mode: ");
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
     case MODE_RUNNING:
      lcd.print("Counting down...");
      break;
     case MODE_Watering:
       lcd.print("   Watering     ");
      break;
  }
  lcd.setCursor(0, 1);
  switch(currentMode)
  {
    
     case MODE_SETUP:
       if((setupHours) < 10) lcd.print("0");
       lcd.print(setupHours);
       lcd.print(":");
       if((setupMinutes) < 10) lcd.print("0");
       lcd.print(setupMinutes);
       lcd.print(":");
       if((setupSeconds) < 10) lcd.print("0");
       lcd.print(setupSeconds);
       lcd.print("    ");
       break;
    
    case MODE_RUNNING:
      if(hour(currentTime) < 10) lcd.print("0");
      lcd.print(hour(currentTime));
      lcd.print(":");
      if(minute(currentTime) < 10) lcd.print("0");
      lcd.print(minute(currentTime));
      lcd.print(":");
      if(second(currentTime) < 10) lcd.print("0");
      lcd.print(second(currentTime));
      break;
    case MODE_Watering:
      lcd.print("                ");
      break;
  }
  delay(10);
   Serial.println(currentMode);
}

void Reset()
{
  currentMode = MODE_IDLE;
  currentHours = setupHours;
  currentMinutes = setupMinutes;
  currentSeconds = setupSeconds;
}
void buzzer_ring() 
{
  for(int i = 0;i < 80; i++)
  {
    //The high and low levels constitute a square-wave signal that triggers the passive buzzer to work
    digitalWrite(buzzer_pin, HIGH);
    delay(1);
    digitalWrite(buzzer_pin, LOW);
    delay(1);
  }
  for(int j = 0;j< 100; j++)
  {
    digitalWrite(buzzer_pin, HIGH);
    delay(2);
    digitalWrite(buzzer_pin, LOW);
    delay(2);
  }
}