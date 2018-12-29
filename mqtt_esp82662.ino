#include <ArduinoJson.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <IRsend.h>

//define pins for IR sender and reciever
const uint16_t pinIRLED = D2;  
const uint16_t pinIRRecv = D6;
IRsend irsend(pinIRLED); 
IRrecv irrecv(pinIRRecv);
decode_results results;

//wifi settings
const char* ssid = "ssid";
const char* password = "pass";

//mqtt settings
const char* mqttServer = "192.168.1.1";
int mqttPort = 1883;
const char* mqttUser = "user";
const char* mqttPassword = "pass";
const char* mqttSubscribTopic = "irsend/send";
const char* mqttPublishTopic = "irsend/recv";

//clients
WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {

  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }

  JsonObject root = doc.as<JsonObject>();
  Serial.print("Recieved from MQTT: ");
  serializeJson(root, Serial);
  Serial.println();

  String manu = root["manufacturer"];
  int code =   root["code"];
  int bits = root["bits"];

  if(manu = "SAMSUNG") {
    irsend.sendSAMSUNG(code,bits);
  }
  else if(manu = "SONY") {
    irsend.sendSony(code,bits,2);
  }
  else if(manu = "LG") {
    irsend.sendLG(code,bits);
  }
  else{
    irsend.sendNEC(code,bits);
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    if (client.connect(String(ESP.getChipId()).c_str(), mqttUser, mqttPassword)) {
      Serial.println("Connected!");
      client.subscribe(mqttSubscribTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//arduino loop and setup methods

void setup() {
  //Setup IR
  irsend.begin();
  irrecv.enableIRIn();  // Start the receiver
  //Setup Serial
  Serial.begin(115200);
  //Setup WiFi
  setup_wifi();
  //Setup MQTT
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
}

void loop() {
  
  //if we have IR data   
  if (irrecv.decode(&results)) {
    
    StaticJsonDocument<200> doc;
    JsonObject root = doc.to<JsonObject>();
    //build JSON
    root["manufacturer"] = typeToString(results.decode_type);
    root["code"] = results.value;
    root["bits"] = results.bits;

    //push JSON to serial and MQTT
    String output;
    serializeJson(root, output);
    client.publish(mqttPublishTopic, output.c_str());
    Serial.println("Recieved from IR" + output);
    irrecv.resume();  // Receive the next value
  }

  //If MQTT drops, reconnect
  if (!client.connected()) {
    reconnect();
  }

  //MQTT client loop
  client.loop();
}
