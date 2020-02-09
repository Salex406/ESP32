
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <Wire.h>
#include "SHT21.h"

#define RXD2 16
#define TXD2 17
#define WAIT_READ_TIMES 100
#define WAIT_READ_DELAY 10

int co2ppm() {
  static byte cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
  static byte response[9] = {0};

  Serial2.write(cmd, 9);
  Serial2.readBytes(response, 9);

  unsigned int responseHigh = (unsigned int) response[2];
  unsigned int responseLow = (unsigned int) response[3];

  return (256 * responseHigh) + responseLow;
}

const char* ssid     = "Stoikost";
const char* password = "korobo4ka";
const int oneWireBus1 = 5;
const int oneWireBus2 = 22;
const int oneWireBus3 = 19;   
const int RelayPin = 2;
const int SensorPin1 = 32;

const int rx_pin = 13; //Serial rx pin no
const int tx_pin = 15; //Serial tx pin no
//const int SensorPin2 = 33;

// Текущее состояние устройств
String Relay1State = "off";
float sensorValue1 = 0;
int sensorValue2 = 0;
float temperatureC1 = 0;
float temperatureC2 = 0;
int co2 = 0;
WiFiServer server(80);
String header;

SHT21 SHT21;

void setup() {

  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial.begin(115200);
  pinMode(RelayPin, OUTPUT);
  //pinMode(SensorPin1, INPUT);
  digitalWrite(RelayPin, LOW);
  //analogReadResolution(11);
  //analogSetAttenuation(ADC_6db);
  SHT21.begin();

  //mySerial.begin(9600);
  //myMHZ19.begin(mySerial);
  
  
  //Подключение к WIFI
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  //Вывод полученного IP в UART (по этому адресу веб-интервейс)
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
  Serial.println("Ready");
}

void loop() {
  delay(1000);
  //Начать принимать подключения
  WiFiClient client = server.available();
  if (client) {
    String currentLine = "";     
    //Пока клиент подключен(есть данные для считывания)   
    while (client.connected()) {
      //Если посланные данные больше нуля
      if (client.available()) {
        //Читаем данные посимвольно
        char c = client.read(); 
        //header содержит все данные клинта(браузер, ОС и т.д). currentline - текущас строка     
        header += c;
        //Если конец строки запроса проверяем, была ли эта строка последней
        if (c == '\n') {
          //Serial.println(currentLine);
          //Если текущая строка есть только перенос строки, значит она последняя и можно выполнять обработку   
          if (currentLine.length() == 0) {
            //sensors1.requestTemperatures(); 
            //temperatureC1 = sensors1.getTempCByIndex(0);
            /*sensors2.requestTemperatures(); 
            temperatureC2 = sensors2.getTempCByIndex(0);
            sensors3.requestTemperatures(); 
            temperatureC3 = sensors3.getTempCByIndex(0);*/
            sensorValue1 = analogRead(SensorPin1);
          
           // sensorValue2 = analogRead(SensorPin2);
            
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
             //Обработка команды от клиента. IndexOF осуществляет поиск подстроки в запросе
             if (header.indexOf("GET /on/l1") >= 0) {
                Serial.print("R1on");
                Relay1State = "on";
                digitalWrite(RelayPin,HIGH);}
             if (header.indexOf("GET /off/l1") >= 0) {
                Serial.print("R1off");
                Relay1State = "off";
                digitalWrite(RelayPin,LOW);}
                
            // Отображение страницы
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">");
            client.println("<style>html { font-size: 23pt; display: inline-block; margin: 0px auto; text-align: left;}");
            client.println(".button { background-color: #F88E01; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 23pt; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #AF7221;}</style></head>");
            
            client.println("<body><h1>Панель управления</h1>");

            client.println("<h2>Температура: ");
            client.println(temperatureC1);
            client.println("°С<\h2>");

            client.println("<h2>Влажность: ");
            client.println(temperatureC2);
            client.println("%<\h2>");

            client.println("<h2>CO2: ");
            client.println(co2);
            client.println("ppm<\h2>");
            
            client.println("<h2>Сенсор: ");
            client.println(sensorValue1);
            client.println("<\h2>");

            
            client.println("<h2>Реле: </h2>");

            //Отображение кнопок в зависимости отсостояния устр-ва
            client.println("<p>");   
            if (Relay1State=="off")client.println("<a href=\"/on/l1\"><button class=\"button\">1: ON</button></a>");
            else client.println("<a href=\"/off/l1\"><button class=\"button button2\">1: OFF</button></a>");

            
            client.println("</body></html>");
            client.println("");
            
          
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          //заполняем текущую строку запроса
          currentLine += c;      
        }
      }
    }
    
    header = "";
    //Разорвать соединение с клиентом после того как прочитали все данные от него
    client.stop();
  }
  else
    {
   
     co2 = co2ppm();
     temperatureC2 = SHT21.getHumidity();
     temperatureC1 = SHT21.getTemperature();
     sensorValue1 = analogRead(SensorPin1);
     delay(2000);
    }
}
