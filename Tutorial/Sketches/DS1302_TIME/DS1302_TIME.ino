#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2); // set the LCD address to 0x27 for a 16 chars and 2 line display
#include <DS1302.h>
#include <stdio.h>

const int kCePin   = 6;  // Chip Enable
const int kIoPin   = 7;  // Input/Output
const int kSclkPin = 8;  // Serial Clock
// 初始化DS1302
DS1302 rtc(kCePin, kIoPin, kSclkPin);

// 按键引脚
const int buttonUp = 2;
const int buttonDown = 3;
const int buttonSet = 4;
const int buttonConfirm = 5;

// 继电器引脚
const int relayPin = 13;

// 变量
int setMode = 0; // 0:正常模式, 1:设置小时, 2:设置分钟
int setHour = 0;
int setMinute = 0;
bool watering = false;

void setup() {
  // 初始化LCD
  lcd.init(); // initialize the lcd 
  lcd.backlight();
  lcd.clear();

  // 初始化DS1302
  rtc.halt(false);
  rtc.writeProtect(false);

  // 设置按键为输入
  pinMode(buttonUp, INPUT_PULLUP);
  pinMode(buttonDown, INPUT_PULLUP);
  pinMode(buttonSet, INPUT_PULLUP);
  pinMode(buttonConfirm, INPUT_PULLUP);

  // 设置继电器为输出
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);

  // 显示初始信息
  lcd.setCursor(0, 0);
  lcd.print("Set Time:");
  delay(5000);
  lcd.setCursor(0, 1);
  lcd.print("HH:MM:SS");
}
/*
String dayAsString(const Time::Day day) {
  switch (day) {
    case Time::kSunday: return "Sun";
    case Time::kMonday: return "Mon";
    case Time::kTuesday: return "Tue";
    case Time::kWednesday: return "Wed";
    case Time::kThursday: return "Thu";
    case Time::kFriday: return "Fri";
    case Time::kSaturday: return "Sat";
  }
  return "(unknown day)";
}
*/
void loop() {
  // 读取当前时间
  Time t = rtc.time();
   // Name the day of the week.
  //const String day = dayAsString(t.day);

  // Format the time and date and insert into the temporary buffer.
  char buf[50];
  snprintf(buf, sizeof(buf), "%s %02d:%02d:%02d",
           t.day,t.hr, t.min, t.sec);

  // 显示当前时间
  lcd.setCursor(0, 0);
  //lcd.print("Time:");
  lcd.print(buf);
  //lcd.print(t.hr);
  //lcd.print(":");
  //lcd.print(t.min);
  //lcd.print(":");
  //lcd.print(t.sec);

  // 检查按键
  if (digitalRead(buttonSet) == HIGH) {
    setMode++;
    if (setMode > 2) setMode = 0;
    delay(200);
  }

  if (setMode == 1) {
    if (digitalRead(buttonUp) == HIGH) {
      setHour++;
      if (setHour > 23) setHour = 0;
      delay(200);
    }
    if (digitalRead(buttonDown) == HIGH) {
      setHour--;
      if (setHour < 0) setHour = 23;
      delay(200);
    }
    lcd.setCursor(0, 1);
    lcd.print("Set Hour:");
    lcd.print(setHour);
  }

  if (setMode == 2) {
    if (digitalRead(buttonUp) == HIGH) {
      setMinute++;
      if (setMinute > 59) setMinute = 0;
      delay(200);
    }
    if (digitalRead(buttonDown) == HIGH) {
      setMinute--;
      if (setMinute < 0) setMinute = 59;
      delay(200);
    }
    lcd.setCursor(0, 1);
    lcd.print("Set Min:");
    lcd.print(setMinute);
  }

  if (digitalRead(buttonConfirm) == HIGH) {
    if (setMode == 1 || setMode == 2) {
      setMode = 0;
      lcd.setCursor(0, 1);
      lcd.print("Time Set!");
      delay(1000);
    }
  }

  // 检查是否到达设定时间
  if (t.hr == setHour && t.min == setMinute && !watering) {
    watering = true;
    digitalWrite(relayPin, HIGH);
    delay(5000); // 浇水5秒
    digitalWrite(relayPin, LOW);
    watering = false;
  }
}