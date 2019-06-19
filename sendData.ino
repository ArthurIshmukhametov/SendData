#include <WiFi.h>
#include <AsyncMqttClient.h>
#include <EtherCard.h>
#include <HTTPClient.h>
#include "timer-api.h"

#include <map>

using namespace std;
#define STATIC 0

WiFiServer server(80);
byte Ethernet::buffer[700];

HTTPClient http;

AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;

#define WIFI_SSID "******"
#define WIFI_PASSWORD "*********** "

static byte mymac[] = { 0xCC, 0x50, 0xE3, 0x8A, 0x9B, 0xDC};
static byte myip[] = { 192, 168, 1, 8 };    //IPAddress(192, 168, 1, 8);
static byte gwip[] = { 192, 168, 1, 1 };    //IPAddress(192, 168, 1, 1);
#define MQTT_HOST IPAddress(192, 168, 1, 2)
#define MQTT_PORT 1883

unsigned long previousMillis = 0;
const long interval = 5000;
String Test_string = "";

std::map <String, String> dict;

bool networkStatus[3];    //  0 - WiFi
                          //  1 - Ethernet
                          //  2 - MQTT
bool ethInterrupt = false;
void setup() {
  Serial.begin(115200);
  
  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));

  timer_init_ISR_1Hz(TIMER_DEFAULT);
  
  WiFi.onEvent(WiFiEvent);
  
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  connectToWifi();
  connectEthernet(5000);
  server.begin();
}

void loop() {
  
}

void timer_handle_interrupts(int timer) {
    Serial.println("goodbye from timer");
    networkStatus[1] = false;
    ethInterrupt = true;
}

void getNetworkStatus(WiFiEvent_t event, int timer){
  
  bool networkStatus[3] = {false,   // WiFi
                           false,   // Ethernet
                           false};  // MQTT
  //  *To-Do: Differentiate if-esle branching to devide choose. 
  //  *       Allows multiple true-value for network status
  
  if (event = SYSTEM_EVENT_STA_DISCONNECTED)
  {
    if (check ethernet)
    {
      timer_handle_interrupts(timer);
    }
  }
  else{
  networkStatus[0]=true;
    if (check mqtt)
    {
      networkStatus[2]=true;
    }
  }
}
String sendData(std::map <String, String> dict){
  
  getNetworkStatus(WiFiEvent, 5000);
  
  if (networkStatus[0])
  {
    if (networkStatus[2])
    {
      publishMQTT(dict);
    }
    else
    {
      String answerWiFiHTTP = POST_HTTP(dict);
    }
  }
  else
  {
    if (networkStatus[1])
    {
      //String token = auth();
      
      String answerEthernet = sendEthernet(dict);   //sendEthernet(map dict, String token);
    }
    else
    {
      storeData(dict);
    }
  }
}




String sendEthernet(std::map <String, String> dict){
  //      SEND POST REQUEST VIA ETHERNET
  WiFiEvent
  //  const char *  urlbuf;
  //  const char *  hoststr;
  //  const char *  additionalheaderline;
  //  const char *  postval;
  //  void(*)(uint8_t, uint16_t, uint16_t)  callback;
  
  //  httpPost(urlbuf, hoststr, additionalheaderline, postval, callback);
  
  //  urlbuf                - Pointer to c-string URL folder
  //  hoststr               - Pointer to c-string hostname
  //  additionalheaderline  - Pointer to c-string with additional HTTP header info
  //  postval               - Pointer to c-string HTML Post value
  //  callback              - Pointer to callback function to handle response
  const char *  postval = dictionaryToPOST(dict);    
  ether.httpPost("server-api.cara.bi", "/query/run/", "Content-Type: text/plain", postval, callback);
}

void callback(uint8_t p1, uint16_t p2, uint16_t p3){
    
}

const char *  dictionaryToPOST(std::map <String, String> dict){
  //      CONVERTING DICTIONARY TO CONST CHAR* FOR POST REQUEST
  String postval_s = "";
  std::map <String, String> :: iterator it = dict.begin();
  for (int i = 0; it != dict.end(); it++, i++) {
    postval_s = postval_s + "\n" + it->first + ": " + it->second + "\n";
    //postval = strcat(postval, "\n", it->first, ": ",it->second, "\n");
  }
  const char * postval = postval_s.c_str();
  return postval;
}

void storeData(std::map <String, String> dict){
  //      SAVE DATA IF ALL CONNECTIONS LOST
  
}

