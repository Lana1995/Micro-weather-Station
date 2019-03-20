
#include <SD.h>
#include <MsTimer2.h>
#include <DHT.h>
#include <ql_interface.h>
#include <Wire.h>
#include "TSL2561.h"
#include <LCD12864RSPI.h>
#include "GP2Y1010AU0F.h"
#include <Rtc_Pcf8563.h>
Rtc_Pcf8563 rtc;
double data[4];                        //store 4 types of climate data
double sum[4];                         //store the sum of 4 types of climate data
int count = 0;                         //record the times
/************* WIFI define ****************** *************/
#define PRODUCT_ID  "1000129322"                      //product ID
#define PRODUCT_KEY "3474667248552863269a22af6d36649b"//product code
#define PRODUCT_VER "01.01"                           //product version
QLCloud    qlcloud;
bool is_wifi_init = false;
/**************************************************************/

/************* SD card define *******************************/
const int chipSelect = 53;                            //CS pin of sd module
/**************************************************************/

/************* LCD define **********************************/
#define AR_SIZE( a ) sizeof( a ) / sizeof( a[0] )
unsigned char show1[] = "weather station";
unsigned char show2[] = "Yan Danli";
/**************************************************************/

/************* DHT11 define **********************************/
#define DHTPIN       14
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
/**************************************************************/

/************* dust sensor define **********************************/
#define PIN_LED 30
#define PIN_OUTPUT A0
GP2Y1010AU0F GP2Y1010AU0F(PIN_LED, PIN_OUTPUT);
/**************************************************************/

/************* light sensor define **********************************/
TSL2561 tsl(TSL2561_ADDR_FLOAT);
/**************************************************************/

int32_t SerialSendBytes(uint8_t*data, int32_t len)
{
  Serial.write(data, len);
}
/*initialize the Wi-Fi module*/
void init_wifi()
{
  qlcloud.init((uint8_t*)PRODUCT_ID, (uint8_t*)PRODUCT_KEY, (uint8_t*)PRODUCT_VER);
}
void upload_data(char *key, double weather) {
  int ret = 0;
  char value[16];
  itoa(weather, value, 10);
  qlcloud.upload_data((uint8_t*)key, strlen(key), (uint8_t*)value, strlen(value));
}

/*query the status*/
void query_satus_cb(int32_t status)
{
  if (status == STATUS_NOT_INIT) {             //initialization finish
    is_wifi_init = false;
    init_wifi();
  } else {                                      //initialization unfinish
    is_wifi_init = true;
  }
}

/*timer*/
void timer_handle()
{
  static int number = 0;

  if (!is_wifi_init) { //wifi uninitialized
    qlcloud.query_module_status();
  }
  else {
    switch (number) { //send climate data every 10s
      case 0:
        upload_data("temperature", data[0]);
        number = 1;
        break;
      case 1:
        upload_data("humidity", data[1]);
        number = 2;
        break;
      case 2:
        upload_data("dust", data[2]);
        number = 3;
        break;
      case 3:
        upload_data("light", data[3]);
        number = 0;
        break;
    }
  }
}
/*data aquisition function*/
void collect_data() {
  data[0] = (int)dht.readTemperature();                  //read temperature

  data[1] = (int)dht.readHumidity();                     //read humidity

  data[2] = (double)  GP2Y1010AU0F.getDustDensity();     //read dusy density

  uint32_t lum = tsl.getFullLuminosity();
  uint16_t ir, full;
  ir = lum >> 16;
  full = lum & 0xFFFF;
  data[3] = (double)tsl.calculateLux(full, ir);          //read light Luminosity
}

/*calculate average function*/
void cal_average() {
  for (int i = 0; i < 4; i++) {                           //process each data from data[0] to data[3]
    sum[i] = sum[i] + data[i];                            //get the sum of 6 samples
    if (count >= 6) {
      data[i] = sum[i] / 6;                               //calculate the average value
      sum[i] = 0;                                         //reset sum
    }
  }
}

/*sd card storage function*/
void sd_storage() {

  String dataString = "";                                   //define an arrary to store imformation
  for (int i = 0; i < 4; i++) {
    dataString += String(data[i]);
    dataString += ",";
  }                                                         //write 4 types of climate data into the array
  dataString += String(rtc.formatDate());                   //write date into the array
  dataString += ",";
  dataString += String(rtc.formatTime(2));                  //write time into the array
  File dataFile = SD.open("datalog.txt", FILE_WRITE);       //open "datalog.txt"writing configuration"
  if (dataFile) {
    dataFile.println(dataString);                           //send datastring to the file
    dataFile.close();                                       //close the file
  }
}

/*LCD display function*/
void lcd_display() {

  LCDA.CLEAR();                                                                                 //clear the screen
  LCDA.DisplayString(0, 0, (unsigned char *)rtc.formatTime(2), 3 * AR_SIZE(rtc.formatTime(2))); //diaplay the time

  int grade = GP2Y1010AU0F.getGradeInfo(data[2]);                                               //display the pollution status
  char *grade_show[] = {"perfect", "good", "light", "medium", "heavy", "severe"};
  LCDA.DisplayString(0, 5, (unsigned char *)grade_show[grade], 3 * AR_SIZE(grade_show[grade]));

  char *name_show[] = {"T(C) :", "H(%) :", "PM   :"};                                           //display the value of temperature, humidity and dust density
  char data_show[3];
  for (int j = 0; j < 3; j++) {
    dtostrf(data[j], 3, 0, data_show);
    LCDA.DisplayString(j + 1, 1, (unsigned char *)name_show[j], 3 * AR_SIZE(name_show[j]));
    LCDA.DisplayString(j + 1, 5, (unsigned char *)data_show, AR_SIZE(data_show));
  }
}

void setup()
{

  Serial.begin(115200);
  qlcloud.reg_send_bytes_cb(SerialSendBytes);    //register the callback function of sending serial bytes
  qlcloud.reg_query_status_cb(query_satus_cb);   //register the callback function of querying status
  qlcloud.set_wifi_debug_status(1);
  dht.begin();                                   //initialize temp/humi seneor
  tsl.begin();                                   //initialize light sensor
  tsl.setGain(TSL2561_GAIN_16X);
  tsl.setTiming(TSL2561_INTEGRATIONTIME_13MS);
  SD.begin(chipSelect);                          //initialize SD card module
  LCDA.Initialise();                             //initialize LCD display
  LCDA.DisplayString(1, 0, show1, AR_SIZE(show1));
  LCDA.DisplayString(3, 2, show2, AR_SIZE(show2));
  MsTimer2::set(10000, timer_handle);            //set a timer to upload data every 10 seconds
  MsTimer2::start();
  delay(5000);
}

void loop()
{
  count++;
  collect_data();                                // collect all the climate data
  lcd_display();                                 //display real-time data
  cal_average();                                 //calculate the average value of each data
  if (count >= 6) {                              //transsmit average data to sd storage
    sd_storage();
    count = 0;
  }
  int available_num = Serial.available();
  while (available_num--) {
    qlcloud.rx_byte(Serial.read());
  }                                              //read serial port buffer

  qlcloud.rx_data_handler();                     //handle serial port data
  delay(10000);

}
