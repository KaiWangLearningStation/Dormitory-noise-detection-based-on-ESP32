// 导入必要的库
#include <WiFi.h>               // 引入WiFi核心库
#include <HTTPClient.h>         // 引入http相关库
#include <ArduinoJson.h>        // 引入JSON处理库
#include "PubSubClient.h"       // 引入MQTT处理库
#include "BemfaIoTSDK.h"        // 引入巴法云 IoT SDK
#include <TFT_eSPI.h>
#include <SPI.h>
#include <TJpg_Decoder.h>
#include <Adafruit_NeoPixel.h>  // 引入WS2812库
#include <ESP32Servo.h>
/* *****************************************************************
 *  字库、图片库
 * *****************************************************************/
#include "img/emoji1.h"
#include "img/emoji2.h"
#include "img/emoji3.h"
#include "img/emoji4.h"

typedef bool (*retry_fun)(char *api, char *tag, poniter_fun fp); //定义一个函数指针

// 设备启动打印信息
const char projectInfo[] PROGMEM = R"rawliteral(
/* *****************************************************************
 * 基于ESP32 240x240 圆屏 噪音表情
 * *****************************************************************/
)rawliteral";
/******************* 常量声明 **********************/
// 当前版本
#define Version  "CL V1.0"

#define SSID "CMCC-pm3h"          // 填入自己的WiFi账号
#define PASSWORD "hw2htwv4"      // 填入自己的WiFi密码

//#define TFT_MOSI 23 // In some display driver board, it might be written as "SDA" and so on.
//#define TFT_SCLK 18
//#define TFT_CS   5  // Chip select control pin
//#define TFT_DC   16  // Data Command control pin
//#define TFT_RST  17  // Reset pin (could connect to Arduino RESET pin)
//#define TFT_BL   4  // LED back-light
#define SERVO_PIN  12            // 舵机引脚
#define VOICE_PIN  34            // 声音传感器对应的引脚
#define WS2812_PIN 22            // WS2812对应的控制引脚

// 注意到GPIO6，7，8，9，10，11已被用于SPI Flash，所以在定义串口TX和RX时请避开这些引脚。
// 由于ESP32开发板的GPIO36、39、34、35是纯输入口，因此最好不要把串口通讯IO口的TX定义在上面这四个口。
// RX则可以随便定义。
#define Serial1_TX 27
#define Serial1_RX 26

#define NUM_PIXELS 12            // ws2812灯珠长度
#define BRIGHT     10            // 灯珠亮度

#define INTERVAL 5000        // 读取时间间隔，默认10s
#define BUFFER_SIZE 200       // 缓存区大小
//---------------- 巴法云相关配置信息 ------------------//
// 二元组信息
#define UID "a0f3348e687648848ceaeb3e970f6ff5"     // 巴法云私钥
#define TCP_SERVER_ADDR "bemfa.com"  // 服务器地址，不需要修改
#define TCP_SERVER_PORT "8344"       // 服务器端口-TCP创客云端口8344-TCP设备云端口8340
#define DEVICE_NAME "esp32"          // 设备名字，自定义，注意不要同名,并且只能是数字+字母组合
#define MAX_PACKAGE_SIZE 512         // 最大字节数

// mqtt主题
#define EMOJI_TOPIC  "emoji"
#define DB_HOUR_TOPIC     "dBHour"  // 12个数据
#define DB_TOPIC "dB"

// API 定义
#define API_GET_MSG "http://apis.bemfa.com/va/getmsg?type=1&uid=" UID    // 获取主题消息API
#define API_EMOJI API_GET_MSG "&topic=" EMOJI_TOPIC
#define API_DB_HOUR API_GET_MSG "&topic=" DB_HOUR_TOPIC

// 设备属性ID 用来解析主题携带的消息
#define ID_VOICE "voice"     // 代表分贝属性
#define ID_EMOJI "emoji"     // 代表表情"
//---------------- 巴法云相关配置信息 ------------------//
/******************* 常量声明 **********************/

