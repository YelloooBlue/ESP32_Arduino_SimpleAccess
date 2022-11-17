/**
 * WIFI相关
 */
#include "config.h"

#include <WiFi.h> //ESP32下的WIFI库

/*【参数配置】WIFI配置*/
#define WIFI_SSID "818"       // WIFI名称
#define WIFI_PSW "hlhxlwyhym" // WIFI密码

WiFiClient Client_WIFI; // WIFI客户端对象

/*
 *************************配置完毕*************************
 */

/*WIFI初始化*/
void WIFI_init()
{
    WiFi.begin(WIFI_SSID, WIFI_PSW); //连接WIFI
    WiFi.setAutoReconnect(true);
    Serial.println("WIFI inited");
}