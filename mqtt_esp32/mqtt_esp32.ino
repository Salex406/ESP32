#include <ESP8266WiFi.h>
#include <stdlib.h>
#include <PubSubClient.h>
#include <Wire.h>


#define STASSID "BestWLAN"
#define STAPSK  "zte30kl7"

#define sht_write 0x80 
#define sht_read 0x81 
#define sht_addr 0x40 
#define sht_temp 0xe3
#define sht_hum 0xe5
 

IPAddress ip(255,255,255,255);
String mac; 
const int Pin1 = 16;

const char* mqtt_server = "sandbox.rightech.io";
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    Serial.println("1");
     digitalWrite(Pin1, HIGH); 
  } else {
    Serial.println("0");
     digitalWrite(Pin1, LOW); 
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    String clientId = "mqtt-iprofi_165438284-xqoa45";

    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("base/relay/led1");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


uint16_t readSensor(uint8_t command)
{
    uint16_t result;

    Wire.beginTransmission(sht_addr); //begin
    Wire.write(command);                     //send the pointer location
    Wire.endTransmission();                  //end
    
    //Hang out while measurement is taken. 50mS max, page 4 of datasheet.
    delay(55);

    //Comes back in three bytes, data(MSB) / data(LSB) / Checksum
    Wire.requestFrom(sht_addr, 3);
    
    //Wait for data to become available
    int counter = 0;
    while(Wire.available() < 3)
    {
        counter++;
        delay(1);
        if(counter > 100) return 1; //Error timout
    }

    //Store the result
    result = ((Wire.read()) << 8);
    result |= Wire.read();
   
    result &= ~0x0003;   // clear two low bits (status bits)
    return result;
}

float GetTemperature(void)
{
    return (-46.85 + 175.72 / 65536.0 * (float)(readSensor(sht_temp)));
}

float GetHumidity(void)
{
    return (-6 + 125 / 65536.0 * (float)(readSensor(sht_hum)));
}

void setup() {
  srand(12);
  Serial.begin(115200);
  Serial.println("strting");
  
  Wire.begin();
  
  pinMode(Pin1, OUTPUT);
  digitalWrite(Pin1, HIGH); 

  
  WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID, STAPSK);
  while (WiFi.status() != WL_CONNECTED) {
    //Serial.print('.');
    delay(500);
  }
  Serial.println("WIFI cnnectd");
  Serial.println(WiFi.localIP());
  mac = WiFi.macAddress();
  Serial.println(mac);



  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
   if (!client.connected()) {
    reconnect();
  }
  client.loop();

 unsigned long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    ++value;
     //int temp = rand() % 10 + 18;
  //int hum = 100;
    float temp = GetTemperature();
    char dat[10];
    sprintf(dat,"%f",temp);
    Serial.println(dat);
    client.publish("base/state/temperature", dat);
    
    float hum = GetHumidity();
    sprintf(dat,"%f",hum);
    Serial.println(dat);
    Serial.println("Published");
    //char msg[20];
    //sprintf(msg,"%d",temp);
 
  
    //sprintf(msg,"%d",hum);
    
    client.publish("base/state/humidity", dat);
  }
  

}

void loop_() {
  byte error, address;
  int nDevices;
  Serial.println("Scanning...");
  nDevices = 0;
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
      nDevices++;
    }
    else if (error==4) {
      Serial.print("Unknow error at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0) {
    Serial.println("No devices found\n");
  }
  else {
    Serial.println("done!\n");
  }
  delay(5000);          
}
