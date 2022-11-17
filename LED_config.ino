/**
 * LED相关配置
 */
#include "config.h"

/*引脚配置*/
#define LED_IO 2

/*LED初始化*/
void LED_init()
{
    pinMode(LED_IO, OUTPUT); // LED
    digitalWrite(LED_IO, HIGH);
    Serial.println("LED inited");
}

/*闪灯函数*/
long lastBlinkTime = millis();
void LED_blink()
{

    /*
        正常隔10s闪
        没连WIFI隔3s闪
        没连MQTT不闪（一般是晚上）
        连接蓝牙爆闪
    */

    int gap = 10000;

    //没连接wifi
    if (WIFI_State != 3)
        gap = 3000;
    else
        //没链接MQTT
        if (MQTT_State != 0)
            gap = 0;

    //连接了蓝牙
    if (BLE_Connected)
        gap = 100;

    //不亮
    if (gap == 0)
    {
        digitalWrite(LED_IO, LOW);
        return;
    }

    if (millis() - lastBlinkTime > gap)
    {
        lastBlinkTime = millis();
        digitalWrite(LED_IO, HIGH);
        delay(50);
        digitalWrite(LED_IO, LOW);
    }
}
