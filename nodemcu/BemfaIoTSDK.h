#ifndef BEMFA_IOT_SDK_H
#define BEMFA_IOT_SDK_H
#include <Arduino.h>
#include <ArduinoJson.h>
#include "Client.h"

typedef void (*poniter_fun)(JsonVariant ele); //定义一个函数指针

typedef struct poniter_desc
{
  char *key;
  poniter_fun fp;
} poniter_desc, *p_poniter_desc;

// 最多绑定20个回调
static poniter_desc poniter_array[20];
// 最多绑定10个主题
static poniter_desc poniter_topic[10];
static p_poniter_desc p_poniter_array;

class BemfaIoTSDK
{
private:
  // mqtt 链接信息，动态生成的
  static char clientId[100];
  static char domain[50];

  // 定时检查 mqtt 链接
  static void mqttCheckConnect();

  static void messageBufferCheck();
  static void sendBuffer();
public:
  // 标记一些 topic 模板
  static char BEMFA_TOPIC_PROP_POST[50];
  // 标记一些 topic 模板
  static char BEMFA_TOPIC_PROP_POST_SET[60];
  // 标记一些 topic 模板
  static char BEMFA_TOPIC_PROP_POST_UP[60];
  // 在主程序 loop 中调用，检查连接和定时发送信息
  static void loop();

  /**
   * 初始化程序
   * @param clientId mqtt客户端id，对应巴法云用于私钥
   * @param _deviceName 设备名字，会用来构建设备主题，尽量模拟阿里云来
   */
  static void begin(Client &espClient,
                    const char *_clientId,
                    const char *_deviceName);

  /**
   * 发送数据到自定义主题
   * @param topic 自定义主题
   * @param payload 字符串形式的json 数据
   */
  static void sendTopicAndPayload(const char *topic,const char *payload);

  /**
   * 发送数据到系统主题
   *
   * 主题名加/set推送消息，表示向所有订阅这个主题的设备们推送消息，假如推送者自己也订阅了这个主题，消息不会被推送给它自己，以防止自己推送的消息被自己接收
   * @param param 字符串形式的json 数据，例如 {"${key}":"${value}"}
   */
  static void sendSet(const char *param);

  /**
   * 发送数据到系统主题
   *
   * 主题名后加/up推送消息，表示只更新云端数据，不进行任何推送
   * @param param 字符串形式的json 数据，例如 {"${key}":"${value}"}
   */
  static void sendUp(const char *param);

  /**
   * 发送数据
   * @param param 字符串形式的json 数据，例如 {"${key}":"${value}"}
   */
  static void send(const char *param);
  /**
   * 发送 float 格式数据
   * @param key 数据的 key
   * @param number 数据的值
   */
  static void send(char *key, float number);
  /**
   * 发送 int 格式数据
   * @param key 数据的 key
   * @param number 数据的值
   */
  static void send(char *key, int number);
  /**
   * 发送 double 格式数据
   * @param key 数据的 key
   * @param number 数据的值
   */
  static void send(char *key, double number);
  /**
   * 发送 string 格式数据
   * @param key 数据的 key
   * @param text 数据的值
   */
  static void send(char *key, char *text);
  /**
   * 绑定属性回调，云服务下发的数据包含此 key 会进入回调，用于监听特定数据的下发
   * @param key 物模型的key
   */
  static int bindData(char *key, poniter_fun fp);
  /**
   * 卸载某个 key 的所有回调（慎用）
   * @param key 物模型的key
   */
  static int unbindData(char *key);

  static int bindTopic(char *topic, poniter_fun fp);
  /**
   * 卸载某个 key 的所有回调（慎用）
   * @param key 物模型的key
   */
  static int unbindTopic(char *topic);
};
#endif
