#define BLYNK_PRINT Serial

#define BLYNK_TEMPLATE_ID "TMPLFDVFJRLq"
#define BLYNK_TEMPLATE_NAME "Display for iWSS"
#define BLYNK_AUTH_TOKEN "F6_P24tVhver56G-W7zcB7iM6KL-xg8A"

#include <Wire.h>           
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <ArduinoJson.h> 
#include <WidgetRTC.h>

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "###";
char pass[] = "###";

LiquidCrystal_I2C lcd(0x27,16,2);
WidgetRTC rtc;
BlynkTimer timer;
WiFiClient client;
const int httpPort = 80;
const char* host = "api.openweathermap.org";
String line,condition; 
int temp,humidity,minu,hr,sstat,gstat;

byte therm[8]  = {	0b00100,	0b01010,	0b01010,	0b01010,	0b01110,	0b11111,	0b11111,	0b01110};
byte humi[8] = {	0b00100,	0b01010,	0b01010,	0b10001,	0b10001,	0b10001,	0b01110,	0b00000};

byte Sun1[8] = {	0b00100,	0b00100,	0b00000,	0b01110,	0b11111,	0b11111,	0b11111,	0b01110};
byte Sun2[8] = {	0b00000,	0b01000,	0b10000,	0b00000,	0b00000,	0b11000,	0b00000,	0b00000};
byte Sun3[8] = {	0b00000,	0b00010,	0b00001,	0b00000,	0b00000,	0b00011,	0b00000,	0b00000};
byte Sun4[8] = {	0b00100,	0b00100,	0b00000,	0b00000,	0b00000,	0b00000,	0b00000,	0b00000};
byte Sun5[8] = {	0b10000,	0b01000,	0b00000,	0b00000,	0b00000,	0b00000,	0b00000,	0b00000};
byte Sun6[8] = {	0b00001,	0b00010,	0b00000,	0b00000,	0b00000,	0b00000,	0b00000,	0b00000};

byte cloud1[8] = {	0b00000,	0b00000,	0b00000,	0b11000,	0b11100,	0b11110,	0b11111,	0b11111};
byte cloud2[8] = {	0b00000,	0b00000,	0b00000,	0b00000,	0b00000,	0b00000,	0b11100,	0b11111};
byte cloud3[8] = {	0b00000,	0b00000,	0b00000,	0b00011,	0b00111,	0b01111,	0b11111,	0b11111};
byte cloud4[8] = {	0b11111,	0b11111,	0b01111,	0b00111,	0b00000,	0b00000,	0b00000,	0b00000};
byte cloud5[8] = {	0b11111,	0b11111,	0b11111,	0b11111,	0b00000,	0b00000,	0b00000,	0b00000};
byte cloud6[8] = {	0b11111,	0b11111,	0b11110,	0b11100,	0b00000,	0b00000,	0b00000,	0b00000};

byte rain1[8] = {	0b11111,	0b11111,	0b01111,	0b00111,	0b00000,	0b00010,	0b00010,	0b00000};
byte rain2[8] = {	0b11111,	0b11111,	0b11111,	0b11111,	0b00000,	0b10010,	0b10010,	0b00000};
byte rain3[8] = {	0b11111,	0b11111,	0b11110,	0b11100,	0b00000,	0b01000,	0b01000,	0b00000};

void Time()
{
  minu=minute();
  hr=hour();
  lcd.clear();
  lcd.setCursor(0, 0);
  if(hr<10)
  lcd.print("Time: 0"+String(hr)+":");
  else
  lcd.print("Time: "+String(hr)+":");
  if(minu<10)
  lcd.print("0"+String(minu));
  else
  lcd.print(minu);
  lcd.setCursor(0, 1);
  lcd.print("Date: "+String(day())+"-"+String(month())+"-"+String(year()));
}

void Weather()
{
  if(condition.indexOf("Cloud")!=-1)
  condition="Cloudy";
  else if(condition.indexOf("Sun")!=-1)
  condition="Sunny";
  else if(condition.indexOf("Rain")!=-1)
  condition="Rainy";
  lcd.clear();
  lcd.createChar(6, therm);
  lcd.createChar(7, humi);
  lcd.setCursor(0, 0);
  lcd.write(byte(6));
  lcd.print(String(temp)+(char)223+"C ");
  lcd.write(byte(7));
  lcd.print(String(humidity)+"%");
  lcd.setCursor(0, 1);
  lcd.print("It's "+condition);
  symbol();
}

