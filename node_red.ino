#include <ESP8266WiFi.h> 
#include <PubSubClient.h>
#include "DHT.h"
#define DHTTYPE DHT11 //กำหนดรุ่นของ DHT
#define mqtt_server "192.168.1.100"  // "192.168.1.122"  //IP Address ของ Host
#define mqtt_port 1883 //Port ของ MQTT broker (default 1883)
//นิยาม pin ต่างๆ
#define DHTPIN D1
#define LED_G D7
#define LED_R D5
//ตั้งค่า Wifi
const char* ssid = "Prawet";    // "DLINK "; 
const char* password = "abcdefabcd";
//ตั้งค่า MQTT credential ใช้ในกรณีที่ broker ต้องการ credential
const char* MQTT_username = NULL; 
const char* MQTT_password = NULL;
//ประกาศตัวแปรใช้ใน Timer
long now = millis();
long lastMeasure = 0;
//สร้าง Client Object
WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE); //สร้าง DHT Object

void setup_wifi() { //ฟังก์ชั่นเชื่อมต่อ Wifi
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP()); //แสดง IP Address ของ ESP8266
}

void reconnect() { //ฟังก์ชั่นต่อ MQTT broker ในกรณีเชื่อมต่อไม่สำเร็จ
  //ทำซ้ำจนกว่าจะเชื่อมต่อ Wifi สำเร็จ
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client", MQTT_username, MQTT_password)) {
      Serial.println("connected");  
      client.subscribe("esp8266/led/+"); // เครื่องหมาย + ใช้สำหรับการ subscribe wildcard
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state()); //Error code
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(String topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  for (int i = 0; i < length; i++) {
    messageTemp += (char)payload[i];
  }
  Serial.println(messageTemp);
  if(topic=="esp8266/led/green"){
      Serial.print("Changing green LED status to ");
      if(messageTemp == "on"){
        digitalWrite(LED_G, HIGH);
        Serial.print("On");
      }
      else if(messageTemp == "off"){
        digitalWrite(LED_G, LOW);
        Serial.print("Off");
      }
  }
  if(topic=="esp8266/led/red"){
      Serial.print("Changing red LED status to ");
      if(messageTemp == "on"){
        digitalWrite(LED_R, HIGH);
        Serial.print("On");
      }
      else if(messageTemp == "off"){
        digitalWrite(LED_R, LOW);
        Serial.print("Off");
      }
  }
  Serial.println();
}

void setup() {
  pinMode(LED_G, OUTPUT);
  pinMode(LED_R, OUTPUT);
  dht.begin();
  Serial.begin(115200);
  delay(10);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  if(!client.loop()){
    client.connect("ESP8266Client", MQTT_username, MQTT_password);
  }
  now = millis();
  if (now - lastMeasure > 10000) {
    lastMeasure = now;

    float humidity = dht.readHumidity();
    float temperatureC = dht.readTemperature();
    float temperatureF = dht.readTemperature(true);

    client.publish("esp8266/temp/celcius",String(temperatureC).c_str());
    client.publish("esp8266/temp/farenheit",String(temperatureF).c_str());
    client.publish("esp8266/humid",String(humidity).c_str());
    Serial.println("payload sent");
  }
}
