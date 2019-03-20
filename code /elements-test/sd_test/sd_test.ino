/*
** MOSI - pin 11
** MISO - pin 12
** CLK - pin 13
** CS - pin 4
*/
#include <SD.h>
#include "DHT.h"

#define DHTPIN 14 
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
const int chipSelect = 53;  //设定CS接口
void setup()
{
  Serial.begin(9600);  //设置串口通信波特率为9600
  dht.begin();
  Serial.print("Initializing SD card...");  //串口输出数据Initializing SD card...
   
  if (!SD.begin(chipSelect)) {  //如果从CS口与SD卡通信失败，串口输出信息Card failed, or not present
    Serial.println("Card failed, or not present");
    return;
  }
  Serial.println("card initialized.");  //与SD卡通信成功，串口输出信息card initialized.
}

void loop()
{
  // 定义数组
  String dataString = "";
  // 读取三个传感器值，写入数组
  float h = dht.readHumidity();
   dataString += String(h);
   dataString += ",";
  float t = dht.readTemperature();
  dataString += String(t);
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print("  \t");
  Serial.print("SD storage: ");
  // 打开文件，注意在同一时间只能有一个文件被打开
  // 如果你要打开另一个文件，就需要先关闭前一个
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  // 打开datalog.txt文件，读写状态，位置在文件末尾。
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // 数组dataString输出到串口
    Serial.println(dataString);
  }
  // 如果无法打开文件，串口输出错误信息error opening datalog.txt
  else {
    Serial.println("error opening datalog.txt");
  }
    delay(3000);
}
