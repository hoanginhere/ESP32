#include <Adafruit_GFX.h>    
#include <Adafruit_ST7789.h>
#include <RTClib.h>
#include "SHTSensor.h"
#define TFT_RST 2 // we use the seesaw for resetting to save a pin
#define TFT_CS 5
#define TFT_DC 4
#define RELAY 27
//SCL = D18 SDA =D23

SHTSensor sht;

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
RTC_DS3231 rtc;
String daysOfTheWeek[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
unsigned long alarmOnTime = 61000;
unsigned long t_ime;
int bt_van=25;//Bt1
// int bt_exit=26;//Bt2
int bt_hour=32;//Bt3
int bt_minute=33;//Bt4
int bt_on=34;//Bt5
int bt_off=35;//Bt6
int hour=0;
int minute=0;
bool alarm_state=0;
bool relay_state = 0;
bool executed =0;
bool dem =0;
int sensor_soil=14;
int set_humidity=25;

//bien cho cam bien luu luong
volatile int flow_frequency; // Đo xung cảm biến lưu lượng
unsigned int l_hour,vol ; // Tính toán số lít/giờ
unsigned char flowsensor = 13; // Cảm biến nối với chân 2
unsigned long currentTime;
unsigned long cloopTime;

void flow () // Hàm ngắt
{
   flow_frequency++;
}
int water_flow()
{
  currentTime = millis();
   // Every second, calculate and print litres/hour
   if (currentTime >= (cloopTime + 1000))
   {
     cloopTime = currentTime; // Updates cloopTime
     // Pulse frequency (Hz) = 7.5Q, Q is flow rate in L/min.
     l_hour = (flow_frequency * 60 / 7.5); // (Pulse frequency x 60 min) / 7.5Q = flowrate in L/hour
     flow_frequency = 0;                   // Reset Counter
    
   }
  return l_hour;
}
// doc cam bien do am dat
int humidity_soil(){
  int doc_cambien = analogRead(sensor_soil); 
  int phantram =100- map(doc_cambien,0,4095,1,100);
  return phantram;
}
// bat tat relay
void relay_update(){
  tft.fillRect(200, 200, 50, 50, ST77XX_BLACK);
  tft.setCursor(200,200);
  tft.setTextSize(2);
  if(relay_state==1){
    digitalWrite(RELAY,HIGH);
    tft.println("OFF ");
  }else {
    digitalWrite(RELAY,LOW);
    tft.println("ON");
  }
}

//bat tat hen gio
void alarm_update(){
  if(alarm_state==1){
  DateTime now = rtc.now();
  if(now.hour()==hour && now.minute()==minute)
      if(executed!=1)
      {
        executed=!executed;
        relay_state=1;
        relay_update();
        t_ime=millis();
        }

  if( executed!=0 && millis()-t_ime>=alarmOnTime )
    {
      executed=!executed;
      relay_state=0;
      relay_update();
    }
  }

}
//hien thi lcd
void display(){
  DateTime now = rtc.now();
  tft.fillRect(10, 90, 160, 40, ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(3);
  tft.setCursor(20,10);
  tft.println(daysOfTheWeek[now.dayOfTheWeek()]);
  tft.setCursor(20,50);
  tft.print(now.year(), DEC);
  tft.print('/');
  tft.print(now.month(), DEC);
  tft.print('/');
  tft.print(now.day(), DEC);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(3);
  tft.setCursor(20,90);
  tft.print(now.hour(), DEC);
  tft.print(':');
  tft.print(now.minute(), DEC);
  tft.print(':');
  tft.print(now.second(), DEC);
  if (sht.readSample()) {
    tft.fillRect(236, 10, 75, 110, ST77XX_BLACK);
    tft.setCursor(200,10);
    tft.setTextSize(2);
    tft.print("RH:");
    tft.print(sht.getHumidity(), 2);
    tft.setCursor(200,40);
    tft.print("T: ");
    tft.print(sht.getTemperature(), 2);
  } else {
      Serial.print("Error in readSample()\n");
  }

  tft.setCursor(200,70);
  tft.print("SH:");
  tft.print(humidity_soil());
  tft.print(" %");

  tft.setCursor(200,100);
      tft.setTextSize(2);
      tft.print("WF:");
      tft.print(water_flow()); // Print litres/hour
      tft.setTextSize(1);
      tft.println(" L/h");
   
}
void IRAM_ATTR set_alarm_hour(){
  hour++;
  if(hour>=24){
    hour=0;
  }
}
void IRAM_ATTR set_alarm_minute(){
  minute=minute+10;
  if(minute>=60){
    minute=0;
  }
}
void set_alram(){
  tft.setCursor(20,150);
  tft.setTextSize(2);
  tft.print("Alarm:");
  tft.setCursor(20,200);
  tft.setTextSize(3);
  tft.print("  :  ");
  tft.fillRect(20, 200, 40, 40, ST77XX_BLACK);
  tft.setCursor(20,200);
  tft.print(hour);
  tft.fillRect(80, 200, 50, 50, ST77XX_BLACK);
  tft.setCursor(80,200);
  tft.print(minute);
}


void setup() {
  Serial.begin(9600);
  pinMode(sensor_soil,INPUT);
  pinMode(bt_van, INPUT);
  pinMode(bt_hour,INPUT);
  pinMode(bt_minute,INPUT);
  pinMode(bt_on,INPUT);
  pinMode(bt_off,INPUT);
  pinMode(bt_van,INPUT);
  pinMode(RELAY,OUTPUT);
  tft.init(240, 320); // chả biết này hàm gì, chắc là khởi tạo
  tft.fillScreen(ST77XX_BLACK);
  tft.setRotation(3);
  tft.drawRect(190, 0, 130, 125,ST77XX_RED);
  tft.setTextColor(ST77XX_GREEN);
  // tft.setTextSize(2);
  // tft.setCursor(0, 0);

  tft.setCursor(200,150);
  tft.setTextSize(2);
  tft.print("VAN");
  attachInterrupt(bt_hour,set_alarm_hour, RISING);
  attachInterrupt(bt_minute,set_alarm_minute, RISING);
  if (!rtc.begin()) {
      Serial.println(F("Couldn't found RTC"));
      while (1);
   }
  
   // nếu mất điện thì đặt ngày giờ
   if (rtc.lostPower()) {
      // đặt ngày và giờ (tổng quát)
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
      
      // ví dụ đặt ngày  2021-01-01 giờ 00:00:00
      // rtc.adjust(DateTime(2021, 1, 1, 0, 0, 0));
   }
  //setup sensor sht30
if (sht.init()) {
      Serial.print("init(): success\n");
  } else {
      Serial.print("init(): failed\n");
  }
  sht.setAccuracy(SHTSensor::SHT_ACCURACY_MEDIUM); // only supported by SHT3x
    t_ime=millis();
//setup cho cảm biến lưu lượng
   pinMode(flowsensor, INPUT);
   digitalWrite(flowsensor, HIGH); // Optional Internal Pull-Up
   attachInterrupt(flowsensor, flow, RISING); // Setup Interrupt
   sei(); // Enable interrupts
   currentTime = millis();
   cloopTime = currentTime;
}

void loop() {
//  tft.fillScreen(ST77XX_WHITE);
  DateTime now = rtc.now();

  display();
  set_alram();
  if (digitalRead(bt_on) == LOW)
  {
      alarm_state = 1;
  }
  else if (digitalRead(bt_off) == LOW)
  {
      alarm_state = 0;
  }
  if(alarm_state==1){
    tft.setCursor(100,150);
    tft.setTextSize(2);
    tft.fillRect(100, 150, 40, 40, ST77XX_BLACK);
    tft.print("A");
      if (set_humidity <= humidity_soil())
      {
      relay_state = 1;
      relay_update();
      }
      else
      {
      relay_state = 0;
      relay_update();
      }
  }
  else if (alarm_state == 0)
  {
    tft.setCursor(100,150);
    tft.setTextSize(2);
    tft.fillRect(100, 150, 40, 40, ST77XX_BLACK);
    tft.print("M");
    if (digitalRead(bt_van) == LOW)
    {
    dem = !dem;
    }
    Serial.print(dem);
    if (dem == 0)
    {
    relay_state = 1;
    relay_update();
    }
    if (dem != 0)
    {
    relay_state = 0;
    relay_update();
    }
  }

  // Serial.print("\n");
  // Serial.print(alarm_state);
// chạy cảm biến lưu lượng

  water_flow();
  alarm_update();
  delay(500);
}
