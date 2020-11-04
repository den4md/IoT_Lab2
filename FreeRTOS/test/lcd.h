#include <LiquidCrystal.h>

int LCD_X;
int LCD_Y;

LiquidCrystal *lcd;

void clear_lcd(const char* fill_char = " ")
{
  for (int i = 0; i < LCD_Y; ++i)
  {
    lcd->setCursor(0, i);
    for (int j = 0; j < LCD_X; ++j)
    {
      printf(fill_char);
    }
  }
  lcd->setCursor(0, 0);
}

void begin_lcd(LiquidCrystal *l, int x, int y)
{
  lcd = l;
  lcd->begin(x, y);
  LCD_X = x;
  LCD_Y = y;
  clear_lcd();
}

void lcd_put_char(char ch, FILE *f)
{
  lcd->print(ch);
}