/******************* 函数声明 **********************/
void initServo(void);                   // 舵机初始化
void doWiFiConnectTick(void);           // 检测WiFi连接状态
void sendDataToBemfa(void);             // 发送数据到巴法云平台
void getVoice(void);                    // 读取 数据
void initBemfaIotSDK(void);             // 初始化巴法云IOT SDK
void initWifiConnect(void);             // 初始化Wifi连接
void initSystemState(void);             // 初始化系统状态
void emojiChangeCallback(JsonVariant p); // 设置水泵开关回调
bool getLastMsg(char *api, char *tag, poniter_fun fp); // 获取对应主题最后一条消息
void retry(retry_fun func, int time, char *api, char *tag, poniter_fun fp); // 错误重试函数
void initW2812(void);
void handleWS2812(void);
void handleServo(void);
int getRed(float value);
int getGreen(float value);
int getBlue(float value);
void getDbData(void);
/******************* 函数声明 **********************/

/******************* 变量定义 **********************/
static WiFiClient espClient;         // 表示 TCP 客户端，跟巴法云MQTT配合使用

Servo myServo;
float dB = 0;                        // 记录噪音DB
int voice = 0;                     // 记录当前噪音数据
unsigned long previousMillis = 0;  // 记录上次读取的时间戳
char param[BUFFER_SIZE];           // 存储上传到巴法云的Json数据
int redArr[NUM_PIXELS] = {255,255,255,255,255,255,255,255,255,255,255,255};
int greenArr[NUM_PIXELS]={255,255,255,255,255,255,255,255,255,255,255,255};
int blueArr[NUM_PIXELS]={255,255,255,255,255,255,255,255,255,255,255,255};
int lastEmojiNum = 0;                  // 记录上一次当前表情状态
int curEmojiNum = 1;                   // 记录当前表情状态
TFT_eSPI tft = TFT_eSPI();              // 引脚请自行配置tft_eSpi库中的 User_Setup.h文件
TFT_eSprite clk = TFT_eSprite(&tft);
Adafruit_NeoPixel pixels(NUM_PIXELS, WS2812_PIN, NEO_GRB + NEO_KHZ800); // 表示WS2812模块
/******************* 变量定义 **********************/

void setup()
{
  delay(2000);          // 延时2秒，用于等待系统上电稳定
  Serial.begin(115200); // 初始化串口，波特率 115200
  Serial.println("");
  Serial.println(projectInfo);
  Serial1.begin(9600,SERIAL_8N1, Serial1_RX, Serial1_TX);//用于 和分贝仪通讯，0为RxD，16为TxD
  initW2812();
  initServo();

  tft_init(); //屏幕初始化

  initWifiConnect(); // 初始化Wifi连接
  initBemfaIotSDK(); // 初始化巴法云IOT SDK
  initSystemState(); // 初始化数据状态
  tft_display_loading_complete();   // 将进度条刷到100%
}

void loop()
{
  doWiFiConnectTick(); // wifi连接状态检测以及重连

  if (WiFi.status() == WL_CONNECTED) {
      if(millis() - previousMillis >= INTERVAL) // 每隔一段时间读取一下传感器数据 interval为时间间隔
      {
        getDbData();
        getVoice();                     // 读取土壤湿度
        voiceToDB();                    // 把voice数据转成DB
        checkDB();
        sendDataToBemfa(); // 发送数据到巴法云平台
        BemfaIoTSDK::loop(); // 检测MQTT 巴法云
        handleWS2812();
        previousMillis = millis(); // 记录当前时间戳
      } else {
         BemfaIoTSDK::loop(); // 检测MQTT 巴法云
      }
  }
  changeEmoji();                  // 改变表情
  delay(500);               // 延时 500 毫秒
}

char command1[8] = {0x01,0x03,0x00,0x00,0x00,0x01,0x84,0x0A};

// 从分贝仪获取数据
void getDbData(void){
  Serial1.print(command1);
  delay(200);
  while (Serial1.available()) {
     Serial.write(Serial1.read());
  }
}

/**
 * 初始化
 */
void initW2812(void){
   pixels.begin();              // 初始化WS2812
   pixels.setBrightness(BRIGHT);  // 设置灯珠亮度
   pixels.clear();
}

void initServo(void){
  myServo.setPeriodHertz(50);
  myServo.attach(SERVO_PIN, 500, 2500);
}

void handleServo(void){
  myServo.write(90);
  delay(500);
  myServo.write(180);
  delay(500);
}

/**
 * 处理WS2812显示效果
 *
 */
void handleWS2812(void)
{
  Serial.println("handleWS2812~"); // 串口打印信息表示执行了此函数
  for(int index=0; index<NUM_PIXELS; index++) // 遍历所有的WS2812灯珠
  {
    Serial.printf("%d r:%d,g:%d,b:%d\n",index,redArr[index], greenArr[index], blueArr[index]);
    pixels.setPixelColor(index, redArr[index], greenArr[index], blueArr[index]);// 设置每个WS2812灯珠RGB值
    pixels.show();   // 发送RGB值给到每个WS2812灯珠
    delay(20);
  }
}

