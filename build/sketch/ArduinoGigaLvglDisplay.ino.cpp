#include <Arduino.h>
#line 1 "C:\\Users\\Snapp\\OneDrive\\Arduino\\Projects\\ArduinoGigaLvglDisplay\\ArduinoGigaLvglDisplay.ino"
/**
 * @file ArduinoGigaLvglDisplay.ino
 * @author Jadon Jung (jadonjung3@gmail.com)
 * @brief Arduino Giga LVGL Display For Robot Car
 * @version 0.1
 * @date 2025-11-30
 *
 * @copyright Copyright (c) 2025
 *
 */

/// @brief Include necessary libraries
#include <Arduino_GigaDisplayTouch.h>

// #include <lv_api_map_v8.h>
#include <lv_api_map_v9_0.h>
#include <lv_api_map_v9_1.h>
#include <lv_api_map_v9_2.h>
#include <lv_api_map_v9_3.h>
#include <lv_conf_internal.h>
#include <lv_conf_kconfig.h>
#include <lv_init.h>
#include <lvgl_private.h>
#include <lvgl.h>

#include "lvgl.h"
#include "Arduino_GigaDisplayTouch.h"
#include "Arduino_H7_Video.h"
#include <vector>

/// @brief Initialize display and touch detector
Arduino_H7_Video Display(800, 480, GigaDisplayShield);
Arduino_GigaDisplayTouch TouchDetector;

/// @brief Message related
lv_obj_t *msg_label = NULL;
unsigned long msg_start_time = 0;
const unsigned long msg_duration = 1500;

/// @brief Constants for buttons
const int OFFSET = 180;
const int WIDTH = 500;  // Width of main buttons
const int HEIGHT = 400; // Height of main buttons

/// @brief Screens
lv_obj_t *screen_buttons;

/// @brief Motor stuff
int motor_speed;
int motor_direction;

/// @brief Motor pins
int ENB_B = 13;
int IN4_B = 12;
int IN3_B = 11;
int IN2_B = 10;
int IN1_B = 9;
int ENA_B = 8;

int ENB_A = 7;
int IN4_A = 6;
int IN3_A = 5;
int IN2_A = 4;
int IN1_A = 3;
int ENA_A = 2;

/// @brief Program storage
std::vector<const char *> program;

// ===== Shared label + vector logic =====
/**
 * @brief
 *
 * @param mode_name mode of movement
 * @param direction
 */
#line 77 "C:\\Users\\Snapp\\OneDrive\\Arduino\\Projects\\ArduinoGigaLvglDisplay\\ArduinoGigaLvglDisplay.ino"
static void apply_mode(const char *mode_name, const char *direction);
#line 115 "C:\\Users\\Snapp\\OneDrive\\Arduino\\Projects\\ArduinoGigaLvglDisplay\\ArduinoGigaLvglDisplay.ino"
static void btn_Run_Program_event_cb(lv_event_t *e);
#line 320 "C:\\Users\\Snapp\\OneDrive\\Arduino\\Projects\\ArduinoGigaLvglDisplay\\ArduinoGigaLvglDisplay.ino"
void set_btnm_bg_colors(lv_obj_t *btnm, uint32_t normal, uint32_t pressed);
#line 328 "C:\\Users\\Snapp\\OneDrive\\Arduino\\Projects\\ArduinoGigaLvglDisplay\\ArduinoGigaLvglDisplay.ino"
void create_ui();
#line 422 "C:\\Users\\Snapp\\OneDrive\\Arduino\\Projects\\ArduinoGigaLvglDisplay\\ArduinoGigaLvglDisplay.ino"
void FR_move(int speed);
#line 443 "C:\\Users\\Snapp\\OneDrive\\Arduino\\Projects\\ArduinoGigaLvglDisplay\\ArduinoGigaLvglDisplay.ino"
void FL_move(int speed);
#line 464 "C:\\Users\\Snapp\\OneDrive\\Arduino\\Projects\\ArduinoGigaLvglDisplay\\ArduinoGigaLvglDisplay.ino"
void RR_move(int speed);
#line 485 "C:\\Users\\Snapp\\OneDrive\\Arduino\\Projects\\ArduinoGigaLvglDisplay\\ArduinoGigaLvglDisplay.ino"
void RL_move(int speed);
#line 502 "C:\\Users\\Snapp\\OneDrive\\Arduino\\Projects\\ArduinoGigaLvglDisplay\\ArduinoGigaLvglDisplay.ino"
void setup();
#line 514 "C:\\Users\\Snapp\\OneDrive\\Arduino\\Projects\\ArduinoGigaLvglDisplay\\ArduinoGigaLvglDisplay.ino"
void loop();
#line 77 "C:\\Users\\Snapp\\OneDrive\\Arduino\\Projects\\ArduinoGigaLvglDisplay\\ArduinoGigaLvglDisplay.ino"
static void apply_mode(const char *mode_name, const char *direction)
{
  if (msg_label != NULL)
  {
    lv_obj_del(msg_label);
    msg_label = NULL;
  }

  msg_label = lv_label_create(lv_scr_act());

  char buffer[64];
  snprintf(buffer, sizeof(buffer), "Applied %s Mode: %s.", mode_name, direction);
  lv_label_set_text(msg_label, buffer);

  program.push_back(direction);

  lv_obj_set_style_text_font(msg_label, &lv_font_montserrat_24, 0);
  lv_obj_align(msg_label, LV_ALIGN_BOTTOM_MID, 0, -20);
  msg_start_time = millis();
}

