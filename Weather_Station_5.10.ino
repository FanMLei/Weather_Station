#include <Arduino.h>
#include <dht11.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <WiFi.h>
#include <ArduinoHttpClient.h>
#include "TF.h"
#include "Audio.h"
#include <MqttClient.h>
#define DHT11PIN 2    //定义温湿度传感器接口
int t, h;
#define SR05 3        //定义红外模块接口

char ssid[] = "test";
char pass[] = "qwerqwer";
String web_address = "api.seniverse.com";
const String APIPASSWORD = "your api password"; //

int temperature, temperature_low_day1, temperature_high_day1, temperature_low_day2,
    temperature_high_day2, temperature_low_day3, temperature_high_day3;                 //实时温度，最低、最高温度
int code, code_day1, code_night1, code_day2, code_night2, code_day3, code_night3;       //实时天气，白天、夜晚天气
int wind_scale_day1, wind_scale_day2, wind_scale_day3;                                  //风力等级
String wind_direction_day1, wind_direction_day2, wind_direction_day3;                   //风向
String date1, date2, date3;            //日期
int point_1;                           //设置提醒
String data, data_day1, data_day2, data_day3, suggestion;

const char* wind_direction_voice;          //语音播放需要的变量
const char* sport;
const char* dress;
const char* uv;
char code_voice[5];
char wind_scale_voice[5];
char temperature_dth11[5];
char temperature_voice[5];
char temperature_low_voice[5];
char temperature_high_voice[5];
char audio_buffer[20] ;
char wind_buffer[20];

WiFiClient wifi;
HttpClient http(wifi, web_address);
dht11 DHT11;

void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600);
  SD.begin();
  Audio.begin(AUDIO_SD);
  Audio.playFile("\\voice\\begin.wav");
  while (WiFi.begin(ssid, pass) != WL_CONNECTED);
  if (Audio.getStatus() == AudioStop)
  {
    Audio.playFile("\\voice\\connected.wav");
    while (Audio.getStatus() != AudioStop);
  }
  Serial1.print("page 1");
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);
}

void loop()
{
  get_data();
  get_temperature();
  //判断是否接受到数据，并播放相应的语音
  if ( data.equals("") && data_day1.equals("") && data_day2.equals("") && data_day3.equals(""))
    Audio.playFile("\\voice\\datafailed.wav");
  else
    Audio.playFile("\\voice\\dataover.wav");
  while(Audio.getStatus() != AudioStop);
  USart_HMI_weather(); //发送数据
  audio();  //播放语音
  delay(20000);
}

