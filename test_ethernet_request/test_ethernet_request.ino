#include <EtherCard.h>

const char website[] PROGMEM = "jsonplaceholder.typicode.com";
static byte session;
Stash stash;
byte Ethernet::buffer[1000];
static byte mymac[] = { 0xCC, 0x50, 0xE3, 0x8A, 0x9B, 0xDC};
static byte myip[] = { 192, 168, 1, 8 };    //IPAddress(192, 168, 1, 8);
static byte gwip[] = { 192, 168, 1, 1 };    //IPAddress(192, 168, 1, 1);

static char * headers="\"Content-type\": \"application/json; charset=UTF-8\"";
const char *  postval="{title: \"foo\",body: \"bar\", userId: 1}";

void setup() {
  Serial.begin(115200);
  connectEthernet();
  sendEthernet();
 }
int i = 0;

void loop() {
  ether.packetLoop(ether.packetReceive());
 
  const char* reply = ether.tcpReply(session);
  if (reply != 0) {
    Serial.println("Got a response!");
    Serial.println(reply);
  }
}

String sendEthernet(){
  Serial.println("[HTTP] Post...");
  byte sd = stash.create();
  stash.println(headers);
  stash.println(postval);
  stash.save();
  int stash_size = stash.size();
  Stash::prepare(PSTR("POST http://$F/posts HTTP/1.0" "\r\n"
    "Host: $F" "\r\n"
    "Content-Length: $D" "\r\n"
    "\r\n"
    "$H"),
  website, website, stash_size, sd);
  session = ether.tcpSend();
  //ether.httpPost("https://jsonplaceholder.typicode.com", "/posts", headers, postval, callback);
}


void connectEthernet(){
    
 if (ether.begin(sizeof Ethernet::buffer, mymac, SS) == 5)
    Serial.println(F("Failed to access Ethernet controller"));
  if (!ether.dhcpSetup())
    Serial.println(F("DHCP failed"));

  ether.printIp("IP:  ", ether.myip);
  ether.printIp("GW:  ", ether.gwip);
  ether.printIp("DNS: ", ether.dnsip);

  if (!ether.dnsLookup(website))
    Serial.println(F("DNS failed"));

  ether.printIp("SRV: ", ether.hisip);
}
