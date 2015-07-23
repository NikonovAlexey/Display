/*
 DISPLAY - common function for menu-on-display

 */

#include "Arduino.h"
#include "LCD4884.h"
#include "Display.h"

Display::Display(MenuItem* menuItems, int menuLen, int backlightState) {
  this->backlightState = backlightState;
  this->joyPin = defJoyPin;
  this->delayCoeff = blinkDelay;
  
  this->menuPoint = 0;
  this->menuOffset = 0;
  this->menuState = MENU_MOVE;
  this->menuItems = menuItems;
  this->menuLength = this->menuCountLength();
  
  pinMode(lightPin, OUTPUT);
  
  // Включаем дисплей  
  lcd.LCD_init();
  lcd.LCD_clear();
  
  backlightOn();
}

/***************************************************
 */
// Включить подсветку дисплея
void Display::backlightOn() {
  this->backlightState = HIGH;
  digitalWrite(lightPin, this->backlightState);
}

// Выключить подсветку дисплея
void Display::backlightOff() {
  this->backlightState = LOW;
  digitalWrite(lightPin, this->backlightState);
}

// Переключить подсветку дисплея
void Display::backlightSwitch() {
  if ( this->backlightState == LOW ) {
    this->backlightState = HIGH;
  } else {
    this->backlightState = LOW;
  }
  digitalWrite(lightPin, this->backlightState);
}

// Помигать подсветкой дисплея count раз
void Display::backlightSwitchCount(int count) {
  int oldstate = this->backlightState;
  this->backlightState = HIGH;
  for (int a = 0; a < count; a++) {
    backlightSwitch();
    delay(delayCoeff);
    backlightSwitch();
    delay(delayCoeff);
  }
  this->backlightState = oldstate;
  digitalWrite(lightPin, oldstate);  
}

/***************************************************
 */
// Расшифровать состояние джойстика
int Display::joystickGetState() {
  int z;
  z = analogRead(joyPin);
  if ( z == 0 )  { return JOYSTICK_LEFT; };
  if ( z < 150 ) { return JOYSTICK_PRESS; };
  if ( z < 350 ) { return JOYSTICK_DOWN; };
  if ( z < 510 ) { return JOYSTICK_RIGHT; };
  if ( z < 750 ) { return JOYSTICK_UP; };
  if ( z >= 750 ) { return JOYSTICK_OFF; };
}

// Ожидание нажатия клавиши джойстика
void Display::joystickWaitKey(int key) {
  do {
    delay(delayCoeff);
  } while ( key != joystickGetState() );
  delay(delayCoeff);
}

// Обработка нажатия на клавишу джойстика
void Display::joystickProcessKey(int key) {
  int currentStep;
  boolean currentState;
  if ( key == JOYSTICK_OFF ) {
    return;
  }
  currentStep = this->menuItems[this->menuPoint].maxstep;
  currentState = this->menuGetState() == MENU_CHANGE ? true : false;
  switch (key) {
  case JOYSTICK_UP:
    if ( currentState ) {
      this->menuChangeValue(currentStep);
    } else {
      this->menuPreviousItem();
    }
    break;
  case JOYSTICK_DOWN:
    if ( currentState ) {
      this->menuChangeValue(-currentStep);
    } else {
      this->menuNextItem();
    }
    break;
  case JOYSTICK_LEFT:
    if ( currentState ) {
      this->menuChangeValue(-1);
    } else {
      this->menuFirstItem();
    }
    break;
  case JOYSTICK_RIGHT:
    if ( currentState ) {
      this->menuChangeValue(1);
    } else {
      this->menuLastItem();      
    }
    break;
  case JOYSTICK_PRESS:
    // Существует несколько типов меню, однако обрабатывать мы должны
    // только следующие: MENU_SUBMENU, MENU_PARAMETER, MENU_ACTION
    // MENU_STOP разделяет группы меню.
    // MENU_STUB описывает заглушку с типовым действием (пискнуть, помигать).
    switch ( this->menuItems[this->menuPoint].type ) {
      case MENU_PARAMETER:
        this->menuChangeState();
        break;
      case MENU_SUBMENU:
        this->menuJumpSubmenu(this->menuItems[this->menuPoint].currval);
        break;
      case MENU_ACTION:
        this->clear(5);
        this->say(0, 2, processLine, MENU_NORMAL);
        this->menuItems[this->menuPoint].callback();
        this->clear(5);
        this->menuOut();
      default:
        this->backlightSwitchCount(1);
        break;
    }
    
    break;
  }
}

/***************************************************
 */
// Очистить всё на экране
void Display::clear() {
  lcd.LCD_clear();
}

// Очистить всё на экране после паузы
void Display::clear(int pause) {
  delay(blinkDelay * pause);
  lcd.LCD_clear();
}

// Стереть линию на экране
void Display::clearLine(int x, int y) {
  lcd.LCD_write_string(x, y, emptyLine, MENU_NORMAL);  
}

