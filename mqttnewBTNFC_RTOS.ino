#include "config.h"



/*
    全局变量
*/



void setup()
{

    Serial.begin(115200); //初始化串口
    /*注册服务（回调等）*/
    BLE_init();

    /*初始化硬件通信*/
    LOCK_init();
    RIFD_init();
    LED_init();

    /*WIFI先连接*/
    WIFI_init();

    /*启动MQTT*/
    MQTT_init();

    /*【常驻】刷卡检测（Core1）*/
    xTaskCreatePinnedToCore(checkCard,   // Task function.
                            "checkCard", //任务名称
                            10000,       //任务的堆栈大小
                            NULL,        //任务的参数
                            1,           //任务的优先级
                            NULL,        //跟踪创建任务的任务句柄
                            1);          //指定核心

    /*【定时器】检查WIFI*/
    TimerHandle_t Timer_checkWIFI = xTimerCreate(
        "checkWIFI",             /*任务名字*/
        1000 / portTICK_RATE_MS, /*设置时钟周期:ms*/
        pdTRUE,                  /*pdTRUE周期调用,pdFALSE:单次调用*/
        (void *)1,               /*计时器优先级*/
        checkWIFI);              /*定时回调函数*/

    /*【定时器】检查MQTT*/
    TimerHandle_t Timer_checkMQTT = xTimerCreate(
        "checkMQTT",             /*任务名字*/
        1000 / portTICK_RATE_MS, /*设置时钟周期:ms*/
        pdTRUE,                  /*pdTRUE周期调用,pdFALSE:单次调用*/
        (void *)1,               /*计时器优先级*/
        checkMQTT);              /*定时回调函数*/

    
    /*【定时器】检查BLE*/
    TimerHandle_t Timer_checkBLE = xTimerCreate(
        "checkBLE",             /*任务名字*/
        1000 / portTICK_RATE_MS, /*设置时钟周期:ms*/
        pdTRUE,                  /*pdTRUE周期调用,pdFALSE:单次调用*/
        (void *)1,               /*计时器优先级*/
        checkBLE);              /*定时回调函数*/


    /*【定时器】更新状态*/
    TimerHandle_t Timer_checkState = xTimerCreate(
        "checkState",            /*任务名字*/
        1000 / portTICK_RATE_MS, /*设置时钟周期:ms*/
        pdTRUE,                  /*pdTRUE周期调用,pdFALSE:单次调用*/
        (void *)0,               /*计时器优先级*/
        checkState);             /*定时回调函数*/


    //启用定时器
    xTimerStart(Timer_checkWIFI, 0); 
    xTimerStart(Timer_checkMQTT, 0); 
    xTimerStart(Timer_checkBLE, 0); 
    xTimerStart(Timer_checkState, 0); 
}

void loop()
{
    LED_blink();//LED
}