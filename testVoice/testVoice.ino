// 导入必要的库
#include <WiFi.h>               // 引入WiFi核心库

// 注意到GPIO6，7，8，9，10，11已被用于SPI Flash，所以在定义串口TX和RX时请避开这些引脚。
// 由于ESP32开发板的GPIO36、39、34、35是纯输入口，因此最好不要把串口通讯IO口的TX定义在上面这四个口。
// RX则可以随便定义。
#define Serial1_TX 27
#define Serial1_RX 26

void setup()
{
  delay(2000);          // 延时2秒，用于等待系统上电稳定
  Serial.begin(115200); // 初始化串口，波特率 115200
  Serial.println("1111111111111");
  Serial1.begin(9600,SERIAL_8N1, Serial1_RX, Serial1_TX);//用于 和分贝仪通讯，0为RxD，16为TxD
}

void loop()
{
  getDbData();
  delay(500);
}

char command1[8] = {0x01,0x03,0x00,0x00,0x00,0x01,0x84,0x0A};

// 从分贝仪获取数据
void getDbData(void){
  Serial1.print(command1);
  delay(2000);
  while (Serial1.available()) {
     Serial.write(Serial1.read());
  }
}