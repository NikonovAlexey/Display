/* DISPLAY - common function for menu-on-display

Управление подсветкой дисплея
=============================

  backlightOn
Включить подсветку дисплеяы

  backlightOff
Выключить подсветку

  backlightSwitch
Переключить состояние подсветки на противоположное

  backlightSwitchCount(int N)
Переключить состояние подсветки N раз

Управление джойстиком
=====================

  getJoystickState
Получить состояние джойстика

 waitJoystickKey(key)
Ожидание нажатия клавиши джойстика


Управление меню
===============


 */

#ifndef Display_h
#define Display_h

#define defJoyPin  0
#define lightPin   7
#define blinkDelay 100

#define JOYSTICK_OFF   0
#define JOYSTICK_UP    1
#define JOYSTICK_DOWN  2
#define JOYSTICK_LEFT  3
#define JOYSTICK_RIGHT 4
#define JOYSTICK_PRESS 5

#define MENU_NORMAL 0
#define MENU_HIGHLIGHT 1

#define MENU_SUBMENU 0
#define MENU_PARAMETER 1
#define MENU_ACTION 2
#define MENU_STOP 3
#define MENU_STUB 4

#define MENU_MOVE 0
#define MENU_CHANGE 1

#define emptyLine "              "
#define welcomeLine   "Starting ..."
#define processLine   "Processing ..."

#define charWidth 6
#define maxValLength 14

#define displayLines 6

struct menuItem {
  int type;
  char* name;
  int currval;
  int minval;
  int maxval;
  int maxstep;
  void (*callback)();
};
typedef struct menuItem MenuItem;

class Display {
public:
  Display();
  Display(MenuItem* mitems, int menuLen, int blinkState);
  
  void backlightOn();
  void backlightOff();
  void backlightSwitch();
  void backlightSwitchCount(int count);
  
  int joystickGetState();
  void joystickWaitKey(int key);
  void joystickProcessKey(int key);
  
  void clear();
  void clear(int pause);
  void clearLine(int x, int y);
  void say(int x, int y, char *message, int style);
  void say(int x, int y, char *message, int value, int style);
  void welcome();
  
  //void menuInit();
  void menuPreviousItem();
  void menuNextItem();
  void menuFirstItem();
  void menuLastItem();
  void menuChangeValue(int stepChange);
  int  menuCountLength();
  int  menuGetState();
  void menuChangeState();
  void menuJumpSubmenu(int firstSubmenuItem);  
  void menuOut();
  
private:
  int delayCoeff;
  int backlightState;
  int joyPin;
  
  int menuPoint;
  int menuState;
  int menuLength;
  int menuOffset;
  
  MenuItem* menuItems;
};

#endif