/**
 * 根据分贝值来切换表情
 */
void checkDB(void) {
  if (dB > 0 && dB <= 35) {
   curEmojiNum = 1;
  } else if (dB > 35 && dB <= 70) {
   curEmojiNum = 2;
  } else {
   curEmojiNum = 3;
  }
}

/**
 * 根据分贝值来切换表情
 */
void changeEmoji(void) {
  if (curEmojiNum == lastEmojiNum) return;
  if(curEmojiNum == 1){
   tft_display_emoji1();
  } else if (curEmojiNum == 2){
   tft_display_emoji2();
  } else if(curEmojiNum == 3){
   tft_display_emoji3();
  } else {
   tft_display_emoji4();
  }
  Serial.printf("changeEmoji %d %d\n",lastEmojiNum,curEmojiNum);
  lastEmojiNum = curEmojiNum;
}

/**
 * 重试执行，避免有些方法执行一次失败
 */
void retry(retry_fun func, int time, char *api, char *tag, poniter_fun fp){
  int retry = 0;
  while(!func(api, tag, fp)){
    retry++;
    Serial.printf("retry :%d\n",retry);
    if (retry == time){
     break;
    }
  }
}

/*
 * 初始化数据信息
 */
void initSystemState(void) {
  retry(getLastMsg, 5, API_EMOJI, "表情", emojiChangeCallback);
  retry(getLastMsg, 5, API_DB_HOUR, "DB数据", dbHourChangeCallback);
}

/**
 * 初始化Wifi连接
 */
void initWifiConnect(void){
  Serial.printf("Connecting to WiFi:%s\n",SSID);// 串口打印当前WiFi热点的名字
  WiFi.disconnect();   // 默认断开之前的连接，回归初始化非连接状态
  WiFi.mode(WIFI_STA); // 设置ESP工作模式为Station模式
  WiFi.begin(SSID, PASSWORD);  // 连接到WiFi

  int cnt = 0; // 记录重试次数
  while (WiFi.status() != WL_CONNECTED) // 当还没有连接上WiFi热点时
  {
     for (byte n = 0; n < 10; n++)
    {
        tft_display_loading(50); //每50毫秒检测一次，刷新一次进度条
     }
     cnt++;             // 累计次数+1
     Serial.print("."); // 串口输出.表示设备正在连接WiFi热点
     if(cnt>=40)        // 超过40秒还没有连接上网络
     {
       ESP.restart();// 重启ESP模块
     }
  }
  Serial.println(WiFi.localIP()); // 打印当前IP地址
}

/**
 * 初始化巴法云IOT SDK
 */
void initBemfaIotSDK(void){
  // 设置自定义主题处理函数
  BemfaIoTSDK::bindTopic(EMOJI_TOPIC,emojiChangeCallback);
  BemfaIoTSDK::bindTopic(DB_HOUR_TOPIC,dbHourChangeCallback);
  // 初始化 iot sdk，需传入 wifi 的 client，和设备产品信息
  BemfaIoTSDK::begin(espClient, UID, DEVICE_NAME);
}

/**
 * 根据每个小时数据来修改rgb颜色
 */
void dbHourChangeCallback(JsonVariant root){
    Serial.println(F("dbHourChangeCallback"));
    // 处理数据
    for (JsonPair kv : root.as<JsonObject>()) {
        Serial.print("Key: ");
        Serial.print(kv.key().c_str());
        int index = atoi(kv.key().c_str());
        float value = root[kv.key().c_str()];
        Serial.print(", Value: ");
        Serial.println(value);
        if(index >= 0 && index < NUM_PIXELS){
          redArr[index] = getRed(value);
          greenArr[index]= getGreen(value);
          blueArr[index]= getBlue(value);
        }
    }
}

/***
0-40 R: 204 G:94 B 255
40-60 R: 164 G:29 B 226
60以上 R: 123 G:0 B 180
*/
int getRed(float value){
   if (value > 0 && value < 40){
     return 204;
   }
   if (value >= 40 && value < 60){
     return 164;
   }
   return 123;
}

int getGreen(float value){
   if (value > 0 && value < 40){
     return 94;
   }
   if (value >= 40 && value < 60){
     return 29;
   }
   return 0;
}

