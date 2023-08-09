void first_touch(int distance_from_center) {
  time_first_touch = time_touch;
  clicked = false;
  was_moved = false;
  coast = false;
  if (distance_from_center >= touchpad_radius - scrolling_edge) {  // check if tapped on scroll area
    if (DEBUG_CONSOLE) {
      Serial.println("Scrolling started...");
    }
    get_scroll_angle();
    old_detent = detent;
    is_scrolling = true;
  } else {
    is_scrolling = false;
  }
}

void scrolling(void) {
  get_scroll_angle();
  if (!in_dead_zone) {
    if (detent - old_detent == 1) {
      Mouse.move(0, 0, -1);
    } else if (detent - old_detent == -1) {
      Mouse.move(0, 0, 1);
    } else if (old_detent == scroll_segments - 1 && detent == 0) {
      Mouse.move(0, 0, -1);
    } else if (detent == scroll_segments - 1 && old_detent == 0) {
      Mouse.move(0, 0, 1);
    }
    if (DEBUG_CONSOLE && detent != old_detent) {
      Serial.print(detent);
      Serial.print(" Scrolling... ");
      Serial.println();
    }
  }
  old_detent = detent;
  in_dead_zone = false;
}

void mouse_move(void) {
  signed char delta_x = Touch_CTS816.x_point - old_x;
  signed char delta_y = Touch_CTS816.y_point - old_y;
  if (delta_x || delta_y) {
    if (DEBUG_CONSOLE) {
      Serial.print(Touch_CTS816.x_point);
      Serial.print(" | ");
      Serial.print(Touch_CTS816.y_point);
      Serial.print(" || ");
      Serial.print(delta_x);
      Serial.print(" | ");
      Serial.print(delta_y);
      Serial.print(" moving...");
      Serial.println();
    }
    was_moved = true;
    Mouse.move(delta_x * mouse_speed, delta_y * mouse_speed, 0);
    if (sqrt(sq(delta_x) + sq(delta_y)) >= coast_speed) {  //magnitude of move vector
      coast = true;
      coast_delta_x = delta_x;
      coast_delta_y = delta_y;
    } else {
      coast = false;
    }
  }
}

void coasting(void) {
  while (coast && !touch_flag) {
    coast_delta_x *= coasting_ratio;
    coast_delta_y *= coasting_ratio;
    Mouse.move(coast_delta_x * mouse_speed, coast_delta_y * mouse_speed, 0);
    if (DEBUG_CONSOLE) {
      Serial.print(coast_delta_x);
      Serial.print(" | ");
      Serial.print(coast_delta_x);
      Serial.println(" coasting...");
    }
    if (abs(coast_delta_x) < 1 && abs(coast_delta_y) < 1) {  // if coasting is over
      coast = false;
    }
  }
  coast = false;
}