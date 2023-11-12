#include "esp_camera.h"
#include <Wire.h>
// ===================
// Select camera model
// ===================
//#define CAMERA_MODEL_WROVER_KIT // Has PSRAM
//#define CAMERA_MODEL_ESP_EYE // Has PSRAM
//#define CAMERA_MODEL_ESP32S3_EYE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_PSRAM // Has PSRAM
//#define CAMERA_MODEL_M5STACK_V2_PSRAM // M5Camera version B Has PSRAM
#define CAMERA_MODEL_M5STACK_WIDE  // M5Camera  //Has PSRAM
//#define CAMERA_MODEL_M5STACK_ESP32CAM // No PSRAM
//#define CAMERA_MODEL_M5STACK_UNITCAM // M5 Unit Camera DIY Kit // No PSRAM
//#define CAMERA_MODEL_AI_THINKER  // Has PSRAM
//#define CAMERA_MODEL_TTGO_T_JOURNAL // No PSRAM
//#define CAMERA_MODEL_XIAO_ESP32S3 // Has PSRAM
// ** Espressif Internal Boards **
//#define CAMERA_MODEL_ESP32_CAM_BOARD
//#define CAMERA_MODEL_ESP32S2_CAM_BOARD
//#define CAMERA_MODEL_ESP32S3_CAM_LCD
//#define CAMERA_MODEL_DFRobot_FireBeetle2_ESP32S3 // Has PSRAM
//#define CAMERA_MODEL_DFRobot_Romeo_ESP32S3 // Has PSRAM
#include "camera_pins.h"

unsigned long timing1, timing2;
bool flag = false;
bool buff_ready = false;
uint8_t send_buff[12];
uint8_t cmd;
uint8_t page = 0;
uint8_t maxpage = 0;
uint32_t i = 0;
camera_fb_t *fb = NULL;
uint8_t res_id = 2;
int pan = 100;
int tilt = 20;
int x_max[] = { 1600, 800, 400 };
int y_max[] = { 1200, 600, 296 };
#define address 0x62
#define pagesize 100
void receiveEvent(int howMany) {
  while (Wire.available() > 0) {
    cmd = uint8_t(Wire.read());
  }
  if (cmd > 2) set_window();
  Serial.println(cmd);
  buff_ready = false;
  return;
}
void send_32(int input, int offset) {
  uint8_t buffer[4];
  send_buff[0 + offset] = (uint8_t)(input >> 24);
  send_buff[1 + offset] = (uint8_t)((input >> 16) & 0xff);
  send_buff[2 + offset] = (uint8_t)((input >> 8) & 0xff);
  send_buff[3 + offset] = (uint8_t)((input)&0xff);
}

void send_32b(size_t input) {
  Wire.write(uint8_t(int(input) >> 24));
  Wire.write(uint8_t((int(input) >> 16) & 0xff));
  Wire.write(uint8_t((int(input) >> 8) & 0xff));
  Wire.write(uint8_t((int(input)) & 0xff));
}
void send_16b(size_t input) {
  Wire.write(uint8_t((int(input) >> 8) & 0xff));
  Wire.write(uint8_t((int(input)) & 0xff));
}


void requestEvent() {
  //Serial.println("Sending Buffer");
  if (cmd == 2) { buff_ready = false; };
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();


  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  Wire.begin(address, 4, 13, 1000000);
  //Wire.begin(address, 17, 16, 1000000);

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_QVGA;
  config.pixel_format = PIXFORMAT_JPEG;  // for streaming
  //config.pixel_format = PIXFORMAT_GRAYSCALE; // for face detection/recognition
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_DRAM;
  config.jpeg_quality = 10;
  config.fb_count = 1;
  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
sensor_t *s = esp_camera_sensor_get();
  // int res = s->set_res_raw(s, startX, startY, endX, endY, offsetX, offsetY, totalX, totalY, outputX, outputY, scale, binning);
  s->set_res_raw(s, res_id, 0, 0, 0, pan, tilt, 160, 240, 160, 240, true, false);
#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif
  fb = esp_camera_fb_get();
}
void set_window() {
  sensor_t *s = esp_camera_sensor_get();
  switch (cmd) {
    case 3:
      res_id = 2;
      pan = (x_max[res_id] / 2) - 80;
      tilt = (y_max[res_id] / 2) - 120;
      break;
    case 4:
      res_id = 1;
      pan = (x_max[res_id] / 2) - 80;
      tilt = (y_max[res_id] / 2) - 120;
      break;
    case 5:
      res_id = 0;
      pan = (x_max[res_id] / 2) - 80;
      tilt = (y_max[res_id] / 2) - 120;
      break;
    case 6:
      pan = (x_max[res_id] / 2) - 80;
      tilt = (y_max[res_id] / 2) - 120;
      break;
    case 7:
      tilt += 20;
      if (tilt > (y_max[res_id] - 240)) tilt = y_max[res_id] - 240;
      break;
    case 8:
      tilt -= 20;
      if (tilt < 0) tilt = 0;
      break;
    case 9:
      pan += 20;
      if (pan > (x_max[res_id] - 160)) pan = x_max[res_id] - 160;
      break;
    case 10:
      pan -= 20;
      if (pan < 0) pan = 0;
      break;
  }
  s->set_res_raw(s, res_id, 0, 0, 0, pan, tilt, 160, 240, 160, 240, true, false);
  Serial.print("Res_id: ");
  Serial.print(res_id);
  Serial.print(" | Pan: ");
  Serial.print(pan);
  Serial.print(" | Tilt: ");
  Serial.println(tilt);
  get_frame();
}

void get_frame() {
  timing1 = micros();
  esp_camera_fb_return(fb);
  fb = esp_camera_fb_get();
  maxpage = ((uint8_t)(fb->len / pagesize)) + 1;
  page = 0;
  timing2 = micros();
  // Serial.println(fb->len);
  Serial.print("Width: ");
  Serial.print(fb->width);
  Serial.print(" | Height: ");
  Serial.print(fb->height);
  Serial.print(" | Len : ");
  Serial.print(fb->len);
  Serial.print(" | required ns: ");
  Serial.println(timing2 - timing1);
}

void loop() {
  if (!buff_ready) {
    Wire.flush();
    switch (cmd) {
      case 1:  // Metadata
        send_32(fb->width, 0);
        send_32(fb->height, 4);
        send_32(fb->len, 8);
        Wire.slaveWrite((uint8_t *)send_buff, 12);
        page = 0;
        break;
      case 2:  // Real data
        Wire.slaveWrite(fb->buf + (page * pagesize), pagesize);
        if (page < maxpage) {
          page++;
        } else {
          get_frame();
        }
        break;
      case 0:
        get_frame();
        Serial.println("Received reset request");
        break;
      default:

        break;
    }
    buff_ready = true;
  }
}
