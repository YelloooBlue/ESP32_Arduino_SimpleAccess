# ESP32_Arduino_SimpleAccess
base on ESP32 for Arduino, door access system


- config.h 全局配置
- mqttnewBTNFC_RTOS.ino 主函数

- RTOS_task.ino RTOS相关任务

- LED_config.ino LED相关操作
- LOCK_config.ino 继电器开门相关操作

- RFID_config.ino RC-522刷卡相关操作

- BLE_config.ino 低功耗蓝牙BLE相关操作
- WIFI_config.ino WIFI相关操作
- MQTT_config.ino MQTT相关操作



### RTOS实现“多核多线程”

freeRTOS资料

[10.1 软件定时器的特性 — 韦东山百问网freeRTOS教程 文档](http://rtos.100ask.net/freeRTOS%E6%95%99%E7%A8%8B/docs/chap10_software_timer/section1.html#id9)

[FreeRTOS - Open Source software for microcontrollers - FreeRTOS xTimerCreateStatic() API function description](https://www.freertos.org/xTimerCreateStatic.html)


ESP32上的RTOS（有点不同）

[FreeRTOS - ESP32 - — ESP-IDF Programming Guide latest documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/freertos.html)

[【ESP32】arduino中的ESP32实时系统FreeRTOS使用教程（一）_创客协会的阿蛋°的博客-CSDN博客_arduino esp32 freertos](https://blog.csdn.net/weixin_51102592/article/details/127081731)


值得注意的是ESP32上不用调用`vTaskStartScheduler`()

> Unlike Vanilla FreeRTOS, users must not call [`vTaskStartScheduler()`](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/freertos.html#_CPPv419vTaskStartSchedulerv "vTaskStartScheduler"). Instead, ESP-IDF FreeRTOS is started automatically. The entry point is a user defined `void app_main(void)` function.


ESP32是双核处理器

| Task Name              | Affinity      | Priority | Description                                                                                                                                                                                                                               |
| ------------------------ | --------------- | ---------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Main Task (`main`)   | CPU0          | 1        | Task that simply calls `app_main`. This task will self delete when `app_main` returns                                                                                                                                                 |
| Idle Tasks (`IDLEx`) | CPU0 and CPU1 | 0        | Idle tasks created for (and pinned to) each CPU                                                                                                                                                                                           |
| IPC Tasks (`ipcx`)   | CPU0 and CPU1 | 24       | IPC tasks created for (and pinned to ) each CPU. IPC tasks are used to implement the IPC feature. See [Inter-Processor Call](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/ipc.html) for more details. |

IPC:Internet Process Connection进程间通信


Idle Task空闲任务

在本项目中必须有机会执行，`vTaskDelete ()`执行后，Task的内存并不会立即释放，而是等IdleTask执行时释放。在每次MQTT重连的Blocking任务执行完毕后，内存要得到释放，否则会爆满。

> NOTE: The idle task is responsible for freeing the kernel allocated memory from tasks that have been deleted. It is therefore important that the `idle task` is not starved of microcontroller processing time if your application makes any calls to `vTaskDelete ()`. Memory allocated by the task code is not automatically freed, and should be freed before the task is deleted.

绑定任务到核心0，在这里优先级设为0，防止IdleTask没有机会执行。

```cpp
xTaskCreatePinnedToCore(reconnectMQTT,   // Task function.
                                "reconnectMQTT", //任务名称
                                10000,           //任务的堆栈大小
                                NULL,            //任务的参数
                                0,               //任务的优先级
                                NULL,            //跟踪创建任务的任务句柄
                                0)    //指定核心0的任务
```

另外，与WIFI、蓝牙相关的任务可以放在0核心运行，其他外围任务放在1核心运行。


### WIFI重连方案设计

WIFI库资料（官方）

[Wi-Fi API — Arduino-ESP32 2.0.2 documentation](https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/api/wifi.html)

[Station Class — ESP8266 Arduino Core documentation](https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/station-class.html#status)


status()枚举类型

```cpp
typedef enum {
    WL_NO_SHIELD        = 255,   // for compatibility with WiFi Shield library
    WL_IDLE_STATUS      = 0,
    WL_NO_SSID_AVAIL    = 1,
    WL_SCAN_COMPLETED   = 2,
    WL_CONNECTED        = 3,
    WL_CONNECT_FAILED   = 4,
    WL_CONNECTION_LOST  = 5,
    WL_DISCONNECTED     = 6
} wl_status_t;
```


setAutoReconnect(true)不能实现重连。

> Note: running `setAutoReconnect(true)` when module is already disconnected will not make it reconnect to the access point. Instead `reconnect()` should be used.


WiFi.reconnect()重连（不能一直调用，会死循环

检测WIFI连接并重连的实现（Timer定时调用该函数）：

```cpp
//①【WIFI】连接检查及重连任务（Timer）
bool WIFI_reConnecting = true; //是否处于连接（重连）中状态，默认begin后自动进入连接状态
bool WIFI_Connected = false;
int reConnectCnt = 0; // WIFI重连次数
void checkWIFI(void *param)
{
    /*WIFI状态检测*/
    WIFI_State = WiFi.status();
    WIFI_Connected = (WIFI_State == WL_CONNECTED);
    // Serial.print("WIFIstate:");
    // Serial.println(WIFI_State);

    // ①WIFI正常连接，不进行下一步操作
    if (WIFI_Connected)
    {
        WIFI_reConnecting = false; //如果连接成功，退出连接（重连）状态
        reConnectCnt = 0;          //重连次数
        return;
    }

    // ②正在等待连接（重连），不进行下一步操作
    if (WIFI_reConnecting)
    {
        Serial.print("WIFI_waitingReConnect..."); //等待连接中
        Serial.println(++reConnectCnt);           //重连次数

        //由于未知原因，reconnect可能会挂掉，如果重连次数大于一定次数，重新reconnect一下
        if (reConnectCnt > 10)
            WIFI_reConnecting = false;
        return;
    }

    /*WIFI重连代码*/
    Serial.println("WIFI_startreConnecting!"); //发起连接（重连）,不能一直发起，会死循环
    WIFI_reConnecting = WiFi.reconnect();
    reConnectCnt = 0; //开始重连，重连次数置0
}
```

由于未知原因，reconnect可能会挂掉，如果重连次数大于一定次数，重新reconnect一下

挂掉的具体表现：status一直是WL_DISCONNECTED，正常重连的话可能是WL_NO_SSID_AVAIL




## MQTT重连方案设计

PubSubClient库资料

[Arduino Client for MQTT](https://pubsubclient.knolleary.net/)

[knolleary/pubsubclient: A client library for the Arduino Ethernet Shield that provides support for MQTT.](https://github.com/knolleary/pubsubclient)

[GitHub - zy19970/Pubsubclient\_API\_document: Pubsubclient库所有API的介绍，方便在Arduino进行MQTT开发时查阅使用。](https://github.com/zy19970/Pubsubclient_API_document)


这个库最鸡肋的就是connect()的阻塞问题[PubSubClient connect time - Google Search](https://github.com/knolleary/pubsubclient/issues/403)


调用connect的时候程序会blocking15秒去尝试连接MQTT服务器，如果一直连不上，程序逻辑一直connect的话，其他部分就没有机会执行了。

设置MQTT_ SOCKET_ TIMEOUT也没用


检测WIFI连接并重连的实现（Timer定时调用该函数）：

```cpp
//②【MQTT】连接检查（Timer）
bool MQTT_reConnecting = false; //是否处于连接（重连）中状态，默认未连接
bool MQTT_Connected = false;
void checkMQTT(void *param)
{
    Client_MQTT.loop(); //刷新客户端，检查心跳包

    /*MQTT状态检测*/
    MQTT_State = Client_MQTT.state();
    MQTT_Connected = (MQTT_State == MQTT_CONNECTED);
    // Serial.print("MQTTstate:");
    // Serial.println(MQTT_State);

    // ①WIFI未连接，不进行下一步操作
    if (!WIFI_Connected)
        return;

    // ②MQTT正常连接，不进行下一步操作
    if (MQTT_Connected)
    {
        MQTT_reConnecting = false; //如果连接成功，退出连接（重连）状态
        return;
    }

    //③正在等待连接（重连），不进行下一步操作
    if (MQTT_reConnecting)
    {
        Serial.println("MQTT_waitingReConnect..."); //等待连接中
        return;
    }

    /*MQTT重连代码*/
    // Serial.println("MQTT_reConnectingTaskStart!::::::::::::::::"); //发起连接（重连），不能一直发起，会爆内存

    /*
        由于PubSubClient的connect()是Blocking的，所以新建个Task来处理
        任务优先级要是0,否则Idle Task无法运行，无法释放已删除的Task的内存。导致内存不足，无法新申请的Task。
        核心为0，与WIFI蓝牙相关，且避免核心1的刷卡抢占。
        返回
            pdPASS(1)即为启动成功
            errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY(-1)内存不足
            pdFAIL(0)失败
    */
    if (xTaskCreatePinnedToCore(reconnectMQTT,   // Task function.
                                "reconnectMQTT", //任务名称
                                10000,           //任务的堆栈大小
                                NULL,            //任务的参数
                                0,               //任务的优先级
                                NULL,            //跟踪创建任务的任务句柄
                                0) != pdPASS     //指定核心0的任务
    )
    {
        Serial.println("MQTT_taskreConnecting_error"); //发起连接（重连）
    };
}
```

MQTT重连函数（Task）：

```cpp
//③【MQTT】重连任务（Blocking）
void reconnectMQTT(void *param)
{
    MQTT_reConnecting = true;

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
    MQTT_reConnecting = false;

    vTaskDelete(NULL);
}
```

延迟一秒订阅，不然刚连上可能订阅不了，导致state=-3（MQTT_ CONNECTION_ LOST）陷入死循环。


## BLE

[arduino-esp32/BLEDevice.h at master · espressif/arduino-esp32](https://github.com/espressif/arduino-esp32/blob/master/libraries/BLE/src/BLEDevice.h)

检测BLE连接，并实现长时间无通信的客户端并断开

```cpp
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

/*蓝牙初始化*/
BLEServer *pServer;
void BLE_init()
{
    BLEDevice::init("");         //蓝牙名称
    pServer = BLEDevice::createServer(); //创建服务器
    pServer->setCallbacks(new MyServerCallbacks());

    BLEService *pService = pServer->createService(SERVICE_UUID); //创建服务

    //创建特征
    BLECharacteristic *pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_WRITE);
    pCharacteristic->setCallbacks(new BLE_Callbacks()); //绑定回调函数
    pCharacteristic->setValue("Hello World"); //设置默认特征值

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
```

断联后需要重新开始广播才能再次被搜索到

每次有消息通讯时通过`lastMsgTime = millis();`更新时间。


## 报错

invalid conversion from 'void (*)()' to 'TaskFunction_t {aka void (*)(void*)}'

Task必须要有参数`(void *param)`


lld_pdu_get_tx_flush_nb HCI packet count mismatch (0, 1)

ESP32底层错误，GitHub上暂时没有解决方案，因为库调用的是ESP官方闭源的API。

刚开始用的不是乐鑫的ESP32（没有LOGO），蓝牙不稳定，时不时报这个错误，换了乐鑫的就行了。