// ===== Run Program (special) =====
/**
 * @brief Handles the "Run Program" button event.
 *
 * This callback is triggered when the user presses the **Prgm Car** button
 * in the button matrix. It displays a temporary "Applied program!" message
 * on the screen, prints all stored program steps to the serial monitor,
 * clears the program list, and records the timestamp so the message can
 * be removed later.
 *
 * The function also deletes any previous message label before creating
 * a new one.
 *
 * @param e Pointer to the LVGL event descriptor. Contains information
 *          about the event and the object that triggered it, although
 *          this callback does not use it directly.
 */
static void btn_Run_Program_event_cb(lv_event_t *e)
{
  if (msg_label != NULL)
  {
    lv_obj_del(msg_label);
    msg_label = NULL;
  }

  msg_label = lv_label_create(lv_scr_act());
  lv_label_set_text(msg_label, "Applied program!");

  for (auto p : program)
  {
    Serial.println(p);
  }
  Serial.println();

  delay(500);
  program.clear();

  lv_obj_set_style_text_font(msg_label, &lv_font_montserrat_24, 0);
  lv_obj_align(msg_label, LV_ALIGN_BOTTOM_MID, 0, -20);
  msg_start_time = millis();
}

// ======== CALLBACK DECLARATIONS ========
static void btnm_event_cb(lv_event_t *e);
static void test_btnm_event_cb(lv_event_t *e);
static void slider_event_cb(lv_event_t *e);
static void toggle_fwdrev_cb(lv_event_t *e);

// ======== MAIN BUTTON CALLBACK ========
/**
 * @brief Handles all main directional and mode selection button events.
 *
 * This callback is triggered whenever a button in the primary button matrix
 * is pressed. It reads the button's text label and maps it to a specific
 * movement mode (Straight, Sideways, Diagonal, Pivot, Pivot Sideways,
 * Rotate). The corresponding mode and direction string are then forwarded
 * to apply_mode().
 *
 * If the selected button is "Prgm Car", this function instead triggers the
 * special program execution callback btn_Run_Program_event_cb().
 *
 * @param e Pointer to the LVGL event descriptor containing details about
 *          the button press event (e.g., target object).
 */
