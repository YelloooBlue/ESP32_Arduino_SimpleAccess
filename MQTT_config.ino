/**
 * MQTT相关
 */
#include "config.h"
#include <PubSubClient.h> //MQTT库
#include <WiFi.h>         //ESP32下的WIFI库

/*【参数配置】MQTT设置*/
#define MQTT_SERVER "yellowblue.top" // MQTT服务器
#define MQTT_SERVER_PORT 1883        // MQTT服务器端口

#define MQTT_ID "414lock"           // ClientID
#define MQTT_USER ""      //用户名
#define MQTT_PSWD "414"   //密码
#define MQTT_TOPIC "414lock" //订阅主题

extern WiFiClient Client_WIFI;
PubSubClient Client_MQTT(Client_WIFI); // MQTT客户端对象（基于WIFI对象）

/*
 *************************配置完毕*************************
 */

/*订阅的主题有消息发布时的回调函数*/
void MQTT_callback(char *topic, byte *payload, unsigned int length)
{

    String comdata = ""; //消息缓存
    for (int i = 0; i < length; i++)
        comdata += ((char)payload[i]);

    /*比对消息和开门密匙，MQTT开锁需要消息与密匙完全相符*/
    if (comdata.equals(OPEN_KEY))
    {
        Serial.println("MQTT_UNLOCK!");
        LOCK_unlock();
    }
}

/*连MQTT函数*/
void MQTT_connect()
{
    Serial.print("MQTT_startReConnecting!"); //发起连接（重连）
    // 尝试去连接,有15s的死区时间，此时间内无法刷卡或蓝牙
    if (Client_MQTT.connect(MQTT_ID, MQTT_USER, MQTT_PSWD))
    {
        Serial.println("MQTT_connected!!"); //连接成功

        vTaskDelay(1000);                  //延迟一秒，不然刚连上可能订阅不了，导致state=-3（MQTT_ CONNECTION_ LOST）
        Client_MQTT.subscribe(MQTT_TOPIC); //订阅主题
    }
    else
    {
        Serial.print("MQTT_error, state="); //连接失败，输出状态
        Serial.println(Client_MQTT.state());
    }
}

/*MQTT初始化*/
void MQTT_init()
{
    /*MQTT服务器初始化*/
    Client_MQTT.setServer(MQTT_SERVER, MQTT_SERVER_PORT); //设置MQTT服务器
    Client_MQTT.setCallback(MQTT_callback);               //绑定回调函数
    Serial.println("MQTT inited");
}