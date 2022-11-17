//#include "config.h"



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
    Client_MQTT.disconnect();
    reConnectCnt = 0; //开始重连，重连次数置0
}

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

//③【MQTT】重连任务（Blocking）
void reconnectMQTT(void *param)
{
    MQTT_reConnecting = true;
    MQTT_connect();
    MQTT_reConnecting = false;
    vTaskDelete(NULL);
}

//④【刷卡】处理任务（常驻）
void checkCard(void *param)
{
    while (true)
        RFID_findCard();
}

//⑤【BLE】检查连接，更新状态（Timer）
void checkBLE(void *param)
{
    BLE_loop();
}

//【状态】状态展示LED等（Timer）
void checkState(void *param)
{
    Serial.println("/**************STATE**************/");

    Serial.print("   ·WIFI: ");
    Serial.print(WIFI_State);
    Serial.println();

    Serial.print("   ·MQTT: ");
    Serial.print(MQTT_State);
    Serial.println();

    Serial.print("   ·BLE: ");
    Serial.print(BLE_Connected);
    Serial.println();

    Serial.print("   ·runtime:");
    Serial.print(millis());
    Serial.println();

    Serial.println("/*********************************/");
}
