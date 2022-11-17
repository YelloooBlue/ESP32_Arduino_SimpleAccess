/**
 * 读卡器相关
 * 类型：任务
 */
#include "config.h"

/*RFID相关库*/
#include <SPI.h>  //SPI库（RC522通讯）
#include <RFID.h> //RFID库

/*【参数配置】RFID配置*/
RFID rfid(21, 5); //读卡器SS（SDA）引脚、读卡器RST引脚（未接）

/*用户配置*/
int userNumber = 7;                                                                                              //用户数
char *userName[] = {"HL", "HLSH", "HLApple", "HXPhone", "HXSH", "LWY", "HYMPhone"};                              //用户姓名列表
char *userUid[] = {"E07BC90CB", "1D236B7025", "141747B4F", "5778FD1F1", "0EFF2908D", "2099F390DA", "2D4835137"}; //用户卡号列表0EFF2908D

/*
 *************************配置完毕*************************
 */

/*卡片检测*/
void RFID_findCard()
{
    unsigned char type[MAX_LEN]; // UID变量

    //找卡
    if (rfid.isCard())
    {
        // Serial.println("Find the card!"); //找到卡片

        //读取卡序列号
        if (rfid.readCardSerial())
        {

            String cardUid = ""; //卡片UID
            for (int i = 0; i < 5; i++)
                cardUid += String(rfid.serNum[i], HEX);

            cardUid.toUpperCase(); //转大写
            // Serial.print("Card UID: ");
            Serial.println(cardUid); //串口输出读取卡号，调试用

            RFID_auth(cardUid); //卡片认证
        }

        Serial.println(rfid.selectTag(rfid.serNum)); //选卡，可返回卡容量（锁定卡片，防止多数读取），去掉本行将连续读卡
    }
    rfid.halt();
}

/*卡片认证函数*/
void RFID_auth(String Uid)
{
    for (int i = 0; i < userNumber; i++)
    {
        if (String(userUid[i]) == Uid)
        {
            Serial.println("Hello!" + String(userName[i]));
            LOCK_unlock();
            return;
        }
    }
    Serial.println("Unkown User!"); //没有通过认证卡片
}

/*RFID初始化*/
void RIFD_init()
{
    /*SPI通信初始化*/
    SPI.begin();
    Serial.println("SPI inited");

    /*RFID初始化*/
    rfid.init();
    Serial.println("RFID inited");
}