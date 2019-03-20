
/***************************************************************************** 
* 
* File Name : qinglian_example_1.ino
* 
* Description: show a example of the usage of qinglianSDK. 
* 
* Copyright (c) 2016 https://www.qinglianyun.com 
* All rights reserved. 
* 
* Author : ykgnaw
* 
* Date : 2016-06-27 
*****************************************************************************/ 
#include <MsTimer2.h>
#include <DHT.h>
#include <ql_interface.h>

/************* DHT11相关定义 **********************************/
#define DHTPIN       7
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
int temperature=0, humidity=0;
/**************************************************************/

/************* 青莲WIFI相关定义 *******************************/
#define PRODUCT_ID  "1000129322"//产品ID，请到青莲云官网注册开发者账号，并获取产品ID及秘钥
#define PRODUCT_KEY "3474667248552863269a22af6d36649b"//产品秘钥
#define PRODUCT_VER "01.01"//产品版本，请按照xx.xx的版本格式进行填写

QLCloud    qlcloud;
bool is_wifi_init = false;

/**************************************************************/
int32_t SerialSendBytes(uint8_t*data, int32_t len)
{
  Serial.write(data, len);
}
//发送初始化wifi模组指令
void init_wifi()
{
  qlcloud.init((uint8_t*)PRODUCT_ID, (uint8_t*)PRODUCT_KEY, (uint8_t*)PRODUCT_VER); 
}
//上传温度值
void upload_temperature()
{
  int ret = 0;
  char value[16];
  char *key_t = "temperature";//变量名要与青莲云平台变量名一致
  itoa(temperature, value, 10);
  qlcloud.upload_data((uint8_t*)key_t, strlen(key_t), (uint8_t*)value, strlen(value)); 
}
//上传湿度值
void upload_humidity()
{
  int ret = 0;
  char value[16];
  char *key_h = "humidity";//变量名要与青莲云平台变量名一致
  itoa(humidity, value, 10);
  qlcloud.upload_data((uint8_t*)key_h, strlen(key_h), (uint8_t*)value, strlen(value));
}
//查询状态回调函数
void query_satus_cb(int32_t sta)
{
  Serial.print("query_satus_cb");
  Serial.println(sta);
  if(sta == STATUS_NOT_INIT) {//wifi模组未初始化
    is_wifi_init = false;
    init_wifi();
  }else {//wifi模组初始化完成
    is_wifi_init = true;
 
  }
}
void gettime_cb(uint32_t tt)
{
 Serial.print("time stamp");
 Serial.println(tt);
  
}

//定时器定时函数
void timer_handle()
{
  static int temp_or_humi = 0;
  qlcloud.get_onlinetime();
  if(!is_wifi_init) {//wifi模组未初始化
    qlcloud.query_module_status();
  } else {//wifi模组初始化完成
    switch(temp_or_humi){//每隔3秒，轮流发送温度、湿度值
    case 0:
      upload_temperature();
      temp_or_humi = 1;
      break;
    case 1:
      upload_humidity();
      temp_or_humi = 0;
      break;
    }
  }
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  //注册串口发数据的回调函数
  qlcloud.reg_send_bytes_cb(SerialSendBytes);
  //注册查询状态的回调函数
  qlcloud.reg_query_status_cb(query_satus_cb);
  //开启调试打印功能
  qlcloud.set_wifi_debug_status(1);
  qlcloud.reg_gettime_cb(gettime_cb);
  //初始化dht模块
  dht.begin();
  
  MsTimer2::set(3000, timer_handle); 
  MsTimer2::start();//开启一个3秒的定时器
}
void loop() {
  // put your main code here, to run repeatedly:
  //读取湿度值
  humidity = (int)dht.readHumidity();
  //读取温度值
  temperature = (int)dht.readTemperature();
  
  delay(500);
  //读取串口缓冲区
  int available_num = Serial.available();
  while(available_num--) {
    qlcloud.rx_byte(Serial.read());
  }
  //处理串口数据
  qlcloud.rx_data_handler();
  delay(1000);
}