void publishMQTT(std::map <String, String> dict){
 //        PUBLISH DATA TO MQTT TOPICS
 
 //unsigned long currentMillis = millis();
 // if (currentMillis - previousMillis >= interval) {
 //   previousMillis = currentMillis;
//
 //   //записываю в строку отправки случайное значение
 //   Test_string=(String)random(0, 100);
 //   Serial.println(Test_string);
 
  std::map <String, String> :: iterator it = dict.begin();
  
  for (int i = 0; it != dict.end(); it++, i++) {
    String topic = "esp32/" + it->first;
    uint16_t packetIdPub2 = mqttClient.publish(topic.c_str(), 2, true,  it->second.c_str());
    
    Serial.print("Publishing on topic esp32/test at QoS 2, packetId: ");  
    Serial.println(packetIdPub2);
  }
}

String POST_HTTP(std::map <String, String> dict){
  //        SEND POST REQUEST VIA WIFI
  String responseWiFiHTTP = "";
  http.begin("http://PUT_YOUR_URL.HERE");
  http.addHeader("Content-Type", "text/plain");
  
  const char *  postval = dictionaryToPOST(dict);
  
  int httpResponseCode = http.POST(postval);
  
  if(httpResponseCode>0){
    responseWiFiHTTP = http.getString(); 
    Serial.println(httpResponseCode);
    Serial.println(responseWiFiHTTP);
  }else{
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
  }
  http.end();
  return responseWiFiHTTP;
}

void connectToWifi() {
  //        CONNECT TO WIFI
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void connectToMqtt() {
  //        CONNECT TO MQTT
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void WiFiEvent(WiFiEvent_t event) {
  //        WIFI EVENT HANDLER
  Serial.printf("[WiFi-event] event: %d\n", event);
  switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      connectToMqtt();
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
  //        MQTT DISCONNECT IF WIFI NOT AVAILIBLE
      xTimerStop(mqttReconnectTimer, 0);
      xTimerStart(wifiReconnectTimer, 0);
      break;
  }
}
 
void onMqttConnect(bool sessionPresent) {
  //        MQTT TOPICS SUBSCRIBITION
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
  uint16_t packetIdSub = mqttClient.subscribe("esp32/led", 0);
  Serial.print("Subscribing at QoS 0, packetId: ");
  Serial.println(packetIdSub);
}
 
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  //        DISCONNECTING MQTT
  Serial.println("Disconnected from MQTT.");
  if (WiFi.isConnected()) {
    xTimerStart(mqttReconnectTimer, 0);
  }
}
 
void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  //        MQTT SUBSCRIBE EVENT
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos); // уровень качества отправки сообщений
}
 
void onMqttUnsubscribe(uint16_t packetId) {
  //        MQTT UNSUBSCRIBE EVENT
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}
 
void onMqttPublish(uint16_t packetId) {
  //        MQTT PUBLISH EVENT
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}
 
void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  //        MQTT RECIVE EVENT
  String messageTemp;
  for (int i = 0; i < len; i++) {
    messageTemp += (char)payload[i];

  //if (strcmp(topic, "esp32/led") == 0) {
  //  if (ledState == LOW) {
  //    ledState = HIGH;
  //  } else {
  //    ledState = LOW;
  //  }
  //  digitalWrite(ledPin, ledState);
  //}
 
  Serial.println("Publish received.");

  Serial.print("  message: ");
  Serial.println(messageTemp);
  Serial.print("  topic: ");
  Serial.println(topic);
  Serial.print("  qos: ");
  Serial.println(properties.qos);
  Serial.print("  dup: ");
  Serial.println(properties.dup);
  Serial.print("  retain: ");
  Serial.println(properties.retain);
  Serial.print("  len: ");
  Serial.println(len);
  Serial.print("  index: ");
  Serial.println(index);
  Serial.print("  total: ");
  Serial.println(total);
  }
}

void connectEthernet(int Timer){
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= Timer) {
    previousMillis = currentMillis;
    
  Serial.println("Trying to get an IP...");  
  Serial.print("MAC: ");
  for (byte i = 0; i < 6; ++i) {
    Serial.print(mymac[i], HEX);
  }
  Serial.println();
  if (ether.begin(sizeof Ethernet::buffer, mymac,5) == 0)
  {
    Serial.println( "Failed to access Ethernet controller");
  }
  else
  {
    Serial.println("Ethernet controller access: OK");
  };

#if STATIC
  Serial.println( "Getting static IP.");
  if (!ether.staticSetup(myip, gwip)) {
    Serial.println( "could not get a static IP");
  }
#else

  Serial.println("Setting up DHCP");
  if (!ether.dhcpSetup()) {
    Serial.println( "DHCP failed");
  }
#endif

  ether.printIp("My IP: ", ether.myip);
  ether.printIp("Netmask: ", ether.netmask);
  ether.printIp("GW IP: ", ether.gwip);
  ether.printIp("DNS IP: ", ether.dnsip);
 }
}
