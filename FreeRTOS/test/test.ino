#include "lcd.h"
#include "led.h"
#include "button.h"

#define LED_ON LOW
#define LED_OFF HIGH

//#define BUTTON_ON LOW
//#define BUTTON_OFF HIGH
#define BUTTON_ON HIGH
#define BUTTON_OFF LOW

#define RS 15
#define EN 2
#define D4 0
#define D5 4
#define D6 16
#define D7 17

#define LCD_X 16
#define LCD_Y 2

#define FIRST_LED_PIN 32
#define FIRST_BUTTON_PIN 26
#define SECOND_LED_PIN 33
#define SECOND_BUTTON_UP_PIN 25
#define SECOND_BUTTON_DOWN_PIN 27

#define BUTTON_IDLE 0
#define BUTTON_PRESSED_OR_NOISE 1
#define BUTTON_HOLD 2

int firstLedState = LED_OFF;
int firstButtonState = BUTTON_IDLE;
int secondLedState = LED_OFF;
int secondButtonUpState = BUTTON_IDLE;
int secondButtonDownState = BUTTON_IDLE;


void myPrintf(const char* cstring)
{
  while (*cstring != '\0')
  {
    lcd_put_char(*cstring, NULL);
    ++cstring;
  }
}

int switchLedState(int ledState)
{
  if (ledState == LED_ON)
  {
    return LED_OFF;
  }
  else
  {
    return LED_ON;
  }
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  lcd = new LiquidCrystal(RS, EN, D4, D5, D6, D7);
  begin_lcd(lcd, LCD_X, LCD_Y);
//  outputStringsInit();
  
  LedInit(FIRST_LED_PIN);
  digitalWrite(FIRST_LED_PIN, firstLedState);
  ButtonInit(FIRST_BUTTON_PIN);
  LedInit(SECOND_LED_PIN);
  digitalWrite(SECOND_LED_PIN, secondLedState);
  ButtonInit(SECOND_BUTTON_UP_PIN);
  ButtonInit(SECOND_BUTTON_DOWN_PIN);

  myPrintf("Stolearov ;)\0");

}

void loop() {
  // put your main code here, to run repeatedly:
  if (digitalRead(FIRST_BUTTON_PIN) == BUTTON_ON)
  {
//      digitalWrite(FIRST_LED_PIN, LED_ON);
    Serial.print("1\n");
  }
//  else
//  {
//    digitalWrite(FIRST_LED_PIN, LED_OFF);
//  }
if (digitalRead(SECOND_BUTTON_UP_PIN) == BUTTON_ON)
{
  Serial.print("2--\n");
}
if (digitalRead(SECOND_BUTTON_DOWN_PIN) == BUTTON_ON)
{
  Serial.print("3-----\n");
}
//digitalWrite(FIRST_LED_PIN, LED_OFF);
//delay(200);
//digitalWrite(FIRST_LED_PIN, LED_ON);
delay(200);
}