void Status()
{
  Blynk.syncVirtual(V1,V2);
  lcd.clear();
  if(sstat)
  lcd.print("SO Light= ON");
  else
  lcd.print("SO Light= OFF");
  lcd.setCursor(0, 1);
  if(gstat)
  lcd.print("Gate Lights= ON");
  else
  lcd.print("Gate Lights= OFF");
}

void getWeather() // To get weather data from openweathermap using API
{
  if (!client.connect(host, httpPort)) 
  {
    Serial.println("connection failed");
    return;
  }
  client.println("GET /data/2.5/weather?lat=10.32&lon=76.20&appid=0c8c393379c1faade0b56cb1c3f212f8 HTTP/1.1");
  client.println("Host: api.openweathermap.org");
  client.println("Connection: close");
  client.println();
  delay(1500);
  while(client.available())
  {
    line = client.readStringUntil('\r'); 
  }
  //Serial.println(line);
  StaticJsonBuffer<1200> jsonBuffer;                   
   JsonObject& root = jsonBuffer.parseObject(line);     
   if (!root.success()) 
   {
    Serial.println("parseObject() failed");             
    getWeather();                                         
    return;                                             
   }
  temp = int(root["main"]["temp"])-273.15;                   
  humidity = root["main"]["humidity"]; 
  condition= String(root["weather"][0]["main"]); 
}

void symbol() //Pictorial representation of current weather condition
{
  if(condition=="Sunny")
  {
    lcd.createChar(0, Sun1);
    lcd.createChar(1, Sun2);
    lcd.createChar(2, Sun3);
    lcd.createChar(3, Sun4);
    lcd.createChar(4, Sun5);
    lcd.createChar(5, Sun6);
    lcd.setCursor(14,0);   
    lcd.write(byte(0));
    lcd.setCursor(15,0);   
    lcd.write(byte(1));
    lcd.setCursor(13,0);   
    lcd.write(byte(2));
    lcd.setCursor(14,1);   
    lcd.write(byte(3));
    lcd.setCursor(15,1);   
    lcd.write(byte(4));
    lcd.setCursor(13,1);   
    lcd.write(byte(5));
  }
  else if(condition=="Cloudy")
  {
    lcd.createChar(0, cloud1);
    lcd.createChar(1, cloud2);
    lcd.createChar(2, cloud3);
    lcd.createChar(3, cloud4);
    lcd.createChar(4, cloud5);
    lcd.createChar(5, cloud6);
    lcd.setCursor(14,0);   
    lcd.write(byte(0));
    lcd.setCursor(15,0);   
    lcd.write(byte(1));
    lcd.setCursor(13,0);
    lcd.write(byte(2));
    lcd.setCursor(13,1);  
    lcd.write(byte(3));
    lcd.setCursor(14,1);   
    lcd.write(byte(4));
    lcd.setCursor(15,1);   
    lcd.write(byte(5));
  }
  else if(condition=="Rainy")
  {
    lcd.createChar(0, cloud1);
    lcd.createChar(1, cloud2);
    lcd.createChar(2, cloud3);
    lcd.createChar(3, rain1);
    lcd.createChar(4, rain2);
    lcd.createChar(5, rain3);
    lcd.setCursor(14,0);   
    lcd.write(byte(0));
    lcd.setCursor(15,0);   
    lcd.write(byte(1));
    lcd.setCursor(13,0);
    lcd.write(byte(2));
    lcd.setCursor(13,1);  
    lcd.write(byte(3));
    lcd.setCursor(14,1);   
    lcd.write(byte(4));
    lcd.setCursor(15,1);   
    lcd.write(byte(5));
  }
}

BLYNK_WRITE(V1)
{
  sstat=param.asInt();//to get SO light data from iWSS
}

BLYNK_WRITE(V2)
{
  gstat=param.asInt();//to get Gate lights data from iWSS
}

BLYNK_CONNECTED()
{
  rtc.begin();
  getWeather();
}

void setup() 
{
  Serial.begin(9600);
  Blynk.begin(auth,ssid, pass);
  lcd.init();
  lcd.clear();         
  lcd.backlight();
  lcd.print("iWSS Display");
  delay(2000);
  setSyncInterval(30 * 60);// for RTC
  timer.setInterval(900*1000,getWeather);// get current weather information every 15min
}

void loop() 
{
  Blynk.run();
  timer.run();
  Time();
  delay(3000);
  Weather();
  delay(3000);
  Status();
  delay(3000);
}