static void btnm_event_cb(lv_event_t *e)
{
  lv_obj_t *btnm = (lv_obj_t *)lv_event_get_target(e);
  const char *txt = lv_btnmatrix_get_btn_text(
      btnm, lv_btnmatrix_get_selected_btn(btnm));

  if (txt == NULL)
    return;

  if (strcmp(txt, "Forward") == 0 || strcmp(txt, "Backward") == 0)
  {
    apply_mode("Straight", txt);
  }
  else if (strcmp(txt, "Left") == 0 || strcmp(txt, "Right") == 0)
  {
    apply_mode("Sideways", txt);
  }
  else if (strcmp(txt, "45") == 0 || strcmp(txt, "135") == 0 || strcmp(txt, "225") == 0 || strcmp(txt, "315") == 0)
  {
    apply_mode("Diagonal", txt);
  }
  else if (strcmp(txt, "Right Fwd") == 0 || strcmp(txt, "Right Bwd") == 0 || strcmp(txt, "Left Fwd") == 0 || strcmp(txt, "Left Bwd") == 0)
  {
    apply_mode("Pivot", txt);
  }
  else if (strcmp(txt, "Front Right") == 0 || strcmp(txt, "Front Left") == 0 || strcmp(txt, "Rear Right") == 0 || strcmp(txt, "Rear Left") == 0)
  {
    apply_mode("Pivot Sideways", txt);
  }
  else if (strcmp(txt, "CCW") == 0 || strcmp(txt, "CW") == 0)
  {
    apply_mode("Rotate", txt);
  }
  else if (strcmp(txt, "Prgm Car") == 0)
  {
    btn_Run_Program_event_cb(e);
  }
}

// ======== test CONTROL BUTTON CALLBACK ========
/**
 * @brief Handles individual test control button events.
 *
 * Triggered when a test control button (FL, FR, RL, RR) is pressed.
 * This callback identifies which test was selected and calls the
 * appropriate test movement function using the current global
 * speed and direction values.
 *
 * @param e Pointer to the LVGL event descriptor. Used to determine
 *          which button in the test control matrix was pressed.
 */
static void test_btnm_event_cb(lv_event_t *e)
{
  lv_obj_t *btnm = (lv_obj_t *)lv_event_get_target(e);
  const char *txt = lv_btnmatrix_get_btn_text(
      btnm, lv_btnmatrix_get_selected_btn(btnm));

  if (txt == NULL)
    return;

  if (strcmp(txt, "FL") == 0)
  {
    Serial.println("Front Left test Move");
    FL_move(motor_speed * motor_direction);
  }
  else if (strcmp(txt, "FR") == 0)
  {
    Serial.println("Front Right test Move");
    FR_move(motor_speed * motor_direction);
  }
  else if (strcmp(txt, "RL") == 0)
  {
    Serial.println("Rear Left test Move");
    RL_move(motor_speed * motor_direction);
  }
  else if (strcmp(txt, "RR") == 0)
  {
    Serial.println("Rear Right test Move");
    RR_move(motor_speed * motor_direction);
  }

  delay(500);
}

// ======== FWD/REV TOGGLE CALLBACK ========
/**
 * @brief Handles toggling between Forward and Reverse.
 *
 * When the toggle button is pressed, this callback swaps the
 * label between "Fwd" and "Rev" and updates the global direction
 * variable accordingly:
 *   - "Fwd" → direction = -1
 *   - "Rev" → direction = 1
 *
 * A message is printed to the serial monitor showing the new mode
 * and numeric direction.
 *
 * @param e Pointer to the LVGL event descriptor. Provides access
 *          to the toggle button object.
 */
static void toggle_fwdrev_cb(lv_event_t *e)
{
  lv_obj_t *btn = (lv_obj_t *)lv_event_get_target(e);
  lv_obj_t *label = lv_obj_get_child(btn, 0);

  const char *txt = lv_label_get_text(label);

  if (strcmp(txt, "Fwd") == 0)
  {
    lv_label_set_text(label, "Rev");
    motor_direction = 1;
  }
  else
  {
    lv_label_set_text(label, "Fwd");
    motor_direction = -1;
  }

  Serial.print("Mode: ");
  Serial.println(lv_label_get_text(label));
  Serial.print("Direction set to ");
  Serial.println(motor_direction);
}

