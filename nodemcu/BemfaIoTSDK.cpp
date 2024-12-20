#include "BemfaIoTSDK.h"
#include "PubSubClient.h"

#define CHECK_INTERVAL 10000
#define MESSAGE_BUFFER_SIZE 10
#define RETRY_CRASH_COUNT 5

static const char *deviceName = NULL;

struct DeviceProperty
{
    String key;
    String value;
};

DeviceProperty PropertyMessageBuffer[MESSAGE_BUFFER_SIZE];

#define MQTT_PORT 9501

#define DATA_CALLBACK_SIZE 20

#define BEMFA_BODY_FORMAT "{\"id\":\"123\",\"version\":\"1.0\",\"method\":\"bemfa.property.post\",\"params\":%s}"

static unsigned long lastMs = 0;
static int retry_count = 0;

static PubSubClient *client = NULL;

char BemfaIoTSDK::clientId[100] = "";
char BemfaIoTSDK::domain[50] = "bemfa.com";

char BemfaIoTSDK::BEMFA_TOPIC_PROP_POST[50] = "SysThingPropertyPost";

// 主题名后加/set推送消息，表示向所有订阅这个主题的设备们推送消息，假如推送者自己也订阅了这个主题，消息不会被推送给它自己，以防止自己推送的消息被自己接收。
char BemfaIoTSDK::BEMFA_TOPIC_PROP_POST_SET[60] = "SysThingPropertyPost/set";

// 主题名后加/up推送消息，表示只更新云端数据，不进行任何推送
char BemfaIoTSDK::BEMFA_TOPIC_PROP_POST_UP[60] = "SysThingPropertyPost/up";

static void parmPass(JsonVariant parm)
{
    //    const char *method = parm["method"];
    for (int i = 0; i < DATA_CALLBACK_SIZE; i++)
    {
        if (poniter_array[i].key)
        {
            bool hasKey = parm["params"].containsKey(poniter_array[i].key);
            if (hasKey)
            {
                poniter_array[i].fp(parm["params"]);
            }
        }
    }
}

static void topicPass(char *topic,JsonVariant parm)
{
    for (int i = 0; i < 10; i++)
    {
        if (poniter_topic[i].key){
          if (strstr(topic, poniter_topic[i].key))
          {
            poniter_topic[i].fp(parm);
          }
        }
    }
}

// 所有云服务的回调都会首先进入这里，例如属性下发
static void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    payload[length] = '\0';
    Serial.println((char *)payload);

    if (strstr(topic, BemfaIoTSDK::BEMFA_TOPIC_PROP_POST))
    {
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, payload); //反序列化JSON数据

        if (!error) //检查反序列化是否成功
        {
            parmPass(doc.as<JsonVariant>()); //将参数传递后打印输出
        }
    } else {
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, payload); //反序列化JSON数据

        if (!error) //检查反序列化是否成功
        {
            topicPass(topic,doc.as<JsonVariant>()); //将参数传递后打印输出
        }
    }
}

static bool mqttConnecting = false;
void(* resetFunc) (void) = 0; //制造重启命令
void BemfaIoTSDK::mqttCheckConnect()
{
    if (client != NULL && !mqttConnecting)
    {
        if (!client->connected())
        {
            // client->disconnect();
            Serial.println("Connecting to MQTT Server ...");
            mqttConnecting = true;
            if (client->connect(clientId))
            {
                // 绑定系统主题
                client->subscribe(BEMFA_TOPIC_PROP_POST);
                // 自定义主题订阅
                for (int i = 0; i < 10; i++)
                {
                    if (poniter_topic[i].fp)
                    {
                        Serial.print("MQTT sub:");
                        Serial.println(poniter_topic[i].key);
                        client->subscribe(poniter_topic[i].key);
                    }
                }
                Serial.println("MQTT Connected!");
            }
            else
            {
                Serial.print("MQTT Connect err:");
                Serial.println(client->state());
                retry_count++;
                if(retry_count > RETRY_CRASH_COUNT){
                    resetFunc();
                }
            }
            mqttConnecting = false;
        }
        else
        {
            Serial.println("state is connected");
            retry_count = 0;
        }
    }
}

void BemfaIoTSDK::begin(Client &espClient,
                         const char *_clientId,
                         const char *_deviceName)
{

    client = new PubSubClient(espClient);
    deviceName = _deviceName;
    sprintf(clientId, "%s", _clientId);
    client->setServer(domain, MQTT_PORT); /* 连接WiFi之后，连接MQTT服务器 */
    client->setCallback(callback);

    mqttCheckConnect();
}

/**
 * 发送数据到自定义主题
 * @param topic 自定义主题
 * @param payload 字符串形式的json 数据
 */
void BemfaIoTSDK::sendTopicAndPayload(const char *topic,const char *payload){
    boolean d = client->publish(topic, payload);
    Serial.print("publish:0 成功:");
    Serial.println(d);
}

