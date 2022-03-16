#include <WiFiNINA.h>
#include <ArduinoMqttClient.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <Arduino_MKRIoTCarrier.h>
#include "property.h"
MKRIoTCarrier carrier;

char ssid[] = SECRET_SSID;    
char pass[] = SECRET_PASS; 
int WiFistatus = WL_IDLE_STATUS;

const int pir_pin = A5;
bool pir_state = LOW;
bool door_state = false;
bool password = false;
bool greeting = true;
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
String message_someone = "Hello you !";
String message_checkin = "The person just looked inside..";
String message_someone_inside = "Someone is staying inside !";
String message_code = "Please type the password. Wait for buzz";
String message_welcome = "Welcome to the room !";
String message_wrong_code = "Wrong password";
String message_no_one = "No one can be seen..";
String message = "";

void callback(char* topic, byte* payload, unsigned int length);
WiFiClient wifiClient;
PubSubClient mqtt(MQTT_HOST, MQTT_PORT, callback, wifiClient);

StaticJsonDocument<100> jsonDoc;
JsonObject payload = jsonDoc.to<JsonObject>();
JsonObject status = payload.createNestedObject("d");
static char msg[100];

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] : ");
  payload[length] = 0; 
  Serial.println((char *)payload);
}

void setup() {
  Serial.begin(9600);
  while(!Serial);
  pinMode(pir_pin, INPUT);
  CARRIER_CASE = false;
  carrier.begin();
  while (WiFistatus != WL_CONNECTED) {
    Serial.print("Attempting to connect to network: ");
    Serial.println(ssid);
    WiFistatus = WiFi.begin(ssid, pass);
    delay(3000);
  }
  if (mqtt.connect(MQTT_DEVICEID, MQTT_USER, MQTT_TOKEN)) {
    Serial.println("MQTT Connected");
    mqtt.subscribe(MQTT_TOPIC_DISPLAY);
  } else {
    Serial.println("MQTT Failed to connect!");
  }
}
void loop() {
  mqtt.loop();
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqtt.connect(MQTT_DEVICEID, MQTT_USER, MQTT_TOKEN)) {
      Serial.println("MQTT Connected");
      mqtt.subscribe(MQTT_TOPIC_DISPLAY);
      mqtt.loop();
    } else {
      Serial.println("MQTT Failed to connect!");
      delay(5000);
    }
  }
  carrier.Buttons.update();
  if (pir_state == LOW) {
    pir_state = digitalRead(pir_pin);
  }
  carrier.IMUmodule.readGyroscope(gyro_x, gyro_y, gyro_z);
  input_code();
  background();
  Serial.println(pir_state);
  if (pir_state == HIGH) {
    if (greeting) {
      message_display(colorBlue, message_someone, false);
      message = message_someone;
      num_person++;
      delay(1000);
      greeting = false;
    }
    if (array_equal(password_empty, password_code) & index_value >= 4 ) {
      background();
      message_display(colorGreen, message_welcome, false);
      message = message_welcome;
      password = true;
      if (carrier.Buttons.getTouch(TOUCH0)) {
       background();
       memset(password_empty, 0, sizeof(password_empty));
       index_value = 0;
       password = false;
     }
    } else if (index_value >= 4 & !password) {
       background();
       message_display(colorRed, message_wrong_code, false);
       message = message_wrong_code;
       memset(password_empty, 0, sizeof(password_empty));
       index_value = 0;
    } else if (gyro_x < 0 & gyro_y < 0 & !door_state) {
        background();
        door_state = true;
        message_display(colorRed, message_door, false);
        message = message_door;
    } else if (door_state) {
        delay(6000);
        Serial.print("Time elapse: ");
        if (gyro_x < 0 & gyro_y < 0) {
          background();
          message_display(colorRed, message_someone_inside, false);
          message = message_someone_inside;
          buzzer_alarm();
        } else {
          background();
          message_display(colorGreen, message_checkin, false);
          message = message_checkin;
        }
        door_state = false;
    } else {
        background();
        message_display(colorOrange, message_code, true);
        message = message_code;
    } 
  } else {
    background();
    message_display(colorOrange, message_no_one, false);
    message = message_no_one;
  }
  //status["message"] = message;
  status["door_state"] = door_state;
  status["someone_detected"] = pir_state;
  status["gyroscope_x"] = gyro_x;
  status["num_person"] = num_person;
  serializeJson(jsonDoc, msg, 100);
  Serial.println(msg);
  delay(10000);
  if (!mqtt.publish(MQTT_TOPIC, msg)) {
    Serial.println("MQTT Publish failed");
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
  for (int j = 0; j < 2; j++) {
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
  carrier.Buzzer.sound(600);
  delay(100);
  carrier.Buzzer.noSound();
}
