// 导入必要的库
#include <Adafruit_NeoPixel.h>  // 引入WS2812库

#define WS2812_PIN 22            // WS2812对应的控制引脚

#define NUM_PIXELS 12            // ws2812灯珠长度
#define BRIGHT     50            // 灯珠亮度


/******************* 函数声明 **********************/
void initW2812(void);
void handleWS2812(void);
/******************* 函数声明 **********************/

/******************* 变量定义 **********************/
Adafruit_NeoPixel pixels(NUM_PIXELS, WS2812_PIN, NEO_GRB + NEO_KHZ800); // 表示WS2812模块
/******************* 变量定义 **********************/

void setup()
{
  delay(2000);          // 延时2秒，用于等待系统上电稳定
  Serial.begin(115200); // 初始化串口，波特率 115200
  Serial.println("");
  Serial.println("13413243214132");
  initW2812();
}

void loop()
{
  handleWS2812();                
  delay(500);               // 延时 500 毫秒
}

/**
 * 初始化
 */
void initW2812(void){
   pixels.begin();              // 初始化WS2812
   pixels.setBrightness(BRIGHT);  // 设置灯珠亮度
   pixels.clear();
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
    pixels.setPixelColor(index, 255, 255, 0);// 设置每个WS2812灯珠RGB值
    pixels.show();   // 发送RGB值给到每个WS2812灯珠
    delay(20);
  }
}