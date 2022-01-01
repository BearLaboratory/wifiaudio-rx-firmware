#include "AudioOut.h"
#include <WiFi.h>
const char *ssid = "BLabAudio";
const char *password = "dengyi1991";
IPAddress localIp(192, 168, 101, 2);
IPAddress gateWay(192, 168, 101, 1);
IPAddress subNet(255, 255, 255, 0);
//----------定义常量------------
#define OPEN_DEBUG true
#define uS_TO_S_FACTOR 1000000ULL
#define TIME_TO_SLEEP  600        //睡眠时间10分钟，到时会醒来检查一次，还是没电会继续睡
#define MUTE_OUT_PIN 32
#define TOUCH_THRESHOLD 40        //触摸灵敏度阈值，越大越灵敏
#define STATUS_PIN 12
#define MUTE_STATUS_PIN 14
#define ADC_PIN 34
// i2s配置
i2s_config_t i2sConfig = { .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
                           .sample_rate = 44100,
                           .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
                           .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
                           .communication_format =
                             i2s_comm_format_t(I2S_COMM_FORMAT_I2S),
                           .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
                           .dma_buf_count = 4,
                           .dma_buf_len = 512,
                           .use_apll = true,
                           .tx_desc_auto_clear = true,
                           .fixed_mclk = I2S_PIN_NO_CHANGE
                         };
// i2s pin配置
i2s_pin_config_t i2SPinConfig = { .bck_io_num = GPIO_NUM_27,
                                  .ws_io_num = GPIO_NUM_26,
                                  .data_out_num = GPIO_NUM_25,
                                  .data_in_num = I2S_PIN_NO_CHANGE
                                };
AudioOut *audioOut = NULL;
int connectedClientNum = 0;
WiFiServer server(8888);
void wifiEvent(WiFiEvent_t event) {
  switch (event) {
    case SYSTEM_EVENT_AP_START:
      Serial.println("ESP32 soft AP started");
      break;
    case SYSTEM_EVENT_AP_STACONNECTED:
      connectedClientNum++;
      Serial.println("Station connected to ESP32 soft AP");
      break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
      connectedClientNum--;
      Serial.println("Station disconnected from ESP32 soft AP");
      break;
    default:
      break;
  }
}
void setup() {
  //  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  //  //读取电压，电池电压低于3.7v就直接睡眠，ADC值大概算了一下在2300未精确测量，睡10分钟然后醒来检测一次
  //  if (analogRead(ADC_PIN) <= 2300) {
  //    esp_deep_sleep_start();
  //  }
  //是否开启debug
  if (OPEN_DEBUG) {
    Serial.begin(115200);
  }
  //状态led初始化
  pinMode(MUTE_OUT_PIN, OUTPUT);
  pinMode(STATUS_PIN, OUTPUT);
  pinMode(MUTE_STATUS_PIN, OUTPUT);
  digitalWrite(MUTE_OUT_PIN, HIGH);
  digitalWrite(STATUS_PIN, LOW);
  digitalWrite(MUTE_STATUS_PIN, LOW);

  //设置WiFi事件
  WiFi.onEvent(wifiEvent);
  WiFi.mode(WIFI_MODE_AP);
  WiFi.softAP(ssid, password);
  //延迟等待ap设置成功
  delay(200);
  WiFi.softAPConfig(localIp, gateWay, subNet);
  IPAddress IP = WiFi.softAPIP();
  server.begin();
  //初始化输出
  audioOut = new AudioOut();
  //1db大概是1.259倍,放大2个db
  audioOut->start(i2sConfig, i2SPinConfig, 1024, 1);
  //延迟等待ap设置成功
  delay(500);
  digitalWrite(STATUS_PIN, HIGH);
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    while (client.connected()) {
      while (client.available() > 0) {
        uint8_t readData[1024];
        client.read(readData, 1024);
        audioOut->startProcessData(readData);
      }
    }
  }
}
