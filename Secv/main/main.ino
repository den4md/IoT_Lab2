#include <stdio.h>

#include "lcd.h"
#include "led.h"
#include "button.h"
#include "timer-api.h"

typedef void (*Task) ();

#define LED_ON HIGH
#define LED_OFF LOW

#define BUTTON_ON LOW
#define BUTTON_OFF HIGH

#define RS 13
#define EN 12
#define D4 11
#define D5 10
#define D6 9
#define D7 8

#define LCD_X 16
#define LCD_Y 2

#define FIRST_LED_PIN 7
#define FIRST_BUTTON_PIN 6
#define SECOND_LED_PIN 5
#define SECOND_BUTTON_UP_PIN 4
#define SECOND_BUTTON_DOWN_PIN 3

#define BUTTON_IDLE 0
#define BUTTON_PRESSED_OR_NOISE 1
#define BUTTON_HOLD 2

#define SECOND_LED_INCREMENTOR 5
#define SECOND_LED_MIN 20
#define SECOND_LED_MAX 100

#define FIRST_LED_ON "Led-1 ON"
#define FIRST_LED_OFF "Led-1 OFF"
#define FIRST_BUTTON_PRESSED "Btn-1 PRESSED"
#define SECOND_LED_ON "Led-2 ON"
#define SECOND_LED_OFF "Led-2 OFF"
#define SECOND_BUTTON_UP_PRESSED "Btn-2 PRESSED"
#define SECOND_BUTTON_DOWN_PRESSED "Btn-3 PRESSED"


int firstLedState = LED_OFF;
int firstButtonState = BUTTON_IDLE;
int secondLedState = LED_OFF;
int secondButtonUpState = BUTTON_IDLE;
int secondButtonDownState = BUTTON_IDLE;

const byte numOfTasks = 5;
int ISRInterval = 0; // ms
char** outputStrings;
bool hasNewString = false;

int queue[numOfTasks] = {1, 2, 2, 3, 2}; // setting the offsets for working intervals
int intervals[numOfTasks] = {5, 50, 5, 5, 1}; // intervals between tasks

int *secondLedQueue = queue + 1;
int *secondLedInterval = intervals + 1;

void clearOutputStrings(char* string)
{
  for (int i = 0; i < LCD_X; ++i)
  {
    string[i] = '\0';
  }
}

void outputStringsInit()
{
  outputStrings = new char*[LCD_Y];
  for (int i = 0; i < LCD_Y; ++i)
  {
    outputStrings[i] = new char[LCD_X + 1];
    clearOutputStrings(outputStrings[i]);
    outputStrings[i][LCD_X] = '\0';
  }
}

