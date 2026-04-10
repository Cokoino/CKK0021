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


const int RSTPin   = 8;  // Chip Enable
const int DATPin   = 6;  // Input/Output
const int CLKPin = 7;  // Serial Clock

DS1302 rtc(RSTPin, DATPin, CLKPin);//Connect DS1302 to  Arduino UNO 

// Connect the module toArduino UNO
const int resetButtonPin = 2;
const int startStopButtonPin = 3;
const int downButtonPin = 4;
const int upButtonPin = 5;
const int relay_pin = 9;
const int LED_R = 10;
const int buzzer_pin = 11;
const int LED_G = 12;

// Introduce variables
int setMode = 0;  // 0:Set the day of the week, 1:set hours, 2:set minutes，3：set seconds
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
const int MODE_Watering = 3;

int currentMode = MODE_IDLE;    // 0=idle 1=setup 2=running 3=ringing
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
  pinMode(relay_pin, OUTPUT);
  pinMode(buzzer_pin, OUTPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);

  /*
   * Display initial information
   */
  lcd.setCursor(0, 0);
  lcd.print("Set Your Time:");//Set the desired dayly watering time
  delay(2000);
  lcd.setCursor(0, 1);
  lcd.print("HH:MM:SS");

  //Set the green LED to turn on
  digitalWrite(LED_G,HIGH);
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
    //When the real-time time reaches the set dayly watering time, the running mode switches to watering mode
      if(t.hr == setupHours && t.min == setupMinutes && t.sec == setupSeconds)
       {
        currentMode = MODE_Watering;
         }
      break;
    case MODE_Watering:
      buzzer_ring();
      digitalWrite(LED_G,LOW);//The green LED is off
      digitalWrite(LED_R,HIGH);//The red LED is on
      digitalWrite(relay_pin,HIGH);//Relay closed, power supply to water pump
      delay(10000);//The water pump works for 10 seconds, you can customize this time
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
      if(t.hr < 10) lcd.print("0");
      lcd.print(t.hr);
      lcd.print(":");
      if(t.min < 10) lcd.print("0");
      lcd.print(t.min);
      lcd.print(":");
      if(t.sec < 10) lcd.print("0");
      lcd.print(t.sec);
      lcd.print(" Ready  ");
      break;
    case MODE_RUNNING:
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
   lcd.setCursor(9, 0);
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
      lcd.print("Running");
      break;
    case MODE_Watering:
      lcd.print("Waterin");
      //flag = true;
      break;
  }
  lcd.setCursor(0, 1);
  switch(currentMode)
  {
    case MODE_IDLE:
      break;
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
      if(currentHours < 10) lcd.print("0");
      lcd.print(currentHours);
      lcd.print(":");
      if(currentMinutes < 10) lcd.print("0");
      lcd.print(currentMinutes);
      lcd.print(":");
      if(currentSeconds < 10) lcd.print("0");
      lcd.print(currentSeconds);
      lcd.print("    ");
      break;
  }
   //Serial.println(currentMode);
}

void buzzer_ring() 
{
  tone(buzzer_pin, 262);  //output the sound with frequency of 262Hz
  delay(250);   //delay in 250ms
  tone(buzzer_pin, 294);;   //output the sound with frequency of 294Hz
  delay(250);   //delay in 250ms
  tone(buzzer_pin, 330);
  delay(250);
  tone(buzzer_pin, 349);
  delay(250);
  tone(buzzer_pin, 392);
  delay(250);
  tone(buzzer_pin, 440);
  delay(250);
  tone(buzzer_pin, 494);
  delay(250);
  tone(buzzer_pin, 532);
  delay(250);
  noTone(buzzer_pin);   //stop sound output
  delay(1000);    
}


void Reset()
{
  currentMode = MODE_IDLE;
  currentHours = setupHours;
  currentMinutes = setupMinutes;
  currentSeconds = setupSeconds;
}