/**************获取所有的天气信息****************/
void get_data()
{
  /***************获取天气实况*******************/
  int httpCode = 0;
  String httpData;
  httpCode = http.get("/v3/weather/now.json?key=" + APIPASSWORD + "&location=zhengzhou&language=en&unit=c");
  if ( httpCode == 0)
  {
    httpCode = http.responseStatusCode();
    if (httpCode >= 0)
    {
      int bodyLen = http.contentLength();
      while ( (http.connected() || http.available()) && (!http.endOfBodyReached()))
      {
        if (http.available())
        {
          char c = http.read();
          httpData += c;
        }
        else
          delay(1000);
      }
      data = httpData.substring((httpData.indexOf("\"now\":") + 6), httpData.indexOf(",\"last"));
      DynamicJsonBuffer jsonBuffer;
      JsonObject& root = jsonBuffer.parseObject(data);
      temperature = root[String("temperature")];
      code = root[String("code")];
      sprintf(temperature_voice, "%d", temperature);
    }
  }
  http.stop();
  delay(100);
  /*****************第一天信息*****************/
  httpCode = 0;
  String httpData_day1;
  httpCode = http.get("/v3/weather/daily.json?key=" + APIPASSWORD + "&location=zhengzhou&language=en&unit=c&start=0&days=1");
  if ( httpCode == 0)
  {
    httpCode = http.responseStatusCode();
    if (httpCode >= 0)
    {
      int bodyLen = http.contentLength();
      while ( (http.connected() || http.available()) && (!http.endOfBodyReached()))
      {
        if (http.available())
        {
          char c = http.read();
          httpData_day1 += c;
        }
        else
          delay(1000);
      }
      data_day1 = httpData_day1.substring((httpData_day1.indexOf("\"daily\"") + 9), httpData_day1.indexOf("],\"last"));
      DynamicJsonBuffer jsonBuffer;
      JsonObject& root = jsonBuffer.parseObject(data_day1);
      date1 = root[("date")].as<String>();
      temperature_low_day1 = root[String("low")];
      temperature_high_day1 = root[String("high")];
      code_day1 = root[String("code_day")];
      code_night1 = root[String("code_night")];
      wind_direction_day1 = root[("wind_direction")].as<String>();
      wind_scale_day1 = root[String("wind_scale")];
      //数据转换
      wind_direction_voice = root[("wind_direction")];
      sprintf(code_voice, "%d", code_day1);
      sprintf(wind_scale_voice, "%d", wind_scale_day1);
      sprintf(temperature_low_voice, "%d", temperature_low_day1);
      sprintf(temperature_high_voice, "%d", temperature_high_day1);
      strcat (wind_buffer, "\\voice\\");
      strcat(wind_buffer, wind_direction_voice);
      strcat(wind_buffer, wind_scale_voice);
      strcat(wind_buffer, ".wav");
    }
  }
  http.stop();
  delay(100);
  /*****************第二天信息*****************/
  httpCode = 0;
  String httpData_day2;
  httpCode = http.get("/v3/weather/daily.json?key=" + APIPASSWORD + "&location=zhengzhou&language=en&unit=c&start=1&days=1");
  if ( httpCode == 0)
  {
    httpCode = http.responseStatusCode();
    if (httpCode >= 0)
    {
      int bodyLen = http.contentLength();
      while ( (http.connected() || http.available()) && (!http.endOfBodyReached()))
      {
        if (http.available())
        {
          char c = http.read();
          httpData_day2 += c;
        }
        else
          delay(1000);
      }
      data_day2 = httpData_day2.substring((httpData_day2.indexOf("\"daily\"") + 9), httpData_day2.indexOf("],\"last"));
      DynamicJsonBuffer jsonBuffer;
      JsonObject& root = jsonBuffer.parseObject(data_day2);
      date2 = root[("date")].as<String>();
      temperature_low_day2 = root[String("low")];
      temperature_high_day2 = root[String("high")];
      code_day2 = root[String("code_day")];
      code_night2 = root[String("code_night")];
      wind_direction_day2 = root[("wind_direction")].as<String>();
      wind_scale_day2 = root[String("wind_scale")];
    }
  }
  http.stop();
  delay(100);
  /*****************第三天信息*****************/
  httpCode = 0;
  String httpData_day3;
  httpCode = http.get("/v3/weather/daily.json?key=" + APIPASSWORD + "&location=zhengzhou&language=en&unit=c&start=2&days=1");
  if ( httpCode == 0)
  {
    httpCode = http.responseStatusCode();
    if (httpCode >= 0)
    {
      int bodyLen = http.contentLength();
      while ( (http.connected() || http.available()) && (!http.endOfBodyReached()))
      {
        if (http.available())
        {
          char c = http.read();
          httpData_day3 += c;
        }
        else
          delay(1000);
      }
      data_day3 = httpData_day3.substring((httpData_day3.indexOf("\"daily\"") + 9), httpData_day3.indexOf("],\"last"));
      DynamicJsonBuffer jsonBuffer;
      JsonObject& root = jsonBuffer.parseObject(data_day3);
      date3 = root[("date")].as<String>();
      temperature_low_day3 = root[String("low")];
      temperature_high_day3 = root[String("high")];
      code_day3 = root[String("code_day")];
      code_night3 = root[String("code_night")];
      wind_direction_day3 = root[("wind_direction")].as<String>();
      wind_scale_day3 = root[String("wind_scale")];
    }
  }
  http.stop();
  delay(100);
  /**************获取生活指数**************/
  httpCode = 0;
  String httpData_suggestion;
  httpCode = http.get("/v3/life/suggestion.json?key=" + APIPASSWORD + "&location=zhengzhou&language=en");
  if ( httpCode == 0)
  {
    httpCode = http.responseStatusCode();
    if (httpCode >= 0)
    {
      int bodyLen = http.contentLength();
      while ( (http.connected() || http.available()) && (!http.endOfBodyReached()))
      {
        if (http.available())
        {
          char c = http.read();
          httpData_suggestion += c;
        }
        else
          delay(1000);
      }
      suggestion = httpData_suggestion.substring((httpData_suggestion.indexOf("\"dressing\"") + 11), httpData_suggestion.indexOf(",\"flu"));
      DynamicJsonBuffer jsonBuffer1;
      JsonObject& root1 = jsonBuffer1.parseObject(suggestion);
      dress = root1[("brief")];
      Serial.println(dress);
      suggestion = httpData_suggestion.substring((httpData_suggestion.indexOf("\"sport\"") + 8), httpData_suggestion.indexOf(",\"travel"));
      DynamicJsonBuffer jsonBuffer2;
      JsonObject& root2 = jsonBuffer2.parseObject(suggestion);
      sport = root2[("brief")];
      Serial.println(sport);
      suggestion = httpData_suggestion.substring((httpData_suggestion.indexOf("\"uv\":") + 5), httpData_suggestion.indexOf("},\"last"));
      DynamicJsonBuffer jsonBuffer3;
      JsonObject& root3 = jsonBuffer3.parseObject(suggestion);
      uv = root3[("brief")];
      Serial.println(uv);
    }
  }
  http.stop();
  delay(100);
}

/**************DTH11******************/
void get_temperature()
{
  int chk = DHT11.read(DHT11PIN);
  h = DHT11.humidity;
  t = DHT11.temperature;
  sprintf(temperature_dth11, "%d", t);
  Serial.println(t);
  if (abs(temperature - t) >= 10)
    point_1 = 1;
}

