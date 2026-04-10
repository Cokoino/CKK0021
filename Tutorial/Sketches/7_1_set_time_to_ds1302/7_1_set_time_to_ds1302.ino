//DS1302 default time: 0:00, 0， 0, 2001

#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2); // set the LCD address to 0x27 for a 16 chars and 2 line display
#include <DS1302.h>

namespace {

// Set the appropriate digital I/O pin connections. These are the pin
// assignments for the Arduino as well for as the DS1302 chip. See the DS1302
// datasheet:
//
//   http://datasheets.maximintegrated.com/en/ds/DS1302.pdf
const int RSTPin   = 8;  // Chip Enable
const int DATPin   = 6;  // Input/Output
const int CLKPin = 7;  // Serial Clock

// Create a DS1302 object.
DS1302 rtc(RSTPin, DATPin, CLKPin);

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
}
void setup() {
  Serial.begin(9600);
  // initialize the lcd 
  lcd.init(); 
  lcd.backlight();
  lcd.clear();
  // Initialize a new chip by turning off write protection and clearing the
  // clock halt flag. These methods needn't always be called. See the DS1302
  // datasheet for details.
  rtc.writeProtect(false);
  rtc.halt(false);

  // Make a new time object to set the date and time.
  // Tuesday, May 13, 2025 at 12:00:00.
  // You can also change this time according to your own needs
  Time t(2025, 05, 13, 12, 00, 00, Time::kTuesday);

  // Set the time and date on the chip.
  rtc.time(t);
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
  lcd.setCursor(0, 0);
  lcd.print(t.mon);
  lcd.print("-");
  lcd.print(t.date);
  lcd.print("-");
  lcd.print(t.yr);
  lcd.print("    ");
  lcd.print(t.day);
  lcd.setCursor(0, 1);
  if((t.hr) < 10) lcd.print("0");
  lcd.print(t.hr);
  lcd.print(":");
  if((t.min) < 10) lcd.print("0");
  lcd.print(t.min);
  lcd.print(":");
  if((t.sec) < 10) lcd.print("0");
  lcd.print(t.sec);
}

// Loop and print the time every second.
void loop() {
  printTime();
  delay(1000);
}
