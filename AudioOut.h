//
// Created by BLab on 2021/10/11.
//

#ifndef WIFIAUDIORX_AUDIOOUT_H
#define WIFIAUDIORX_AUDIOOUT_H

#include "driver/i2s.h"

class AudioOut {
  private:
    // i2s序列
    i2s_port_t i2sPort = I2S_NUM_0;
    // i2s配置
    i2s_config_t i2SConfig;
    // i2s pin设置
    i2s_pin_config_t i2SPinConfig;
    //播放句柄
    TaskHandle_t playHandle;
    //两个缓冲buffer
    int16_t *receiveAudioBuffer;
    //发送缓存
    int16_t *playAudioBuffer;
    //接收缓冲区大小
    int32_t receivePackageSize;
    int bytesRead = 0;
    int32_t bufferPointer = 0;
    int db;

    void processData(uint8_t *bufferData);
    void addSingleData(int16_t singleData);
    void doPlay();

  public:
    void start(i2s_config_t i2SConfig, i2s_pin_config_t i2SPinConfig,
               int32_t receivePackageSize, int db);
    void startProcessData(uint8_t *bufferData);
    friend void playTask(void *param);
};

#endif  // WIFIAUDIORX_AUDIOOUT_H