// ======== SLIDER CALLBACK ========
/**
 * @brief Updates the global speed variable when a slider is moved.
 *
 * This callback is triggered whenever the user interacts with a
 * speed slider. It reads the slider's current value and assigns it
 * to the global speed variable.
 *
 * If user data is attached to the event (typically a string naming
 * the slider), that label is printed to the serial monitor for clarity.
 *
 * @param e Pointer to the LVGL event descriptor. Used to access the
 *          slider object and optional user data.
 */
static void slider_event_cb(lv_event_t *e)
{
  lv_obj_t *slider = (lv_obj_t *)lv_event_get_target(e);
  motor_speed = lv_slider_get_value(slider);

  // Identify which slider this is (optional, if you name them)
  const char *name = (const char *)lv_event_get_user_data(e);
  Serial.print(name);
  Serial.print(" speed set to ");
  Serial.println(motor_speed);
}

// ======== UI CREATION FUNCTION ========
/**
 * @brief Set the btnm bg colors object
 *
 * @param btnm button map object
 * @param normal color at normal state
 * @param pressed color at pressed state
 */
void set_btnm_bg_colors(lv_obj_t *btnm, uint32_t normal, uint32_t pressed)
{
  lv_obj_set_style_bg_color(btnm, lv_color_hex(normal), LV_PART_ITEMS);
  lv_obj_set_style_bg_opa(btnm, LV_OPA_COVER, LV_PART_ITEMS);
  lv_obj_set_style_bg_color(btnm, lv_color_hex(pressed), LV_PART_ITEMS | LV_STATE_PRESSED);
  lv_obj_set_style_bg_opa(btnm, LV_OPA_COVER, LV_PART_ITEMS | LV_STATE_PRESSED);
}

