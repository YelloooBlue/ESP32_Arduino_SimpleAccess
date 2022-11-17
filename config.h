/**
 * 总配置文件
 */
#ifndef CONFIG
#define CONFIG


//全局参数
#define OPEN_KEY "414UnLoCk" //开锁密码


//全局变量
int WIFI_State;
/*
    255：WL_NO_SHIELD，兼容WiFi Shield而设计
    0：WL_IDLE_STATUS 正在WiFi工作模式间切换
    1：WL_NO_SSID_AVAIL 无法访问设置的SSID网络
    2：WL_SCAN_COMPLETED 扫描完成
    3：WL_CONNECTED 连接成功
    4：WL_CONNECT_FAILED 连接失败
    5：WL_CONNECTION_LOST 丢失连接
    6：WL_DISCONNECTED 断开连接
*/
int MQTT_State;
/*
    -4 : MQTT_CONNECTION_TIMEOUT - 服务器在保持活动时间内没有响应。
    -3 : MQTT_CONNECTION_LOST - 网络连接中断。
    -2 : MQTT_CONNECT_FAILED - 网络连接失败。
    -1 : MQTT_DISCONNECTED - 客户端干净地断开连接。
    0 : MQTT_CONNECTED - 客户端已连接。
    1 : MQTT_CONNECT_BAD_PROTOCOL - 服务器不支持请求的MQTT版本。
    2 : MQTT_CONNECT_BAD_CLIENT_ID - 服务器拒绝了客户端标识符。
    3 : MQTT_CONNECT_UNAVAILABLE - 服务器无法接受连接。
    4 : MQTT_CONNECT_BAD_CREDENTIALS - 用户名/密码被拒绝。
    5 : MQTT_CONNECT_UNAUTHORIZED - 客户端无权连接。
*/
bool BLE_Connected;
# endif