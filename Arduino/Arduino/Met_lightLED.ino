#include <Arduino.h>

#include "TrinketHidCombo.h"
#include <WS2812.h>

#define PIN_ENCODER_A 0
#define PIN_ENCODER_B 2
#define TRINKET_PINx  PINB
#define PIN_ENCODER_SWITCH 1
#define LED_PIN 5
#define NUMPIXELS 7

static uint8_t enc_prev_pos   = 0;
static uint8_t enc_flags      = 0;
static char    sw_was_pressed = 0;

WS2812 LED(NUMPIXELS);
cRGB value;

int currentPosition = 3;

const uint8_t PROGMEM gamma8[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };
  
void setup()
{
  // set pins as input with internal pull-up resistors enabled
  pinMode(PIN_ENCODER_A, INPUT);
  pinMode(PIN_ENCODER_B, INPUT);
  digitalWrite(PIN_ENCODER_A, HIGH);
  digitalWrite(PIN_ENCODER_B, HIGH);

  pinMode(PIN_ENCODER_SWITCH, INPUT);
  // the switch is active-high, not active-low
  // since it shares the pin with Trinket's built-in LED
  // the LED acts as a pull-down resistor
  digitalWrite(PIN_ENCODER_SWITCH, LOW);

  TrinketHidCombo.begin(); // start the USB device engine and enumerate
  
  LED.setOutput(5);
  
  // get an initial reading on the encoder pins
  if (digitalRead(PIN_ENCODER_A) == LOW) {
    enc_prev_pos |= (1 << 0);
  }
  if (digitalRead(PIN_ENCODER_B) == LOW) {
    enc_prev_pos |= (1 << 1);
  }

  updateRing();

}

void updatePosition(int next){
  currentPosition = currentPosition + next;
  if(currentPosition < 0)
    currentPosition = currentPosition + 7;
  if(currentPosition > 6)
    currentPosition = currentPosition - 7;
  updateRing();
}

byte calcPosition(byte current, int next){
  int r = current + next;
  if(r < 0)
    r = r + 7;
  if(r > 6)
    r = r - 7;
  return r;
}

void updateRing(){
  value.b = pgm_read_byte(&gamma8[200]); value.g = pgm_read_byte(&gamma8[200]); value.r = pgm_read_byte(&gamma8[200]);
  LED.set_crgb_at(currentPosition, value);
  value.b = pgm_read_byte(&gamma8[85]); value.g = pgm_read_byte(&gamma8[85]); value.r = pgm_read_byte(&gamma8[85]);
  LED.set_crgb_at(calcPosition(currentPosition, -1), value); 
  LED.set_crgb_at(calcPosition(currentPosition, +1), value);
  value.b = pgm_read_byte(&gamma8[40]); value.g = pgm_read_byte(&gamma8[40]); value.r = pgm_read_byte(&gamma8[40]); 
  LED.set_crgb_at(calcPosition(currentPosition, -2), value);  
  LED.set_crgb_at(calcPosition(currentPosition, +2), value);
  value.b = 0; value.g = 0; value.r = 0; 
  LED.set_crgb_at(calcPosition(currentPosition, -3), value);
  LED.set_crgb_at(calcPosition(currentPosition, +3), value);
  LED.sync();
}

void loop(){

  
  int8_t enc_action = 0; // 1 or -1 if moved, sign is direction

  // note: for better performance, the code will now use
  // direct port access techniques
  // http://www.arduino.cc/en/Reference/PortManipulation
  uint8_t enc_cur_pos = 0;
  // read in the encoder state first
  if (bit_is_clear(TRINKET_PINx, PIN_ENCODER_A)) {
    enc_cur_pos |= (1 << 0);
  }
  if (bit_is_clear(TRINKET_PINx, PIN_ENCODER_B)) {
    enc_cur_pos |= (1 << 1);
  }

  // if any rotation at all
  if (enc_cur_pos != enc_prev_pos)
  {
    if (enc_prev_pos == 0x00)
    {
      // this is the first edge
      if (enc_cur_pos == 0x01) {
        enc_flags |= (1 << 0);
      }
      else if (enc_cur_pos == 0x02) {
        enc_flags |= (1 << 1);
      }
    }

    if (enc_cur_pos == 0x03)
    {
      // this is when the encoder is in the middle of a "step"
      enc_flags |= (1 << 4);
    }
    else if (enc_cur_pos == 0x00)
    {
      // this is the final edge
      if (enc_prev_pos == 0x02) {
        enc_flags |= (1 << 2);
      }
      else if (enc_prev_pos == 0x01) {
        enc_flags |= (1 << 3);
      }

      // check the first and last edge
      // or maybe one edge is missing, if missing then require the middle state
      // this will reject bounces and false movements
      if (bit_is_set(enc_flags, 0) && (bit_is_set(enc_flags, 2) || bit_is_set(enc_flags, 4))) {
        enc_action = 1;
      }
      else if (bit_is_set(enc_flags, 2) && (bit_is_set(enc_flags, 0) || bit_is_set(enc_flags, 4))) {
        enc_action = 1;
      }
      else if (bit_is_set(enc_flags, 1) && (bit_is_set(enc_flags, 3) || bit_is_set(enc_flags, 4))) {
        enc_action = -1;
      }
      else if (bit_is_set(enc_flags, 3) && (bit_is_set(enc_flags, 1) || bit_is_set(enc_flags, 4))) {
        enc_action = -1;
      }

      enc_flags = 0; // reset for next time
    }
  }

  enc_prev_pos = enc_cur_pos;

  if (enc_action > 0) {
    TrinketHidCombo.pressMultimediaKey(MMKEY_VOL_DOWN);
    updatePosition(-1);
  }
  else if (enc_action < 0) {
    TrinketHidCombo.pressMultimediaKey(MMKEY_VOL_UP);
    updatePosition(1);
  }

  // remember that the switch is active-high
  if (bit_is_set(TRINKET_PINx, PIN_ENCODER_SWITCH)) 
  {
    if (sw_was_pressed == 0) // only on initial press, so the keystroke is not repeated while the button is held down
    {
      TrinketHidCombo.pressMultimediaKey(MMKEY_MUTE);
      value.b = pgm_read_byte(&gamma8[100]); value.g = pgm_read_byte(&gamma8[100]); value.r = pgm_read_byte(&gamma8[100]);
      for(byte i = 0; i < 7; i++){
        LED.set_crgb_at(i, value);
      }
      LED.sync();
      delay(5); // debounce delay
    }
    sw_was_pressed = 1;
  }
  else
  {
    if (sw_was_pressed != 0) {
      value.b = 0; value.g = 0; value.r = 0;
      for(byte i = 0; i < 7; i++){
        LED.set_crgb_at(i, value);
      }
      updateRing();
      LED.sync();
      delay(5); // debounce delay
    }
    sw_was_pressed = 0;
  }

  TrinketHidCombo.poll(); // check if USB needs anything done
}
