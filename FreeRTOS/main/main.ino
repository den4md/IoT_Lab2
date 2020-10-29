#include <stdio.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

#include "lcd.h"
#include "led.h"
#include "button.h"

typedef void (*Task) (void *parameters);

#define LED_ON LOW
#define LED_OFF HIGH
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

#define SECOND_LED_INCREMENTOR 100
#define SECOND_LED_MIN 400
#define SECOND_LED_MAX 2000

#define FIRST_LED_ON "Led-1 ON"
#define FIRST_LED_OFF "Led-1 OFF"
#define FIRST_BUTTON_PRESSED "Btn-1 PRESSED"
#define SECOND_LED_ON "Led-2 ON"
#define SECOND_LED_OFF "Led-2 OFF"
#define SECOND_BUTTON_UP_PRESSED "Btn-2 PRESSED"
#define SECOND_BUTTON_DOWN_PRESSED "Btn-3 PRESSED"

#define FIRST_TASK_DELAY_MS 100
#define THIRD_TASK_DELAY_MS 100
#define FOURTH_TASK_DELAY_MS 100
#define FIFTH_TASK_DELAY_MS 30
// #define NUM_OF_USED_WORDS 39  // 75 words == 300 bytes; 300 bytes * 5 tasks == 1500 bytes of RAM, actual program has 1528 free RAM

int firstLedState = LED_OFF;
int firstButtonState = BUTTON_IDLE;
int secondLedState = LED_OFF;
int secondButtonUpState = BUTTON_IDLE;
int secondButtonDownState = BUTTON_IDLE;

const byte numOfTasks = 5;
char** outputStrings;
bool hasNewString = false;

TaskHandle_t *BlockTaskHandle = NULL;  // First LED should suspend/resume the seccond
SemaphoreHandle_t mutex;
int secondLedIntervalMS = 1000;

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

void FirstTask(void *parameters)  // check if FIRST_BUTTON is pressed
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
      while (xSemaphoreTake(mutex, 0) == pdFALSE) {}
      addOutput(firstLedState == LED_ON ? FIRST_LED_ON : FIRST_LED_OFF);

      if (firstLedState == LED_OFF)
      {
        vTaskResume(*BlockTaskHandle);
      }
      else
      {
        vTaskSuspend(*BlockTaskHandle);
      }
      xSemaphoreGive(mutex);
    }
  }
  else
  {
    firstButtonState = BUTTON_IDLE;
  }

  vTaskDelay(pdMS_TO_TICKS(FIRST_TASK_DELAY_MS));
}

void SecondTask(void *parameters) // check if state of SECOND_LED need to be switched
{
  if (firstLedState == LED_OFF)
  {
    secondLedState = switchLedState(secondLedState);
    digitalWrite(SECOND_LED_PIN, secondLedState);
    while (xSemaphoreTake(mutex, 0) == pdFALSE) {}
    addOutput(secondLedState == LED_ON ? SECOND_LED_ON : SECOND_LED_OFF);
    xSemaphoreGive(mutex);
  }
  vTaskDelay(pdMS_TO_TICKS(secondLedIntervalMS));
}

void ThirdTask(void *parameters)  // check if SECOND_BUTTON_UP is pressed
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
      if (secondLedIntervalMS < SECOND_LED_MAX)
      {
        secondLedIntervalMS += SECOND_LED_INCREMENTOR;
      }
      while (xSemaphoreTake(mutex, 0) == pdFALSE) {}
      addOutput(SECOND_BUTTON_UP_PRESSED);
      xSemaphoreGive(mutex);
    }
  }
  else
  {
    secondButtonUpState = BUTTON_IDLE;
  }
  vTaskDelay(pdMS_TO_TICKS(THIRD_TASK_DELAY_MS));
}

void FourthTask(void *parameters)  // check if SECOND_BUTTON_DOWN is pressed
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
      if (secondLedIntervalMS > SECOND_LED_MIN)
      {
        secondLedIntervalMS -= SECOND_LED_INCREMENTOR;
      }
      while (xSemaphoreTake(mutex, 0) == pdFALSE) {}
      addOutput(SECOND_BUTTON_DOWN_PRESSED);
      xSemaphoreGive(mutex);
    }
  }
  else
  {
    secondButtonDownState = BUTTON_IDLE;
  }
  vTaskDelay(pdMS_TO_TICKS(FOURTH_TASK_DELAY_MS));
}

void FifthTask(void *parameters)  // output
{
  if (hasNewString)
  {
    while (xSemaphoreTake(mutex, 0) == pdFALSE) {}
    clear_lcd();
    for (int i = 0; i < LCD_Y; ++i)
    {
      lcd->setCursor(0, i);
      printf(outputStrings[i]);
    }

    hasNewString = false;
    xSemaphoreGive(mutex);
  }
  vTaskDelay(pdMS_TO_TICKS(FIFTH_TASK_DELAY_MS));
}

Task tasks[numOfTasks] = 
{
  FirstTask,
  SecondTask,
  ThirdTask,
  FourthTask,
  FifthTask
};

int placeOfTaskToBlock = 1;  // SecondTask should be blocked by FirstTask

const char* const taskNames[numOfTasks] = 
{
  "FirstTask",
  "SecondTask",
  "ThirdTask",
  "FourthTask",
  "FifthTask"
};

int *taskWords[numOfTasks]=
{
  768, 768, 768, 768, 768
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

  mutex = xSemaphoreCreateMutex();
  
  for (int i = 0; i < numOfTasks; ++i)
  {
    xTaskCreate(tasks[i],
      taskNames[i],
      taskWords[i],
      NULL,  // input parameters for tasks
      1,  // same prior
      (i == placeOfTaskToBlock ? BlockTaskHandle : NULL));  // TaskHandler (to suspend - resume)
  }
}

void loop(){ }