// Вывести сообщение
void Display::say(int x, int y, char *message, int style) {
  clearLine(x, y);
  lcd.LCD_write_string(x, y, message, style);
}

// Вывести сообщение с параметром
void Display::say(int x, int y, char *message, int value, int style) {
  char buffer[maxValLength];
  String msg = String(message);
  int length = msg.length();
  
  itoa(value, buffer, 10);
  clearLine(x, y);
  if ( this->menuState == MENU_CHANGE ) {
    lcd.LCD_write_string(x, y, message, MENU_NORMAL);
  } else {
    lcd.LCD_write_string(x, y, message, style);
  }
  lcd.LCD_write_string(x + length * charWidth, y, buffer, style);
}

void Display::welcome() {
  // Показываем приветствие
  this->say(6, 2, welcomeLine, MENU_NORMAL);
  // Ждём и очищаем экран
  this->clear(5);
}

/***************************************************
 */
// Вернуться к предыдущему пункту меню
void Display::menuPreviousItem() {
  this->menuPoint--;
  if ( this->menuPoint < this->menuOffset ) { 
    this->menuPoint = this->menuOffset + this->menuLength - 1; 
  }
}

// Перейти к следующему пункту меню
void Display::menuNextItem() {
  this->menuPoint++;
  if ( this->menuPoint > this->menuOffset + this->menuLength - 1 ) {
    this->menuPoint = this->menuOffset; 
  }
}

// Вернуться к первому пункту меню
void Display::menuFirstItem() {
  this->menuPoint = this->menuOffset; 
}

// Перейти к последнемуы пункту меню
void Display::menuLastItem() {
  this->menuPoint = this->menuOffset + this->menuLength - 1; 
}

// Изменить значение параметра текущей строчки меню
void Display::menuChangeValue(int stepChange) {
  int currval = this->menuItems[this->menuPoint].currval;
  currval += stepChange;
  if ( currval > this->menuItems[this->menuPoint].maxval ) {
    currval = this->menuItems[this->menuPoint].maxval;
    this->backlightSwitchCount(1);
  }
  if ( currval < this->menuItems[this->menuPoint].minval ) {
    currval = this->menuItems[this->menuPoint].minval;
    this->backlightSwitchCount(1);
  }
  this->menuItems[this->menuPoint].currval = currval;
}

// Посчитать количество пунктов меню до следующего разделителя
int Display::menuCountLength() {
  int vPointer = this->menuPoint;
  int counter = 0;
  
  while ( this->menuItems[vPointer].type != MENU_STOP ) {
    vPointer++;
    counter++;
    if ( counter > 24 ) {
      return 0;
    };
  };
  
  return counter;
}

// Получить текущее состояние меню (навигация / изменение параметра)
int Display::menuGetState() {
  return this->menuState;
}

// Изменить статус меню (начать изменение параметра / вернуться к навигации)
void Display::menuChangeState() {
  if ( this->menuItems[this->menuPoint].type == MENU_PARAMETER ) {
    if ( this->menuState == MENU_CHANGE ) {
      this->menuState = MENU_MOVE;
    } else {
      this->menuState = MENU_CHANGE;
    }
  } else {
    this->menuState = MENU_MOVE;
  }
}

// Перейти в подменюы
void Display::menuJumpSubmenu(int firstSubmenuItem) {
  this->menuPoint = firstSubmenuItem;
  this->menuOffset = firstSubmenuItem;
  this->menuLength = this->menuCountLength();
  this->clear(1);
}

// Вывести меню на экран
void Display::menuOut() {
  int mFirstItem = this->menuOffset;
  int mLastItem = this->menuOffset + this->menuLength;
  int mLength = this->menuLength;
  int dLines = displayLines;
  int i;
  int z = 0;
  
  if ( dLines > mLength ) ( dLines = mLength );
  
  mFirstItem = this->menuPoint - dLines/2;
  mLastItem = this->menuPoint + dLines/2;
  if ( mFirstItem < this->menuOffset ) { mFirstItem = this->menuOffset; mLastItem = mFirstItem + dLines; };
  if ( mLastItem > this->menuOffset + this->menuLength ) { mLastItem = this->menuOffset + this->menuLength; mFirstItem = mLastItem - dLines; };
  if ( dLines >= mLength ) { mFirstItem = this->menuOffset; mLastItem = this->menuOffset + this->menuLength; };
  
  for ( i = mFirstItem ; i <= mLastItem; i++ ) {
    if ( this->menuItems[i].type == MENU_PARAMETER ) {
      say(0, z, this->menuItems[i].name, this->menuItems[i].currval, i == this->menuPoint ? MENU_HIGHLIGHT : MENU_NORMAL );
    } else {
      say(0, z, this->menuItems[i].name, i == this->menuPoint ? MENU_HIGHLIGHT : MENU_NORMAL );
    }
    z++;
  }
}



