# 这是2020年ti杯电赛省赛a题的项目

## 分为

手机端（android），主显示端，姿态检测手环端，心率滤波读取端

## mcu

采用的是esp32.结合了适配esp32的arduino以及rtos框架进行开发。

### 开发环境 及 语言

安卓为android studio java开发

esp32为platform io  c/c++

### 节点间通信方式

tcp直连，手机端为总服务端

### 手机端

android 原生开发

### 主显示端

屏幕ili9341 spi

触摸xpt2046

图形 adafruit gfx

ad芯片 ads112c04

测温 lmt70

### 姿态检测端 

9轴 bno055

### 心率检测

心电  ads1292

## bilibili视频

https://www.bilibili.com/video/BV1uA411j7Ti/