void addOutput(const char* string)
{
  for (int i = 0; i < LCD_Y - 1; ++i)
  {
    clearOutputStrings(outputStrings[i]);
    for (int j = 0; j < LCD_X; ++j)
    {
      if (outputStrings[i + 1][j] == '\0')
      {
        break;
      }
      outputStrings[i][j] = outputStrings[i + 1][j];
    }
  }

  
  clearOutputStrings(outputStrings[LCD_Y - 1]);
  for (int i = 0; i < LCD_X; ++i)
  {
    if (string[i] == '\0')
    {
      break;
    }
    outputStrings[LCD_Y - 1][i] = string[i];
  }

  hasNewString = true;
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

void FirstTask()  // check if FIRST_BUTTON is pressed
{
  if (digitalRead(FIRST_BUTTON_PIN) == BUTTON_ON)
  {
    if (firstButtonState == BUTTON_IDLE)
    {
      firstButtonState = BUTTON_PRESSED_OR_NOISE;
    }
    else if (firstButtonState == BUTTON_PRESSED_OR_NOISE)
    {
      firstButtonState = BUTTON_HOLD;
      firstLedState = switchLedState(firstLedState);
      digitalWrite(FIRST_LED_PIN, firstLedState);
//      addOutput(FIRST_BUTTON_PRESSED);
      addOutput(firstLedState == LED_ON ? FIRST_LED_ON : FIRST_LED_OFF);

      if (firstLedState == LED_OFF)
      {
        *secondLedQueue = *secondLedInterval;
      }
    }
  }
  else
  {
    firstButtonState = BUTTON_IDLE;
  }
}

void SecondTask() // check if state of SECOND_LED need to be switched
{
  if (firstLedState == LED_OFF)
  {
    secondLedState = switchLedState(secondLedState);
    digitalWrite(SECOND_LED_PIN, secondLedState);
    addOutput(secondLedState == LED_ON ? SECOND_LED_ON : SECOND_LED_OFF);
  }
}

void ThirdTask()  // check if SECOND_BUTTON_UP is pressed
{
  if (digitalRead(SECOND_BUTTON_UP_PIN) == BUTTON_ON)
  {
    if (secondButtonUpState == BUTTON_IDLE)
    {
      secondButtonUpState = BUTTON_PRESSED_OR_NOISE;
    }
    else if (secondButtonUpState == BUTTON_PRESSED_OR_NOISE)
    {
      secondButtonUpState = BUTTON_HOLD;
      if (*secondLedInterval < SECOND_LED_MAX)
      {
        *secondLedInterval += SECOND_LED_INCREMENTOR;
        *secondLedQueue += SECOND_LED_INCREMENTOR;
      }
      addOutput(SECOND_BUTTON_UP_PRESSED);
    }
  }
  else
  {
    secondButtonUpState = BUTTON_IDLE;
  }
}

void FourthTask()  // check if SECOND_BUTTON_DOWN is pressed
{
  if (digitalRead(SECOND_BUTTON_DOWN_PIN) == BUTTON_ON)
  {
    if (secondButtonDownState == BUTTON_IDLE)
    {
      secondButtonDownState = BUTTON_PRESSED_OR_NOISE;
    }
    else if (secondButtonDownState == BUTTON_PRESSED_OR_NOISE)
    {
      secondButtonDownState = BUTTON_HOLD;
      if (*secondLedInterval > SECOND_LED_MIN)
      {
        *secondLedInterval -= SECOND_LED_INCREMENTOR;
        *secondLedQueue -= SECOND_LED_INCREMENTOR;
        if (*secondLedQueue < 0)
        {
          *secondLedQueue = 0;
        }
      }
      addOutput(SECOND_BUTTON_DOWN_PRESSED);
    }
  }
  else
  {
    secondButtonDownState = BUTTON_IDLE;
  }
}

void FifthTask()  // output
{
  if (hasNewString)
  {
    clear_lcd();
    for (int i = 0; i < LCD_Y; ++i)
    {
      lcd->setCursor(0, i);
      printf(outputStrings[i]);
    }

    hasNewString = false;
  }
}

Task tasks[numOfTasks] = 
{
  FirstTask,
  SecondTask,
  ThirdTask,
  FourthTask,
  FifthTask
};

void setup()
{
  Serial.begin(9600);
  lcd = new LiquidCrystal(RS, EN, D4, D5, D6, D7);
  begin_lcd(lcd, LCD_X, LCD_Y);
  outputStringsInit();
  
  FILE* stream = fdevopen(lcd_put_char, NULL);
  stderr = stdout = stdin = stream;
  
  LedInit(FIRST_LED_PIN);
  ButtonInit(FIRST_BUTTON_PIN);
  LedInit(SECOND_LED_PIN);
  ButtonInit(SECOND_BUTTON_UP_PIN);
  ButtonInit(SECOND_BUTTON_DOWN_PIN);

  ISRInterval = 20;
  timer_init_ISR_50Hz(TIMER_DEFAULT);
}

void timer_handle_interrupts(int timer)
{
  for (int i = 0; i < numOfTasks; ++i)  // it goes through all tasks and check if some task is needed to execute
  {
    --queue[i];
    if (queue[i] <= 0)
    {
      tasks[i]();
      queue[i] = intervals[i];
    }
  }
}

void loop(){ }
