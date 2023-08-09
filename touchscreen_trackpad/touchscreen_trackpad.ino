#include "LCD_Test.h"
#include <Mouse.h>

#define DARK_GRAY 0x18E3

#define DEBUG_CONSOLE true  // also edit in DEC_Config.cpp

void Touch_INT_callback();
bool touch_flag = false;
unsigned int time_touch = 0;
unsigned int old_time_touch = 0;
unsigned int old_time = 0;
unsigned int time_first_touch = 0;
uint16_t old_x, old_y;
float mouse_speed = 1;
unsigned int click_speed = 95;
int coasting_percent = 70;
bool clicked = false;
bool was_moved = false;
bool coast = false;
UWORD *BlackImage;
UDOUBLE Imagesize = LCD_1IN28_HEIGHT * LCD_1IN28_WIDTH * 2;

const int touchpad_radius = 120;  // Radius of the circular touchpad
const int scrolling_edge = 10;    // Width of the scrolling edge
const int scrolling_deadzone = 60;
const int scroll_segments = 45;
const int coast_speed = 10;
const float coasting_ratio = 0.99;
float coast_delta_x = 0;
float coast_delta_y = 0;
bool is_scrolling = false;
bool in_dead_zone = false;
float angle = 0;
int detent = 0;
int old_detent = 0;

void get_scroll_angle(void) {
  // Scroll based on circular motion within the scrolling edge
  float angle_rad = atan2(Touch_CTS816.y_point - 120, Touch_CTS816.x_point - 120);  // Calculate angle in radians

  // Convert radians to degrees
  // float angle_deg = angle_rad * 180.0 / M_PI;

  // Ensure angle is in the range [0, 360)
  if (angle_rad < 0) {
    angle_rad += 2 * M_PI;
  }
  // detent = angle_rad;
  detent = map(angle_rad * 1000, 0, 2 * M_PI * 1000, 0, scroll_segments);
}

void setup() {
  if (DEV_Module_Init() != 0) {
    if (DEBUG_CONSOLE) {
      Serial.println("GPIO Init Fail!");
    }
  }
  // Serial.println("GPIO Init successful!");
  LCD_1IN28_Init(HORIZONTAL);
  DEV_SET_PWM(0);
  LCD_1IN28_Clear(WHITE);
  DEV_SET_PWM(20);
  if ((BlackImage = (UWORD *)malloc(Imagesize)) == NULL) {
    if (DEBUG_CONSOLE) {
      Serial.println("Failed to apply for black memory...");
    }
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
  attachInterrupt(Touch_INT_PIN, &Touch_INT_callback, RISING);
  DEV_SET_PWM(20);
  Mouse.begin();

  LCD_1IN28_Display(BlackImage);
  Paint_Clear(BLACK);
  Paint_DrawCircle(120, 120, touchpad_radius - scrolling_edge, DARK_GRAY, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);
  Paint_DrawCircle(120, 120, touchpad_radius - scrolling_deadzone, DARK_GRAY, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);
  LCD_1IN28_Display(BlackImage);
  old_time = millis();
  // Serial.print("Setup done.");
}

void loop() {
  if (touch_flag) {  // touch event detected
    time_touch = millis();
    unsigned int delta_time_touch = time_touch - old_time_touch;
    CST816S_Get_Point();
    int distance_from_center = sqrt(sq(120 - Touch_CTS816.x_point) + sq(120 - Touch_CTS816.y_point));
    if (delta_time_touch >= 20) {
      first_touch(distance_from_center);
    } else {
      if (is_scrolling && distance_from_center >= touchpad_radius - scrolling_deadzone) {  //checks if in scrolling mode and not too close to the center
        scrolling();
      } else if (is_scrolling) { // in the dead zone
        in_dead_zone = true;
      } else if (!is_scrolling) {  // mouse move
        mouse_move();
      }
    }
    old_x = Touch_CTS816.x_point;
    old_y = Touch_CTS816.y_point;
    touch_flag = false;
    old_time_touch = time_touch;
  } else {  // no touch flag
    if (coast) {
      coasting();
    } else if (millis() - old_time_touch >= 20 && millis() - time_first_touch <= click_speed) {  // at least one cycle untouched & if tap within click speed
      if (!was_moved && !clicked) {
        if (DEBUG_CONSOLE) {
          Serial.println("CLICK!");
        }
        Mouse.press(MOUSE_LEFT);
        delay(10);
        Mouse.release(MOUSE_LEFT);
        clicked = true;
      }
    }
  }
  DEV_Delay_us(1000);
}

void Touch_INT_callback() {
  touch_flag = true;
}