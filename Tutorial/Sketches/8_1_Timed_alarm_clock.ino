#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2); // set the LCD address to 0x27 for a 16 chars and 2 line display
#include <DS1302.h>
#include <stdio.h>

const int RST   = 8;  // Chip Enable
const int DAT   = 6;  // Input/Output
const int CLK = 7;  // Serial Clock
// 初始化DS1302
DS1302 rtc(RST, DAT, CLK);

// 按键引脚
const int resetButtonPin = 2;
const int startStopButtonPin = 3;
const int downButtonPin = 4;
const int upButtonPin = 5;
const int buzzerpin = 9;
const int relayPin = 10;

// 变量
int setMode = 0; // 0:正常模式, 1:设置小时, 2:设置分钟
int setupHours = 0;
int setupMinutes = 0;
int setupSeconds = 0; 
int currentHours = 0;
int currentMinutes = 0;
int currentSeconds = 0;
bool watering = false;
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
  // 初始化LCD
  lcd.init(); // initialize the lcd 
  lcd.backlight();
  lcd.clear();

  // 初始化DS1302
  rtc.halt(true);
  rtc.writeProtect(true);

  // 设置按键为输入
  pinMode(resetButtonPin, INPUT_PULLUP);
  pinMode(startStopButtonPin, INPUT_PULLUP);
  pinMode(upButtonPin, INPUT_PULLUP);
  pinMode(downButtonPin, INPUT_PULLUP);
  pinMode(relayPin, OUTPUT);
  pinMode(buzzerpin, OUTPUT);

  // 显示初始信息
  lcd.setCursor(0, 0);
  lcd.print("Set Time:");
  delay(5000);
  lcd.setCursor(0, 1);
  lcd.print("HH:MM:SS");
}

void loop() {
  // 读取当前时间
  Time t = rtc.time();

  // 显示当前时间
  //lcd.setCursor(0, 0);
  //lcd.print("Time:");
  //lcd.print(t.hr);
  //lcd.print(":");
  //lcd.print(t.min);
  //lcd.print(":");
  //lcd.print(t.sec);

  
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
  //delay(20);
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
          if (t.hr == setupHours && t.min == setupMinutes && t.sec == setupSeconds&& !watering) {
             //watering = true;
             digitalWrite(relayPin, HIGH);
             delay(5000); // 浇水5秒
             digitalWrite(relayPin, LOW);
             watering = false;
         }
        }
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
      if(t.hr == setupHours && t.min == setupMinutes)
       {
        currentMode = MODE_BUZZING;
         }
      break;
    case MODE_BUZZING:
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
      delay(1000);
      digitalWrite(relayPin,HIGH);
      delay(5000);
      digitalWrite(relayPin,LOW);
      delay(1000);
      currentMode = MODE_IDLE;
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
      lcd.print(currentHours);
      lcd.print(" ");
      lcd.print(currentMinutes);
      lcd.print(" ");
      lcd.print(currentSeconds);
      lcd.print("    ");
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
      lcd.setCursor(0, 1);
      lcd.print(setupHours);
      lcd.print(" ");
      lcd.print(setupMinutes);
      lcd.print(" ");
      lcd.print(setupSeconds);
      lcd.print("    ");
      break;
    case MODE_RUNNING:
      lcd.print("time comparison");
      lcd.setCursor(0, 1);
      lcd.print(currentHours);
      lcd.print("  ");
      lcd.print(currentMinutes);
      lcd.print(" ");
      lcd.print(currentSeconds);
      lcd.print("    ");
      break;
    case MODE_BUZZING:
      lcd.print("BUZZING");
      lcd.setCursor(0, 1);
      lcd.print("        ");
      flag = true;
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
