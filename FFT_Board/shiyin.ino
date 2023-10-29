#include "public.h"
#include "arduinoFFT.h" //要下arduinoFFT库

#include "SSD1306Wire.h"
/*
音频ADC-->D34

OLED   方便调试看数据，下面那行是频率是过了一个阈值然后保存的数据，环境频率就可以滤除了
SDA   23
SCL   22



*/
#define SAMPLES 2048 //采样点数，应为2的次方，可以根据要采样的时间持续时间设置
#define SAMPLING_FREQUENCY 10000 //采样频率 设置为待测频率上限两倍


//-----管脚宏定义-----
#define AudioADCpin  34       //音频ADC-->D34
//UI
#define SDA   23
#define SCL   22
SSD1306Wire display(0x3c, SDA, SCL);
//UIend

double peakcun;//调试

unsigned int sampling_period_us;
unsigned long microseconds;

double vReal[SAMPLES];
double vImag[SAMPLES];

arduinoFFT FFT = arduinoFFT(); //创建一个arduinoFFT对象

void setup() 
{
  Serial.begin(115200);
  sampling_period_us = round(1000000*(1.0/SAMPLING_FREQUENCY));


//UI部分
  display.init();//初始化UI
  display.flipScreenVertically();//垂直翻转屏幕设置
  display.setFont(ArialMT_Plain_16);//设置字体大小
  display.drawString(0, 0, "Start up");//显示

  display.display();//将缓存数据写入到显示器
//ui结束


}

void loop() 
{
  /*采集音频数据*/
  for(int i=0; i<SAMPLES; i++) 
  {
    microseconds = micros();    //Overflows after around 70 minutes!
   
    vReal[i] = analogRead(AudioADCpin);
    vImag[i] = 0;

    //Wait until sampling period_us has passed since we took the last sample data
    while(micros() - microseconds < sampling_period_us){
      // do nothing to wait out the sample period
    }
  }


  /*窗口化和FFT*/
  FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);

  /*提取频率特征*/
  double peak = FFT.MajorPeak(vReal, SAMPLES, SAMPLING_FREQUENCY);


  
  //-----UI-----
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
  display.setFont(ArialMT_Plain_16);
  display.drawString(64, 28, String(peak));
//调试
  display.setFont(ArialMT_Plain_10);
  display.drawString(64, 40, String(peakcun));


//
  display.display();//将缓存数据写入到显示器
  //-----UIend-----

  Serial.println(int(peak)); //串口打印出主频率

  if(peak>1500&&peak<4000){
    peakcun=peak;
  }



 
}