void create_ui()
{
  screen_buttons = lv_obj_create(NULL);

  // ===== Main Button Matrix =====
  static const char *btnm_map[] = {
      "Forward", "Backward", "Left", "Right", "\n",
      "45", "135", "225", "315", "\n",
      "Right Fwd", "Right Bwd", "Left Fwd", "Left Bwd", "\n",
      "Front Right", "Front Left", "Rear Right", "Rear Left", "\n",
      "CCW", "CW", "Prgm Car", ""};

  lv_obj_t *btnm = lv_btnmatrix_create(screen_buttons);
  lv_btnmatrix_set_map(btnm, btnm_map);
  lv_obj_set_size(btnm, 460, 300);
  lv_obj_align(btnm, LV_ALIGN_TOP_LEFT, 10, 0);

  set_btnm_bg_colors(btnm, 0x008800, 0xe74c3c);

  lv_obj_set_style_text_color(btnm, lv_color_hex(0xffffff), LV_PART_ITEMS);

  lv_obj_add_event_cb(btnm, btnm_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

  // ===== test Test Button Matrix =====
  static const char *test_btnm_map[] = {"FL", "FR", "\n", "RL", "RR", ""};
  lv_obj_t *test_btnm = lv_btnmatrix_create(screen_buttons);
  lv_btnmatrix_set_map(test_btnm, test_btnm_map);

  lv_obj_set_size(test_btnm, 200, 120);
  lv_obj_align(test_btnm, LV_ALIGN_BOTTOM_LEFT, 20, -20);

  set_btnm_bg_colors(test_btnm, 0x5555ff);

  lv_obj_add_event_cb(test_btnm, test_btnm_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

  // ===== Speed Sliders =====
  const char *labels[] = {"FR", "FL", "RR", "RL"};
  int y_positions[] = {20, 60, 100, 140};

  for (int i = 0; i < 4; i++)
  {
    // Create label
    lv_obj_t *label = lv_label_create(screen_buttons);
    lv_label_set_text(label, labels[i]);
    lv_obj_align(label, LV_ALIGN_TOP_RIGHT, -230, y_positions[i]);

    // Create slider
    lv_obj_t *slider = lv_slider_create(screen_buttons);
    lv_slider_set_range(slider, 0, 255);
    lv_obj_set_size(slider, 150, 20);
    lv_obj_align_to(slider, label, LV_ALIGN_OUT_RIGHT_MID, 15, 0);

    // Create value label
    lv_obj_t *value_label = lv_label_create(screen_buttons);
    char buf[4];
    snprintf(buf, sizeof(buf), "%d", lv_slider_get_value(slider));
    lv_label_set_text(value_label, buf);
    lv_obj_align_to(value_label, slider, LV_ALIGN_OUT_RIGHT_MID, 15, 0);

    // Add event callback that updates the value label
    lv_obj_add_event_cb(
        slider, [](lv_event_t *e)
        {
        lv_obj_t* s = (lv_obj_t*)lv_event_get_target(e);
        lv_obj_t* val_lbl = (lv_obj_t*)lv_event_get_user_data(e);
        char buf[4];
        snprintf(buf, sizeof(buf), "%d", lv_slider_get_value(s));
        lv_label_set_text(val_lbl, buf); },
        LV_EVENT_VALUE_CHANGED, value_label);

    // Add slider event callback to update speed
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, (void *)labels[i]);
  }

  // ===== Fwd/Rev Toggle Button =====
  lv_obj_t *toggle_FwdRev = lv_btn_create(screen_buttons);
  lv_obj_set_size(toggle_FwdRev, 80, 40);
  lv_obj_align(toggle_FwdRev, LV_ALIGN_TOP_RIGHT, -20, 180);

  lv_obj_t *toggle_label = lv_label_create(toggle_FwdRev);
  lv_label_set_text(toggle_label, "Fwd");
  lv_obj_center(toggle_label);

  // Set direction
  lv_obj_add_event_cb(toggle_FwdRev, toggle_fwdrev_cb, LV_EVENT_CLICKED, NULL);
}

// ===== test movement functions =====

/**
 * @brief Moves the front-right (FR) motor at the specified speed.
 *
 * @param speed Speed of movement (-255 to 255)
 */
void FR_move(int speed)
{
  if (speed >= 0)
  {
    digitalWrite(IN1_B, HIGH);
    digitalWrite(IN2_B, LOW);
    analogWrite(ENA_B, speed);
  }
  else
  {
    digitalWrite(IN1_B, LOW);
    digitalWrite(IN2_B, HIGH);
    analogWrite(ENA_B, -speed);
  }
}

/**
 * @brief Moves the front-left (FL) motor at the specified speed.
 *
 * @param speed Speed of movement (-255 to 255)
 */
void FL_move(int speed)
{
  if (speed >= 0)
  {
    digitalWrite(IN3_B, HIGH);
    digitalWrite(IN4_B, LOW);
    analogWrite(ENB_B, speed);
  }
  else
  {
    digitalWrite(IN3_B, LOW);
    digitalWrite(IN4_B, HIGH);
    analogWrite(ENB_B, -speed);
  }
}

/**
 * @brief Moves the rear-right (RR) motor at the specified speed.
 *
 * @param speed Speed of movement (-255 to 255)
 */
void RR_move(int speed)
{
  if (speed >= 0)
  {
    digitalWrite(IN1_A, HIGH);
    digitalWrite(IN2_A, LOW);
    analogWrite(ENA_A, speed);
  }
  else
  {
    digitalWrite(IN1_A, LOW);
    digitalWrite(IN2_A, HIGH);
    analogWrite(ENA_A, -speed);
  }
}

/**
 * @brief Moves the rear-left (RL) motor at the specified speed.
 *
 * @param speed Speed of movement (-255 to 255)
 */
void RL_move(int speed)
{
  if (speed >= 0)
  {
    digitalWrite(IN3_A, HIGH);
    digitalWrite(IN4_A, LOW);
    analogWrite(ENB_A, speed);
  }
  else
  {
    digitalWrite(IN3_A, LOW);
    digitalWrite(IN4_A, HIGH);
    analogWrite(ENB_A, -speed);
  }
}

// ===== Setup & loop =====
void setup()
{
  delay(3000);
  Serial.begin(115200);

  Display.begin();
  TouchDetector.begin();

  create_ui();
  lv_scr_load(screen_buttons);
}

void loop()
{
  lv_timer_handler();

  if (msg_label != NULL && (millis() - msg_start_time > msg_duration))
  {
    lv_obj_del(msg_label);
    msg_label = NULL;
  }
}