/*************语音播报************/
void audio()
{
  strcat (audio_buffer, "\\voice\\");
  strcat(audio_buffer, code_voice);
  strcat(audio_buffer, ".wav");
  Audio.playFile(audio_buffer);
  while (Audio.getStatus() != AudioStop); 
  if (Audio.getStatus() == AudioStop)
  {
    memset(audio_buffer, '\0', sizeof(audio_buffer));
    strcat(audio_buffer, "\\voice\\");
    strcat(audio_buffer, temperature_high_voice);
    strcat(audio_buffer, "th.wav");
    Audio.playFile(audio_buffer);
    while (Audio.getStatus() != AudioStop);
    if (Audio.getStatus() == AudioStop)
    {
      memset(audio_buffer, '\0', sizeof(audio_buffer));
      strcat(audio_buffer, "\\voice\\");
      strcat(audio_buffer, temperature_low_voice);
      strcat(audio_buffer, "tl.wav");
      Audio.playFile(audio_buffer);
      while (Audio.getStatus() != AudioStop);     
      if (Audio.getStatus() == AudioStop)
      {  
        memset(audio_buffer, '\0', sizeof(audio_buffer));    
        Audio.playFile(wind_buffer);
        while (Audio.getStatus() != AudioStop);
        if (Audio.getStatus() == AudioStop)
        {
          memset(wind_buffer, '\0', sizeof(wind_buffer));
          strcat (audio_buffer, "\\voice\\");
          strcat(audio_buffer, temperature_voice);
          strcat(audio_buffer, "to.wav");
          Audio.playFile(audio_buffer);
          while(Audio.getStatus() != AudioStop);
          if(Audio.getStatus() == AudioStop)
          {
            memset(audio_buffer, '\0', sizeof(audio_buffer));
            strcat (audio_buffer, "\\voice\\");
            strcat(audio_buffer, temperature_dth11);
            strcat(audio_buffer, "ti.wav");
            Audio.playFile(audio_buffer);
            while(Audio.getStatus() != AudioStop);
            memset(wind_buffer, '\0', sizeof(wind_buffer));
          }         
        }
      }
    }
  }
}
/**********显示界面************/
void USart_HMI_weather()
{
  /********切换到Weather页面*********/

  /********切换天气图片*********/
  Serial1.print("picq 0,30,180,210,");
  Serial1.print(code);
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);
  /*******实时天气状况*******/
  Serial1.print("va0.val=");
  Serial1.print(code);
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.print("click t2,1");
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);
  /********实时温度*******/
  Serial1.print("t0.txt=\"");
  Serial1.print(temperature);
  Serial1.print("\"");
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.print("vis t1,1");
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);
  /********滚动显示当天详细天气信息*******/
  Serial1.print("g0.txt=\"\"");
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.print("g0.txt=\"");
  Serial1.print(temperature_low_day1);
  Serial1.print("-");
  Serial1.print(temperature_high_day1);
  Serial1.print("\"");
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.print("va1.val=");
  Serial1.print(code_day1);
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.print("va2.val=");
  Serial1.print(code_night1);
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.print("va3.txt=\"");
  Serial1.print(wind_direction_day1);
  Serial1.print("\"");
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.print("va4.val=");
  Serial1.print(wind_scale_day1);
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.print("click g0,1");
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);
  /********第一天日期显示*********/
  Serial1.print("t13.txt=\"");
  Serial1.print(date1);
  Serial1.print("\"");
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.print("vis t14,1");
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);
  /********第二天日期显示*********/
  Serial1.print("t5.txt=\"");
  Serial1.print(date2);
  Serial1.print("\"");
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);
  /********第三天日期显示*********/
  Serial1.print("t9.txt=\"");
  Serial1.print(date3);
  Serial1.print("\"");
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);
  /*********第二天最低最高温度********/
  Serial1.print("t6.txt=\"");
  Serial1.print(temperature_low_day2);
  Serial1.print("-");
  Serial1.print(temperature_high_day2);
  Serial1.print("\"");
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.print("click t6,1");
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);
  /********第三天的最低最高温度*******/
  Serial1.print("t10.txt=\"");
  Serial1.print(temperature_low_day3);
  Serial1.print("-");
  Serial1.print(temperature_high_day3);
  Serial1.print("\"");
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.print("click t10,1");
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);
  /********第二天天气状况*********/
  Serial1.print("va5.val=");
  Serial1.print(code_day2);
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.print("click t7,1");
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.print("va6.txt=\"");
  Serial1.print(wind_direction_day2);
  Serial1.print("\"");
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.print("va7.val=");
  Serial1.print(wind_scale_day2);
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.print("click t8,1");
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);
  /********第三天天气状况********/
  Serial1.print("va8.val=");
  Serial1.print(code_day1);
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.print("click t11,1");
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.print("va9.txt=\"");
  Serial1.print(wind_direction_day3);
  Serial1.print("\"");
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.print("va10.val=");
  Serial1.print(wind_scale_day3);
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.print("click t12,1");
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);
}

