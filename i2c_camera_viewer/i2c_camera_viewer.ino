#include <M5Stack.h>

#define CAMERA1 0x61
#define CAMERA2 0x62
#define JOY_ADDR 0x52
#define pagesize 100
size_t len_1;
size_t width_1;
size_t height_1;
size_t len_2;
size_t width_2;
size_t height_2;
uint8_t b0, b1, b2, b3;
uint8_t rec_len;
uint8_t error;
uint8_t page = 0;
uint8_t maxpage = 0;
uint8_t buffer1[20000];
uint8_t buffer2[20000];
int buf_idx;
uint8_t res_id = 2;
int offset;
char data[100];
uint8_t delay_frame[] = { 10, 40, 80 };
unsigned long oldtime, newtime = 0;
void setup() {
  M5.begin();
  M5.Power.begin();
  Wire.setClock(1000000);
  Wire.begin();

  M5.Lcd.setTextFont(2);
  M5.Lcd.println("Camera test - master");
  M5.Lcd.println(Wire.getClock());
}

void send_config(uint8_t value) {
  Wire.beginTransmission(CAMERA1);
  Wire.write(value);
  error = Wire.endTransmission();
  Wire.beginTransmission(CAMERA2);
  Wire.write(value);
  error = Wire.endTransmission();
}
void check_joy() {

  static uint8_t x_data, y_data, button_data;
  Wire.requestFrom(JOY_ADDR, 3);
  if (Wire.available()) {
    x_data = Wire.read();
    y_data = Wire.read();
    button_data = Wire.read();
  }
  sprintf(data, "x:%d y:%d button:%d\n", x_data, y_data, button_data);
  Serial.print(data);
  if (x_data < 90) {
    send_config(10);
    delay(200);
  };

  if (x_data > 155) {
    send_config(9);
    delay(200);
  };

  if (y_data < 90) {
    send_config(7);
    delay(200);
  };
  if (y_data > 155) {
    send_config(8);
    delay(200);
  };

  if (button_data == 1) {
    res_id++;
    if (res_id > 2) res_id = 0;
    send_config(res_id + 3);
    delay(200);
  };
}

void get_metadata() {
  Wire.beginTransmission(CAMERA1);
  Wire.write(1);
  error = Wire.endTransmission();
  delay(2);
  Serial.print("Error: ");
  Serial.println(error);
  Serial.println("requesting metadata");

  rec_len = Wire.requestFrom(CAMERA1, 12);

  Serial.print("Request sent, received: ");
  Serial.print(rec_len);
  Serial.print(" | Available: ");
  Serial.println(Wire.available());
  // while (Wire.available()) {
  b0 = Wire.read();
  b1 = Wire.read();
  b2 = Wire.read();
  b3 = Wire.read();
  width_1 = b0 << 24 | b1 << 16 | b2 << 8 | b3;
  b0 = Wire.read();
  b1 = Wire.read();
  b2 = Wire.read();
  b3 = Wire.read();
  height_1 = b0 << 24 | b1 << 16 | b2 << 8 | b3;
  b0 = Wire.read();
  b1 = Wire.read();
  b2 = Wire.read();
  b3 = Wire.read();
  len_1 = b0 << 24 | b1 << 16 | b2 << 8 | b3;
  // camera 2
  Wire.beginTransmission(CAMERA2);
  Wire.write(1);
  error = Wire.endTransmission();
  delay(2);
  Serial.print("Error: ");
  Serial.println(error);
  Serial.println("requesting metadata");

  rec_len = Wire.requestFrom(CAMERA2, 12);

  Serial.print("Request sent, received: ");
  Serial.print(rec_len);
  Serial.print(" | Available: ");
  Serial.println(Wire.available());
  // while (Wire.available()) {
  b0 = Wire.read();
  b1 = Wire.read();
  b2 = Wire.read();
  b3 = Wire.read();
  width_2 = b0 << 24 | b1 << 16 | b2 << 8 | b3;
  b0 = Wire.read();
  b1 = Wire.read();
  b2 = Wire.read();
  b3 = Wire.read();
  height_2 = b0 << 24 | b1 << 16 | b2 << 8 | b3;
  b0 = Wire.read();
  b1 = Wire.read();
  b2 = Wire.read();
  b3 = Wire.read();
  len_2 = b0 << 24 | b1 << 16 | b2 << 8 | b3;
}

void loop() {
  check_joy();
  get_metadata();
  //}
  while (Wire.available()) {
    Wire.read();
  }
  if ((len_1 > 1000) && (len_1 < 20000)) {
    Serial.println("Img Ok");
    // get frame
    maxpage = ((uint8_t)(len_1 / pagesize)) + 1;
    page = 0;
    buf_idx = 0;
    Wire.beginTransmission(CAMERA1);
    Wire.write(2);
    Wire.endTransmission();
    delay(2);

    for (int j = 0; j < maxpage; j++) {
      Wire.requestFrom(CAMERA1, pagesize);
      while (Wire.available()) {
        buffer1[buf_idx++] = Wire.read();
      }
    }
  } else {
    // try to reset the image
    Wire.beginTransmission(CAMERA1);
    Wire.write(0);
    error = Wire.endTransmission();
    Serial.println("Error, sending reset");
    delay(200);
  }
  // *******************************************
  // camera2

  //}
  while (Wire.available()) {
    Wire.read();
  }
  if ((len_2 > 1000) && (len_2 < 20000)) {
    Serial.println("Img Ok");
    // get frame
    maxpage = ((uint8_t)(len_2 / pagesize)) + 1;
    page = 0;
    buf_idx = 0;
    Wire.beginTransmission(CAMERA2);
    Wire.write(2);
    Wire.endTransmission();
    delay(2);

    for (int j = 0; j < maxpage; j++) {
      Wire.requestFrom(CAMERA2, pagesize);
      while (Wire.available()) {
        buffer2[buf_idx++] = Wire.read();
      }
    }
    M5.Lcd.drawJpg(buffer1, len_1, 0, 0);

    M5.Lcd.drawJpg(buffer2, len_2, 161, 0);
    M5.Lcd.drawFastVLine(160, 0, 240, 0xF800);
    delay(delay_frame[res_id]);
    Serial.print("Time: ");
    Serial.println(newtime-oldtime);
    oldtime = newtime;
    newtime = micros();
    //delay(2000);
  } else {
    Wire.beginTransmission(CAMERA2);
    Wire.write(0);
    error = Wire.endTransmission();
    Serial.println("Error, sending reset");
    delay(200);
  }
}
