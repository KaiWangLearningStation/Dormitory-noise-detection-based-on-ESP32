int rotation = 2;       // 屏幕方向

//初始化屏幕
extern void tft_init(void){
  tft.begin();           // TFT初始化
  tft.setRotation(rotation);    // 旋转角度0-3
  tft.setTextColor(TFT_BLACK, TFT_WHITE); //设置字体颜色
  tft.fillScreen(TFT_BLACK); // 清屏
  
  TJpgDec.setJpgScale(1);       // 设置放大倍数
  TJpgDec.setSwapBytes(true);   // 它的作用是设置TFT液晶屏的像素字节序。在某些情况下，像素字节序可能需要被交换，以确保图像正确显示。这段代码中的true表示需要交换字节序，而false则表示不需要交换字节序。
  TJpgDec.setCallback(tft_output);  // 回调函数
}

extern bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap) {
  if (y >= tft.height())
    return 0;
  // 这句代码是将一个位图(bitmap)显示在TFT屏幕上，其中x和y是位图左上角的坐标，w和h是位图的宽度和高度。具体来说，它将位图的像素数据推送到TFT屏幕上，从而在指定的位置显示出来。
  tft.pushImage(x, y, w, h, bitmap);
  return 1;
}

// 进度条
byte loadNum = 6;
extern void tft_display_loading(byte delayTime){
  clk.setColorDepth(8);                                // 设置TFT屏幕的颜色深度为8位。TFT屏幕的颜色深度指的是每个像素点可以显示的颜色数量，8位颜色深度可以显示256种颜色。
  clk.createSprite(200, 50);                           // 创建Sprite
  clk.fillSprite(TFT_BLACK);                           // 填充颜色：黑色

  clk.drawRoundRect(0, 0, 200, 16, 8, TFT_WHITE);      // 画一个圆角矩形框，白色
  clk.fillRoundRect(3, 3, loadNum, 10, 5, TFT_WHITE);  // 画一个填充的圆角矩形，白色
  clk.setTextDatum(CC_DATUM);                          // 设置文本显示基准为居中对齐
  clk.setTextColor(TFT_GREEN, TFT_BLACK);              // 设置文本的前景色和背景色
  clk.drawString("Connecting to WiFi", 100, 40, 2);    // 显示“Connecting to WiFi”这个字符串，位置为(100,40)，字体大小为2。
  clk.pushSprite(20, 110);                             // Sprite中内容一次推向屏幕
  clk.deleteSprite();                                  // 删除Sprite
  loadNum += 1;
  if (loadNum >= 194)
  {
    loadNum = 194;
  }
  delay(delayTime);
}

// 将进度条刷到100%
extern void tft_display_loading_complete(void){
  while (loadNum < 194) { //让动画走完
    tft_display_loading(1);
  }
} 

extern void tft_display_emoji1(void){
  tft.fillScreen(TFT_BLACK);                      //清屏
  TJpgDec.drawJpg(0,0,emoji1[0], emoji1_size[0]);
}

extern void tft_display_emoji2(void){
  tft.fillScreen(TFT_BLACK);                      //清屏
  TJpgDec.drawJpg(0,0,emoji2[0], emoji2_size[0]);
}

extern void tft_display_emoji3(void){
  tft.fillScreen(TFT_BLACK);                      //清屏
  TJpgDec.drawJpg(0,0,emoji3[0], emoji3_size[0]);
}

extern void tft_display_emoji4(void){
  tft.fillScreen(TFT_BLACK);                      //清屏
  TJpgDec.drawJpg(0,0,emoji4[0], emoji4_size[0]);
}