void BemfaIoTSDK::sendSet(const char *param)
{
    char jsonBuf[1024];
    sprintf(jsonBuf, BEMFA_BODY_FORMAT, param);
    Serial.println(jsonBuf);
    boolean d = d = client->publish(BEMFA_TOPIC_PROP_POST_SET, jsonBuf);

    Serial.print("sendSet publish:0 成功:");
    Serial.println(d);
}

void BemfaIoTSDK::sendUp(const char *param)
{
    char jsonBuf[1024];
    sprintf(jsonBuf, BEMFA_BODY_FORMAT, param);
    Serial.println(jsonBuf);

    boolean d = client->publish(BEMFA_TOPIC_PROP_POST_UP, jsonBuf);

    Serial.print("sendUp publish:0 成功:");
    Serial.println(d);
}

void BemfaIoTSDK::loop()
{
    client->loop();
    if (millis() - lastMs >= CHECK_INTERVAL)
    {
        lastMs = millis();
        mqttCheckConnect();
        messageBufferCheck();
    }
}

unsigned long lastSendMS = 0;

// 检查是否有数据需要发送
void BemfaIoTSDK::messageBufferCheck()
{
    int bufferSize = 0;
    for (int i = 0; i < MESSAGE_BUFFER_SIZE; i++)
    {
        if (PropertyMessageBuffer[i].key.length() > 0)
        {
            bufferSize++;
        }
    }
    // Serial.println("bufferSize:");
    // Serial.println(bufferSize);
    if (bufferSize > 0)
    {
        if (bufferSize >= MESSAGE_BUFFER_SIZE)
        {
            sendBuffer();
        }
        else
        {
            unsigned long nowMS = millis();
            // 3s 发送一次数据
            if (nowMS - lastSendMS > 5000)
            {
                sendBuffer();
                lastSendMS = nowMS;
            }
        }
    }
}

// 发送 buffer 数据
void BemfaIoTSDK::sendBuffer()
{
    int i;
    String buffer;
    for (i = 0; i < MESSAGE_BUFFER_SIZE; i++)
    {
        if (PropertyMessageBuffer[i].key.length() > 0)
        {
            buffer += "\"" + PropertyMessageBuffer[i].key + "\":" + PropertyMessageBuffer[i].value + ",";
            PropertyMessageBuffer[i].key = "";
            PropertyMessageBuffer[i].value = "";
        }
    }

    buffer = "{" + buffer.substring(0, buffer.length() - 1) + "}";
    send(buffer.c_str());
}

void addMessageToBuffer(char *key, String value)
{
    int i;
    for (i = 0; i < MESSAGE_BUFFER_SIZE; i++)
    {
        if (PropertyMessageBuffer[i].key.length() == 0)
        {
            PropertyMessageBuffer[i].key = key;
            PropertyMessageBuffer[i].value = value;
            break;
        }
    }
}
void BemfaIoTSDK::send(const char *param)
{

    char jsonBuf[1024];
    sprintf(jsonBuf, BEMFA_BODY_FORMAT, param);
    Serial.println(jsonBuf);
    boolean d = client->publish(BEMFA_TOPIC_PROP_POST, jsonBuf);
    Serial.print("publish:0 成功:");
    Serial.println(d);
}
void BemfaIoTSDK::send(char *key, float number)
{
    addMessageToBuffer(key, String(number));
    messageBufferCheck();
}
void BemfaIoTSDK::send(char *key, int number)
{
    addMessageToBuffer(key, String(number));
    messageBufferCheck();
}
void BemfaIoTSDK::send(char *key, double number)
{
    addMessageToBuffer(key, String(number));
    messageBufferCheck();
}

void BemfaIoTSDK::send(char *key, char *text)
{
    addMessageToBuffer(key, "\"" + String(text) + "\"");
    messageBufferCheck();
}

int BemfaIoTSDK::bindData(char *key, poniter_fun fp)
{
    int i;
    for (i = 0; i < DATA_CALLBACK_SIZE; i++)
    {
        if (!poniter_array[i].fp)
        {
            poniter_array[i].key = key;
            poniter_array[i].fp = fp;
            return 0;
        }
    }
    return -1;
}

int BemfaIoTSDK::bindTopic(char *topic, poniter_fun fp)
{
    int i;
    for (i = 0; i < 10; i++)
    {
        if (!poniter_topic[i].fp)
        {
            poniter_topic[i].key = topic;
            poniter_topic[i].fp = fp;
            return 0;
        }
    }
    return -1;
}

int BemfaIoTSDK::unbindTopic(char *topic)
{
    int i;
    for (i = 0; i < 10; i++)
    {
        if (!strcmp(poniter_topic[i].key, topic))
        {
            poniter_topic[i].key = NULL;
            poniter_topic[i].fp = NULL;
            return 0;
        }
    }
    return -1;
}

int BemfaIoTSDK::unbindData(char *key)
{
    int i;
    for (i = 0; i < DATA_CALLBACK_SIZE; i++)
    {
        if (!strcmp(poniter_array[i].key, key))
        {
            poniter_array[i].key = NULL;
            poniter_array[i].fp = NULL;
            return 0;
        }
    }
    return -1;
}
