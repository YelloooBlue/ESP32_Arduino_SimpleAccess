/**
 * 低功耗蓝牙BLE相关
 */
#include "config.h"

/*蓝牙相关库*/
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

/*【参数配置】UUID设置*/
#define SERVICE_UUID "8928ea76-d9ff-4d4a-b422-b8f16c176bd5"
#define CHARACTERISTIC_UUID "548a2099-1095-4391-957f-e95a452542cc"
#define CONNECT_TIMOUT 15000 //最长连接时间（防止一个设备一直占用）

/*
 *************************配置完毕*************************
 */

/*蓝牙服务器回调函数*/
bool oldDeviceConnected = false; //上次连接状态
long lastMsgTime = millis();     //开机后两秒的初始化时间（等待网络连接）

class MyServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer)
    {
        BLE_Connected = true;
    }
    void onDisconnect(BLEServer *pServer)
    {
        BLE_Connected = false;
    }
};

/*蓝牙服务回调函数*/
class BLE_Callbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        std::string rxValue = pCharacteristic->getValue(); //接收信息

        if (rxValue.length() > 0)
        {
            lastMsgTime = millis();
            String cmd = rxValue.c_str(); // std::string转String
            // Serial.println(cmd);
            if (cmd.equals(OPEN_KEY))
            {
                Serial.println("BT_UNLOCK!");
                LOCK_unlock();
            }
        }
    }
};

/*蓝牙初始化*/
BLEServer *pServer;
void BLE_init()
{
    BLEDevice::init("智能门锁");         //蓝牙名称
    pServer = BLEDevice::createServer(); //创建服务器
    pServer->setCallbacks(new MyServerCallbacks());

    BLEService *pService = pServer->createService(SERVICE_UUID); //创建服务

    //创建特征
    BLECharacteristic *pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_WRITE);
    pCharacteristic->setCallbacks(new BLE_Callbacks()); //绑定回调函数
    pCharacteristic->setValue("Hello World says Neil"); //设置默认特征值

    pService->start(); //启动服务器

    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    pAdvertising->start(); //开始广播（被发现）
    pServer->startAdvertising();

    Serial.println("BT inited");
}

void BLE_loop()
{
    // BLE断开连接时
    if (!BLE_Connected && oldDeviceConnected)
    {
        Serial.println("BLE_disConnected！！！！！！！！！");
        // 给蓝牙堆栈准备数据的时间
        delay(500);
        pServer->startAdvertising();
        // 重新开始广播
        // Serial.println("start advertising");
        oldDeviceConnected = BLE_Connected;
    }

    // BLE正在连接时
    if (BLE_Connected && !oldDeviceConnected)
    {
        Serial.println("BLE_Connected！！！！！！！！！");
        // 正在连接时进行的操作
        oldDeviceConnected = BLE_Connected;
        lastMsgTime = millis();
    }

    // 已连接时
    if (BLE_Connected)
    {
        /*连接超过一定时间没有发消息，自动断开*/
        if (labs(millis() - lastMsgTime) > CONNECT_TIMOUT)
        {
            lastMsgTime = millis();
            pServer->disconnect(pServer->getConnId());
        }
    }
}