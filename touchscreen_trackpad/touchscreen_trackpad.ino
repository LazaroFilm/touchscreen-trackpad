#include "LCD_Test.h"
#include <Mouse.h>

#define DARK_GRAY 0x18E3
void Touch_INT_callback();
bool touch_flag = false;
unsigned int time_touch = 0;
unsigned int old_time_touch = 0;
unsigned int old_time = 0;
unsigned int time_first_touch = 0;
uint16_t old_x, old_y;
float mouse_speed = 1;
unsigned int click_speed = 90;
int delta_x = 0;
int delta_y = 0;
int coasting_percent = 70;
bool clicked = false;
bool was_moved = false;
UWORD *BlackImage;
UDOUBLE Imagesize = LCD_1IN28_HEIGHT * LCD_1IN28_WIDTH * 2;

void setup() {
  if (DEV_Module_Init() != 0)
    Serial.println("GPIO Init Fail!");
  else
    // Serial.println("GPIO Init successful!");
    LCD_1IN28_Init(HORIZONTAL);
  DEV_SET_PWM(0);
  LCD_1IN28_Clear(WHITE);
  DEV_SET_PWM(100);
  if ((BlackImage = (UWORD *)malloc(Imagesize)) == NULL) {
    Serial.println("Failed to apply for black memory...");
    exit(0);
  }
  Paint_NewImage((UBYTE *)BlackImage, LCD_1IN28.WIDTH, LCD_1IN28.HEIGHT, 0, WHITE);
  Paint_SetScale(65);
  Paint_Clear(WHITE);
  Paint_SetRotate(ROTATE_0);
  Paint_Clear(WHITE);
  // QMI8658_init();
  CST816S_init(CST816S_Point_Mode);
  DEV_KEY_Config(Touch_INT_PIN);
  while (!digitalRead(Touch_INT_PIN)) {
    Serial.println("stop touching!");
  }
  attachInterrupt(Touch_INT_PIN, &Touch_INT_callback, RISING);
  // Serial.println("QMI8658_init\r\n");
  DEV_SET_PWM(50);
  Mouse.begin();


  LCD_1IN28_Display(BlackImage);


  Paint_Clear(DARK_GRAY);
  // Paint_DrawRectangle(0, 00, 240, 47, 0X2595, DOT_PIXEL_2X2, DRAW_FILL_FULL);
  Paint_DrawString_EN(56, 25, "- touchpad -", &Font16, WHITE, WHITE);
  LCD_1IN28_Display(BlackImage);
  old_time = millis();
}

void loop() {
  // if (millis() - old_time >= 1000) {  // heartbeat <3
  //   Serial.print(" - - - ");
  //   Serial.print(millis() / 1000);
  //   Serial.println(" - - - ");
  //   old_time = millis();
  // }

  if (touch_flag) {  // touch event detected
    // Serial.print(clicked);
    // Serial.print(was_moved);
    // Serial.print(" | ");
    time_touch = millis();
    unsigned int delta_time_touch = time_touch - old_time_touch;
    CST816S_Get_Point();
    if (delta_time_touch >= 20) {  // first touch
      time_first_touch = time_touch;
      clicked = false;
      was_moved = false;
    } else {  // touching
      delta_x = Touch_CTS816.x_point - old_x;
      delta_y = Touch_CTS816.y_point - old_y;
      // Serial.print(delta_x);
      // Serial.print(" | ");
      // Serial.println(delta_y);
      if (delta_x || delta_y) {  // moving
        was_moved = true;
        Mouse.move(delta_x * mouse_speed, delta_y * mouse_speed, 0);
      }
    }
    old_x = Touch_CTS816.x_point;
    old_y = Touch_CTS816.y_point;
    touch_flag = false;
    old_time_touch = time_touch;
  } else {                                               // no touch flag
    if (millis() - old_time_touch >= 20) {               // at least one cycle untouched.
      if (millis() - time_first_touch <= click_speed) {  // if tap within click speed
        if (!was_moved && !clicked) {
          // Serial.println("CLICK!");
          Mouse.press(MOUSE_LEFT);
          delay(10);
          Mouse.release(MOUSE_LEFT);
          clicked = true;
        }
      }
    }
  }
  DEV_Delay_us(1000);
}


    /*TODO:
    - Some coasting when the last move value was high and keep that direction and slowly decelerate until touched again.
    - Two finger detection and scroll (with coasting too)
    - Two finger right click
    - Tap and hold / drag
    - For later: Screen animation matching the movements? Maybe use second core for this so it doesn't slow the mouse?
      (maybe dragging a retro pink grid or dots to save sprite size to simulate a ball rolling, maybe useful for the coasting)
    */



void Touch_INT_callback() {
  touch_flag = true;
}