int getBlue(float value){
   if (value > 0 && value < 40){
     return 255;
   }
   if (value >= 40 && value < 60){
     return 226;
   }
   return 180;
}

/**
 * 设置表情包切换回调
 */
void emojiChangeCallback(JsonVariant p){
    Serial.println(F("emojiChangeCallback"));
    // 处理数据
    if(p.containsKey(ID_EMOJI)) {
       curEmojiNum  = p[ID_EMOJI];
       Serial.printf("type:%d\n", curEmojiNum);
       previousMillis = millis();
    }
}

/**
 * 功能：连接路由心跳函数
 */
void doWiFiConnectTick(void) {
  static uint32_t lastWiFiCheckTick = 0; // 记录最近一次检测WiFi连接状态的时间点
  static uint32_t disConnectCount = 0;   // 记录WiFi断开连接的次数

  if(WiFi.status() == WL_CONNECTED) // 当前WiFi处于连接状态
  {
    disConnectCount = 0;// 重置WiFi断开连接为0
    return;
  }

  if(millis() - lastWiFiCheckTick > 1000) // 检测间隔大于1秒
  {
    lastWiFiCheckTick = millis();       // 记录时间点
    Serial.println("WiFi disConnect!"); // 串口输出.表示设备已经断开连接

    disConnectCount++;      // WiFi断开连接的次数累计加1
    if(disConnectCount>=40) // 断开连接累计次数达到40次，表示可能wifi连接异常
    {
      ESP.restart();// 重启ESP模块
    }
  }
}

/**
 * 发送数据到巴法云平台
 */
void sendDataToBemfa(void){
  if(WiFi.status() != WL_CONNECTED) // Wifi处于连接状态
  {
     Serial.println("network not connected!");
     return;
  }
  memset(param, 0, BUFFER_SIZE);      //清空缓存数据
  sprintf(param, "{\"%s\":%d}",ID_VOICE, voice); // 构成需要上传到巴法云平台的Json数据
  Serial.printf("param:%s\n", param); // 串口输出最终发送的数据
  BemfaIoTSDK::sendTopicAndPayload(DB_TOPIC,param);           // 发送数据到 巴法云平台
}

/**
 * 获取对应主题最后一条消息
 * @param api 请求api
 * @param tag 标签 用来打印日志
 * @param fp  回调方法
 *
 * @result bool 是否请求成功
 */
bool getLastMsg(char *api, char *tag, poniter_fun fp) {
    Serial.printf("[%s]getLastMsg: %s\n", tag, api);
    HTTPClient http;
    //设置请求url
    http.begin(api);
    //发起请求，并获取状态码
    int httpResponseCode = http.GET();
    bool result = false;

    if(httpResponseCode==200){
        String response = http.getString();
        Serial.printf("[%s]response: %s\n", tag, response.c_str());
        StaticJsonDocument<600> doc;
        DeserializationError error = deserializeJson(doc, response); //反序列化JSON数据
        //检查反序列化是否成功
        if (!error){
          int code = doc["code"];
          // 请求成功
          if (code == 0) {
            String msg = doc["data"][0]["msg"];
            Serial.printf("[%s]msg: %s\n", tag, msg);
            StaticJsonDocument<300> docMsg;
            DeserializationError errorMsg = deserializeJson(docMsg, msg); //反序列化JSON数据
            //检查反序列化是否成功
            if (!errorMsg){
                 Serial.printf("[%s]success!\n", tag);
                 fp(docMsg.as<JsonVariant>());//将参数传递后打印输出
                 result = true;
             } else {
               Serial.printf("[%s]Error on Msg Json Parse\n", tag);
             }
          } else {
            // 打印错误信息
            Serial.printf("[%s]error:%s\n", tag, doc["message"]);
          }
        } else {
          Serial.printf("[%s]Error on Root Json Parse\n", tag);
        }
    }else{
        Serial.printf("[%s]Error on sending GET:%d\n", tag,httpResponseCode);
    }

    Serial.printf("[%s]getLastMsg finish!\n", tag);
    //回收下次再用
    http.end();
    return result;
}

void getVoice(void)
{
  Serial.print("声音数据 "); // 串口输出内容
  voice = analogRead(VOICE_PIN);
  Serial.println(voice, DEC);
}

void voiceToDB(void){
  dB = voice / 4096.0 * 100; //Convert ADC value to dB using Regression values
  Serial.print("DB:"); // 串口输出内容
  Serial.println(dB); // 串口输出内容
}