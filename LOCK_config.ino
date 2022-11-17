/**
 * 锁相关配置
 */
#include "config.h"

/*继电器开门配置*/
#define JDQ_IO 13
#define OPEN_TIME 800 //开门延时时间（ms）

/*继电器初始化*/
void LOCK_init()
{
    pinMode(JDQ_IO, OUTPUT); //继电器
    digitalWrite(JDQ_IO, LOW);
    Serial.println("LOCK inited");
}

/*开锁函数*/
void LOCK_unlock()
{
    digitalWrite(JDQ_IO, HIGH); // sets the LED on
    delay(OPEN_TIME);           // waits for a second
    digitalWrite(JDQ_IO, LOW);
}
