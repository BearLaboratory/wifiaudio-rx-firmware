//
// Created by BLab on 2021/10/11.
//

#include <Arduino.h>
#include "AudioOut.h"
#include "driver/i2s.h"

void AudioOut::addSingleData(int16_t singleData) {
  this->receiveAudioBuffer[this->bufferPointer++] = singleData;
  if (this->bufferPointer == this->receivePackageSize) {
    std::swap(this->receiveAudioBuffer, this->playAudioBuffer);
    this->bufferPointer = 0;
    //通知播放
    xTaskNotify(this->playHandle, 1, eIncrement);
  }
}

void AudioOut::processData(uint8_t *bufferData) {
  int16_t *rawDatas = (int16_t *)bufferData;
  for (int i = 0; i < this->receivePackageSize; i++) {
    if (this->db == 0) {
      addSingleData(rawDatas[i] );
    } else {
      addSingleData(rawDatas[i] * 1.259 * this->db);
    }

  }
}
void AudioOut::startProcessData(uint8_t *bufferData) {
  this->processData(bufferData);
}

void playTask(void *param) {
  AudioOut *audioOut = (AudioOut *)param;
  const TickType_t xMaxBlockTime = pdMS_TO_TICKS(100);
  while (true) {
    uint32_t ulNotificationValue = ulTaskNotifyTake(pdTRUE, xMaxBlockTime);
    if (ulNotificationValue > 0) {
      //开始播放
      audioOut->doPlay();
    }
  }
}

void AudioOut::start(i2s_config_t i2SConfig, i2s_pin_config_t i2SPinConfig,
                     int32_t receivePackageSize, int db) {
  this->i2SConfig = i2SConfig;
  this->i2SPinConfig = i2SPinConfig;
  this->db = db;
  this->receivePackageSize = receivePackageSize / sizeof(int16_t);
  //分配buffer大小
  this->receiveAudioBuffer = (int16_t *)malloc(receivePackageSize);
  this->playAudioBuffer = (int16_t *)malloc(receivePackageSize);

  //安装i2s驱动
  i2s_driver_install(this->i2sPort, &i2SConfig, 0, NULL);
  //设置i2spin
  i2s_set_pin(this->i2sPort, &i2SPinConfig);
  //在0核心中读取数据
  xTaskCreatePinnedToCore(playTask, "playTask", 102400, this, 1,
                          &playHandle, 0);
}

void AudioOut::doPlay() {
  size_t bytesWritten = 0;
  i2s_write(this->i2sPort, this->playAudioBuffer, this->receivePackageSize * sizeof(int16_t),
            &bytesWritten, 10);
}
