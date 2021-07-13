/*
 * fitness_timer.ino   v1.0
 * weigu.lu
 * LOLIN (Wemos) D1 mini pro with Wemos OLED display (new version with buttons)
 * Rotary encoder: on D5, D6, D7
 * use forked lib for OLED display on https://github.com/weigu1/Adafruit_SSD1306_Wemos_OLED
 * Use forked encoder lib from here: https://github.com/weigu1/Encoder
 * Download zip files and add with "Sketch -> Include Library -> Add .ZIP Library" 
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306_Wemos_OLED.h>
#include <LOLIN_I2C_BUTTON.h>
#include <Encoder_Wemos.h>

const byte PROGRAMS_NR = 5;
// Here are the programs declared; -1 = default, 1 = B1, 2 = B2, 0 not assigned
int programs[PROGRAMS_NR][4] = { {1,300,15,-1}, 
                                 {3,45,15,1},
                                 {5,30,15,2},
                                 {2,45,15,0},
                                 {2,30,15,0},                                 
};

// global variables for program and play_program_sound(n);
byte play_program_counter = 0, play_program_case = 0;
unsigned long delay_prev_time_1 = 0;
unsigned long delay_prev_time_2 = 0;
byte current_program = 0;

//------- Encoder -------
const byte PIN_ENC_A = 14;  // D5 (SCK)
const byte PIN_ENC_B = 12;  // D6 (MISO)
const byte PIN_ENC_PB = 13; // D7 (MOSI)
Encoder enc(PIN_ENC_A, PIN_ENC_B);      // create encoder object
int enc_position = -999;
int new_enc_position; 
byte encoder_value;

//------- Display -------
#define OLED_RESET -1
Adafruit_SSD1306 display(OLED_RESET);
String Display_line[PROGRAMS_NR];
I2C_BUTTON button(DEFAULT_I2C_BUTTON_ADDRESS); //I2C Address 0x31
String Key_String[] = {"None", "Press", "Long", "Double", "Hold"};
int change_buttons_new_value;

//------- Piezo -------
const byte PIN_PIEZO = 2;  // D4

void setup(void) {
  Serial.begin(115200);
  init_display();
  delay(1000);
  Serial.println("\nFitness Timer\n");    
  pinMode(PIN_PIEZO, OUTPUT);  
  pinMode(PIN_ENC_PB, INPUT_PULLUP);  
  sprint_program_info();  
}

void loop(void) {  
  change_button_assignment();
  if (button.get() == 0) { // Button press
    if (button.BUTTON_B) { // B1
      for (byte i=0; i<PROGRAMS_NR; i++) {
        if (programs[i][3] == 1) {
          play_program_case = 0;
          current_program = i;
        }
      }
      sprint_program_info();
      Serial.println("Beginning in 5 seconds");
      write_display(current_program);  
      delay(5000); // to get in position :)
    }
    if (button.BUTTON_A) { // B2
      for (byte i=0; i<PROGRAMS_NR; i++) {
        if (programs[i][3] == 2) {
          play_program_case = 0;
          current_program = i;
        }
      }
      sprint_program_info();
      Serial.println("Beginning in 5 seconds");
      write_display(current_program);  
      delay(5000); // to get in position :)
    }
    
  }  
  write_display(current_program);  
  play_program_sound(current_program);  
}

void init_display() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(4, 8);
  display.setTextColor(WHITE); // call sensors.requestTemperatures() to issue a global temperature
  display.println("FITNESS");
  display.setCursor(20, 30);
  display.println("TIMER");
  display.display();
}  

void sprint_program_info() { 
  Serial.print("Playing program nr " + String(current_program) + ":  ");
  Serial.print(String(programs[current_program][0]) + " times " + String(programs[current_program][1]));
  Serial.println("s with " + String(programs[current_program][2]) + " s pause");
}

void change_button_assignment() { 
  if (!digitalRead(PIN_ENC_PB)) {
    delay(1000);
    while (digitalRead(PIN_ENC_PB)) {      
      get_encoder(encoder_value, 0, PROGRAMS_NR-1); // get rotary;
      Serial.println(encoder_value);
      write_display(encoder_value);
      change_buttons_new_value = programs[encoder_value][3];
      if (button.get() == 0) { // Button press
        if (button.BUTTON_A) {
          change_buttons_new_value = 2;
          Serial.println("B2: " + Key_String[button.BUTTON_A]);
        }
        if (button.BUTTON_B) {
          change_buttons_new_value = 1;
          Serial.println("B1: " + Key_String[button.BUTTON_B]);
        }
      }  
      for (byte i=0; i<PROGRAMS_NR; i++) {
        if (change_buttons_new_value == 1) {
          if ((programs[i][3] == change_buttons_new_value) && (programs[i][3] != 2)) {            
            programs[i][3] = 0;
          }
        }
        else { // change_buttons_new_value = 2)
          if ((programs[i][3] == change_buttons_new_value) && (programs[i][3] != 1)) {
            programs[i][3] = 0;
          }
        }
      }
      programs[encoder_value][3] = change_buttons_new_value;
    }
    delay(1000);    
  }
}  

void play_program_sound(byte program_nr) { 
  unsigned long millis_now;
  switch (play_program_case) {
    case 0:  // first call      
      tone(PIN_PIEZO, 440, 200); // 440 Hz, 200 ms             
      play_program_counter = programs[program_nr][0];
      play_program_case = 1;
      delay_prev_time_1 = millis();
      break;
    case 1: // 1 delay
      millis_now = millis();
      if((millis_now - delay_prev_time_1) >= programs[program_nr][1]*1000) {
        play_program_counter--;
        if (play_program_counter == 0) {
          for (byte i=0; i<3; i++) {
            tone(PIN_PIEZO, 880, 200); // 1460 Hz, 200 ms
            delay(200+100);
          }  
        }
        else {
          tone(PIN_PIEZO, 880, 500); // 880 Hz, 500 ms
        }
        play_program_case = 2;
        delay_prev_time_2 = millis();
      }          
      break;
    case 2: // 2 delay
      millis_now = millis();
      if((millis_now - delay_prev_time_2) >= programs[program_nr][2]*1000) {
        tone(PIN_PIEZO, 440, 200); // 440 Hz, 200 ms            
        if (play_program_counter==0) {
          play_program_case = 0;
        }
        else {
          play_program_case = 1;
          delay_prev_time_1 = millis();
        }            
      }        
      break;
  }
}

void write_display(byte item) {
  display.clearDisplay();
  display.setCursor(0, 0);
  for (byte i=0; i<PROGRAMS_NR; i++) {
    Display_line[i] = String(programs[i][0]) + "x" + String(programs[i][1]);
    if (programs[i][1] <100) {
      Display_line[i] += " "; 
    }
    Display_line[i] += (" " + String(programs[i][2]) + " ");
    if (programs[i][3] == 1) {
      Display_line[i] += "1"; 
    }
    if (programs[i][3] == 2) {
      Display_line[i] += "2"; 
    }
    display.setCursor(0,(i*9)+1);
    if (i == item) { 
      display.fillRect(0,i*9,64,9,WHITE);
      display.setTextColor(BLACK);
    }
    else {
      display.fillRect(0,i*9,64,9,BLACK);
      display.setTextColor(WHITE);
    }      
    display.print(Display_line[i]);
  }  
  display.display();
}

//------ Rotary Encoder ------
// flag is set if encoder value changes!
void get_encoder(byte &value, byte value_min, byte value_max) {
  new_enc_position = enc.read();
  if (new_enc_position/4 != enc_position) {    
    if (new_enc_position <= value_min*4) {
      new_enc_position = value_min*4;  
      enc.write(value_min*4);
    }  
    if (new_enc_position >= value_max*4) {
      new_enc_position = value_max*4;  
      enc.write(value_max*4);
    }
    enc_position = new_enc_position/4;
    value = enc_position;
  delayMicroseconds(10000); //10ms
  }
}
