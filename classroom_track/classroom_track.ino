#include <Arduino_MKRIoTCarrier.h>
MKRIoTCarrier carrier;

const int pir_pin = A5;
bool pir_state = LOW;
bool door_state = false;
bool password = false;
int num_person = 0;
int password_code[] = {2, 4, 1, 3, 4};
int password_empty[] = {0, 0, 0, 0, 0};
int standby[] = {0, 0, 0, 0, 0};
int index_value = 0;

float gyro_x;
float gyro_y;
float gyro_z;

uint32_t colorRed = carrier.leds.Color(0, 255, 0);
uint32_t colorBlue = carrier.leds.Color(0, 0, 255);
uint32_t colorGreen = carrier.leds.Color(255, 0, 0);
uint32_t colorOrange = carrier.leds.Color(240, 240, 0);
uint32_t colorOff = carrier.leds.Color(0,0,0);

String message_door = "The door is being open !";
String message_someone = "Someone's  inside!";
String message_checkin = "Someone's just checked inside..";
String message_code = "Please type the password. Wait for buzz";
String message_welcome = "Welcome to the jungle";
String message_wrong_code = "Wrong password";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while(!Serial);
  pinMode(pir_pin, INPUT);
  CARRIER_CASE = false;
  carrier.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  carrier.Buttons.update();
  pir_state = digitalRead(pir_pin);
  carrier.IMUmodule.readGyroscope(gyro_x, gyro_y, gyro_z);
  //Serial.print(gyro_x);
  //Serial.print(gyro_y);
  //Serial.println(gyro_z);
  input_code();
  background();
  if (array_equal(password_empty, password_code) & index_value == 7) {
    background();
    message_display(colorGreen, message_welcome, false);
    password = true;
    if (carrier.Buttons.getTouch(TOUCH0)) {
     background();
     memset(password_empty, 0, sizeof(password_empty));
     index_value = 0;
     for (int i = 0; i < sizeof(password_empty); i++) {
      Serial.print(password_empty[i]);
     }
     password = false;
    }
  } else if (index_value == 7 & !password) {
     background();
     message_display(colorRed, message_wrong_code, false);
     memset(password_empty, 0, sizeof(password_empty));
     index_value = 0;
  } else if (!password) {
      background();
      if (gyro_x < 0 & gyro_y < 0) {
        message_display(colorBlue, message_door, false);
        door_state = true;
        if (pir_state == HIGH) {
          background();
          message_display(colorRed, message_someone, false);
          num_person++;
          buzzer_alarm();
          password = true;
        } else {
          background();
          message_display(colorGreen, message_checkin, false);
      } 
    } else {
      message_display(colorOrange, message_code, true); 
    }
  } 
} 

void message_display(uint32_t color, String message, bool sInput) {
  if (sInput) {
    for(int i = 0; i < 5; i++)
    {
      carrier.display.print(password_empty[i]);
    }
    carrier.display.println("");
  }
  carrier.display.println(message);
  carrier.leds.fill((color), 0, 1);
  carrier.leds.show();
  delay(3000);
}
void background() {
  carrier.display.fillScreen(ST77XX_BLACK);
  carrier.display.setCursor(10, 50);
  carrier.display.setTextSize(3);
  carrier.display.println("");
  carrier.leds.fill((colorOff), 0, 1);
  carrier.leds.show();
}
void buzzer_alarm() {
  for (int j = 0; j < 3; j++) {
    for (int i = 200; i < 1500; i += 10) {
      carrier.Buzzer.sound(i);
      delay(6);
    }
    for (int k = 1500; k < 200; k -= 10) {
      carrier.Buzzer.sound(k);
      delay(3);
    }
  }
  carrier.Buzzer.noSound();
}
void input_code() {
  for (int i = 0; i < sizeof(password_empty); i++) {
   if (password_empty[i] == 0) {
    index_value = i;
    break;
   }
  }
  if (carrier.Buttons.getTouch(TOUCH0)) {
    password_empty[index_value] = 5;
    mini_buzz();
    delay(300);
  }
  else if (carrier.Buttons.getTouch(TOUCH1)) {
    password_empty[index_value] = 1;
    mini_buzz();
    delay(300);
  }
  else if (carrier.Buttons.getTouch(TOUCH2)) {
    password_empty[index_value] = 2;
    mini_buzz();
    delay(300);
  }
  else if (carrier.Buttons.getTouch(TOUCH3)) {
    password_empty[index_value] = 3;
    mini_buzz();
    delay(300);
  }
  else if (carrier.Buttons.getTouch(TOUCH4)) {
    password_empty[index_value] = 4;
    mini_buzz();
    delay(300);
  }
}
bool array_equal(int array1[], int array2[]) {
  for (int i = 0; i < sizeof(array1); i++) {
    if (array1[i] != array2[i]) {
      return false;
    }
  }
  return true;
}
void mini_buzz() {
  //carrier.Buzzer.sound(600);
  //delay(100);
  carrier.Buzzer.noSound();
}
