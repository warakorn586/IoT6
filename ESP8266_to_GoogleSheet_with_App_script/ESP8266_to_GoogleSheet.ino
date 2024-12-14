// -------------------------------------- Including the libraries.
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <DHT.h>

// --------------------------------------- Defining LED and SW PINs on the ESP8266 Board.
#define On_Board_LED_PIN  D2
#define SW  D5

// --------------------------------------- Defining DHT22 PINs on the ESP8266 Board.
#define DHTPIN  D4
#define DHTTYPE  DHT22
DHT dht(DHTPIN, DHTTYPE);

// --------------------------------------- SSID and PASSWORD of your WiFi network.
const char* ssid = "____?____";    // Your wifi name
const char* password = "____?____";  // Your wifi password

// Google Deployment ID.
String Deployment_ID = "____?____";

// Host & HttpsPort
const char* host = "script.google.com";
const int httpsPort = 443;

WiFiClientSecure client;

String Switch_State = "On";
String Status_Read_Sensor = "Ok";

// ---------------------------------------- VOID SETUP()
// put your setup code here, to run once:
void setup() {
  pinMode(On_Board_LED_PIN, OUTPUT);
  pinMode(SW, INPUT_PULLUP);
  dht.begin();

  Serial.begin(115200);
  delay(1000);

  // ---------------------------------------- Set Wifi to STA mode
  Serial.println();
  Serial.println("-------------");
  Serial.println("WIFI mode : STA");
  WiFi.mode(WIFI_STA);
  Serial.println("-------------");

  // ---------------------------------------- Connect to Wi-Fi (STA).
  Serial.println();
  Serial.println("------------");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

int connecting_process_timed_out = 40; // 40 seconds.
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    digitalWrite(On_Board_LED_PIN, HIGH);
    delay(250);
    digitalWrite(On_Board_LED_PIN, LOW);
    delay(250);
    if (connecting_process_timed_out > 0) connecting_process_timed_out--;
    if (connecting_process_timed_out == 0) {
      delay(1000);
      ESP.restart();
    }
  }

  digitalWrite(On_Board_LED_PIN, LOW);
  
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("------------");
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  client.setInsecure();
  delay(2000);
}

// ---------------------------------------- VOID LOOP()
// put your main code here, to run repeatedly:
void loop() {
  float Temp = dht.readTemperature();
  float Humd = dht.readHumidity();

  if(isnan(Temp) || isnan(Humd)){
    Serial.println("Failed!");
  } else {
    Serial.print("Temperature : ");
    Serial.print(Temp);
    Serial.println("*C");
    Serial.print("Humidity : ");
    Serial.print(Humd);
    Serial.println("%");
    Serial.println("-------------");  
  }

  if(digitalRead(SW) == HIGH){
    Switch_State = "On";
  } else {
    Switch_State = "Off";
  }

  // -------------------------------- Conditions that are executed when WiFi is connected.
  // This condition is the condition for sending or writing data to Google Sheets.
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }

  digitalWrite(On_Board_LED_PIN, HIGH);

  // Create a URL for sending or writing data to Google Sheets.
  String Send_Data_URL = "/macros/s/" + Deployment_ID + "/exec?sts=write";
  Send_Data_URL += "&srs=" + Status_Read_Sensor;
  Send_Data_URL += "&temp=" + String(Temp);
  Send_Data_URL += "&humd=" + String(Humd);
  Send_Data_URL += "&swtc=" + Switch_State;

  Serial.println();
  Serial.println("-------------");
  Serial.println("Send data to Google Spreadsheet...");
  Serial.print("URL : https://");
  Serial.print(host);
  Serial.println(Send_Data_URL);

  client.print(String("GET ") + Send_Data_URL + " HTTP/1.1\r\n" 
                              + "Host: " + host + "\r\n" 
                              + "User-Agent: BuildFailureDetectorESP8266\r\n" 
                              + "Connection: close\r\n\r\n");
  
  Serial.println("closing connection");  
  Serial.println("-------------");
  Serial.println();
  delay(10000);
  digitalWrite(On_Board_LED_PIN, LOW);
}
