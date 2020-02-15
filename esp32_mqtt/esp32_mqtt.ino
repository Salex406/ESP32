#include <Wire.h>
#include <WiFi.h>
#include <stdlib.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <Stdio.h>
#include <DallasTemperature.h>

WiFiClient cl;
PubSubClient client(cl);

WiFiServer server(80);

const char* ssid = "BestWLAN";
const char* password = "zte30kl7";

const int led_pin = 23;
const uint8_t sht_addr = 0x40;
const uint8_t l_addr = 0x23;
byte addr[8];

void i2c_write(uint8_t reg, uint8_t val, uint8_t number, uint8_t addr)
{
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

uint8_t i2c_read(uint8_t reg, uint8_t addr)
{
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(addr,1);
  uint16_t cnt = 0;
  while(Wire.available() < 1)
  {
    delay(1);
    cnt++;
    if(cnt>1000)
    {
      Serial.println("read error");
      break;
    }
  }
  return Wire.read();
}

float sht_temp()
{
  Wire.beginTransmission(sht_addr);
  Wire.write(0xE3);
  Wire.endTransmission();
  delay(100);

  Wire.requestFrom(sht_addr,3);
  uint16_t cnt = 0;
  while(Wire.available() < 3)
  {
    delay(1);
    cnt++;
    if(cnt>1000)
    {
      Serial.println("SHT error");
      break;
    }
  }
  uint16_t res = (Wire.read()<<8);
  res|=Wire.read();
  res &= ~0x0003;   // clear two low bits (status bits)
  float temp_ = -46.85 + 175.72/65536.0*(float)res;
  return temp_;
}

float sht_hum()
{
  Wire.beginTransmission(sht_addr);
  Wire.write(0xE5);
  Wire.endTransmission();
  delay(100);

  Wire.requestFrom(sht_addr,3);
  uint16_t cnt = 0;
  while(Wire.available() < 3)
  {
    delay(1);
    cnt++;
    if(cnt>1000)
    {
      Serial.println("SHT error");
      break;
    }
  }
  uint16_t res = (Wire.read()<<8);
  res |= Wire.read();
  res &= ~0x0003;   // clear two low bits (status bits)
  float hum_ = -6 + 125/65536.0*(float)res;
  return hum_;
}

float light()
{
  Wire.requestFrom(l_addr,2);
  uint16_t cnt = 0;
  while(Wire.available() < 2)
  {
    delay(1);
    cnt++;
    if(cnt>1000)
    {
      Serial.println("SHT error");
      break;
    }
  }
  uint16_t res = (Wire.read()<<8);
  res |= Wire.read();
  float lux = res/1.2;
  return lux;
}

void callback(char* topic, byte* payload, unsigned int length)
{
  Serial.println("New msg in topic");
  Serial.println(topic);
  for(int i=0;i<length;i++)
  {
    Serial.print(payload[i]);
  }
  if(payload[0]=='1')
  {
    digitalWrite(led_pin, HIGH);
    float temp=sht_temp(),hum=sht_hum();
    Serial.println("temp");
    Serial.println(temp);
    Serial.println("hum");
    Serial.println(hum);

    Serial.println("light");
    Serial.println(light());



  }
  else
  {
    digitalWrite(led_pin, LOW);
  }
}

void reconnect()
{
  char* client_n = "mqtt-iprofi_165438284-xqoa45";
  while(!client.connected())
  {
    Serial.println("Connecting to Rigtech");
    client.connect(client_n);
    delay(5000); 
    Serial.println("State:");
    Serial.println(client.state());
    client.subscribe("base/relay/led1");
  }
}

void setup() {
  
  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, HIGH);
  
  Serial.begin(115200);
  WiFi.begin(ssid,password);
  while(WiFi.status() != WL_CONNECTED)
  {
    delay(300);
    Serial.print(".");
  }
  Serial.println("Connected");
  Wire.begin();
  for(uint8_t i=1;i<127;i++)
  {
    Wire.beginTransmission(i);
    uint8_t error = Wire.endTransmission();
    if(error==0)
    {
      Serial.println("Found at address:");
      Serial.println(i, HEX);
    }
  }
  client.setServer("sandbox.rightech.io", 1883);
  client.setCallback(callback);
  

//  uint8_t* buf = 0x10;
  //i2c_write(0xF4,NULL,l_addr);
 // i2c_write(0x01,(uint8_t*)0x10,1,cc_addr);
 
  Wire.beginTransmission(l_addr);
  Wire.write(0x01);
  Wire.endTransmission();

  delay(100);

  Wire.beginTransmission(l_addr);
  Wire.write(0x11);
  Wire.endTransmission();

  server.begin();
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

unsigned long a = 0;
void loop() {
  // put your main code here, to run repeatedly:
  client.loop();
  WiFiClient client_ = server.available();
  if (client_) { 
            float temp=sht_temp(),hum=sht_hum();
            client_.println("<!DOCTYPE html><html>");
            client_.println("<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">");
            client_.println("<style>html { font-size: 23pt; display: inline-block; margin: 0px auto; text-align: left;}");
            client_.println(".button { background-color: #F88E01; border: none; color: white; padding: 16px 40px;");
            client_.println("text-decoration: none; font-size: 23pt; margin: 2px; cursor: pointer;}");
            client_.println(".button2 {background-color: #AF7221;}</style></head>");
            
            client_.println("<body><h1>Панель управления</h1>");

            client_.println("<h2>Температура: ");
            client_.println(temp);
            client_.println("°С<\h2>");

            client_.println("<h2>Влажность: ");
            client_.println(hum);
            client_.println("%<\h2>");

            client_.println("</body></html>");
            client_.println("");

    }
  
  unsigned long mllis = millis();
  if((mllis - a) > 5000)
  {
    a = mllis;
    char s[10];
    if(!client.connected()) reconnect();
    sprintf(s,"%f",sht_hum());
    client.publish("base/state/humidity",s);
    sprintf(s,"%f",sht_temp());
    client.publish("base/state/temperature",s);
  }
  
}
