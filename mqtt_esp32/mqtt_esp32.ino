#include <ESP8266WiFi.h>
#include <stdlib.h>
#include <PubSubClient.h>

#define STASSID "BestWLAN"
#define STAPSK  "zte30kl7"


IPAddress ip(255,255,255,255);
String mac; 

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
  } else {
    Serial.println("0");
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

void setup() {
  srand(12);
  Serial.begin(115200);
  Serial.println("strting");
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID, STAPSK);
  while (WiFi.status() != WL_CONNECTED) {
    //Serial.print('.');
    delay(500);
  }
  Serial.println("cnnectd");
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
     int temp = rand() % 10 + 18;
  int hum = 100;
  Serial.print("Publish message: ");
    char msg[20];
    sprintf(msg,"%d",temp);
 
    client.publish("base/state/temperature", msg);
  
    sprintf(msg,"%d",hum);
    
    client.publish("base/state/humidity", msg);
  }
  

}
