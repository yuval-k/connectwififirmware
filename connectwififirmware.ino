#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)

#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>  

#include <WiFiUdp.h>
#include <OSCMessage.h>                // for sending OSC messages

int led = 16;

#define DATA_SIZE 4
#define PACKET_SIZE 5
#define port_udp 3134
//#define broadcast_udp "255.255.255.255"

WiFiUDP Udp;                           // A UDP instance to let us send and receive packets over UDP
const IPAddress broadcast_udp(255,255,255,255);   // remote IP of the target device
const unsigned int destPort = 3134;    // remote port of the target device where the NodeMCU sends OSC to


byte last_packet[PACKET_SIZE] = {0};

unsigned long send_deadline = 0;
constexpr unsigned long SEND_PERIOD = 5000;

// the setup routine runs once when you press reset:
void setup() {                
  Serial.begin(9600);
  Serial.setTimeout(100);
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);     
  WiFiManager wifiManager;
  //first parameter is name of access point, second is the password
  wifiManager.autoConnect("Connect_FW_WiFi", "Connect_FW_WiFi");

  send_deadline = millis() + SEND_PERIOD;
}


// the loop routine runs over and over again forever:
void loop() {
  static bool pinstate = false;
  byte buffer_datagram[PACKET_SIZE];
  if (Serial.available())
  {
    pinstate = !pinstate;
        digitalWrite(led, pinstate?HIGH:LOW);
    Serial.readBytes(buffer_datagram, PACKET_SIZE);
      if (checkCRC(buffer_datagram))
      {
        memcpy(last_packet, buffer_datagram, PACKET_SIZE);
        send_packet();
        Serial.println("Datagram checksum OK");
      }
    else
      Serial.println("Datagram checksum ERROR");
    
    Serial.flush();
  } 
  if (send_deadline < millis()) {
      Serial.println("Timed out, sending udp packet again cause UDP is UDP..");
      send_packet();
      send_deadline = millis() + SEND_PERIOD;

  }
}

bool checkCRC(const byte buffer_datagram[])
{
  byte checksum=0;
  for (int i=0; i < DATA_SIZE; i++)
  {
    checksum = checksum + buffer_datagram[i];
    Serial.println(buffer_datagram[i]);
  }

  if (checksum == buffer_datagram[PACKET_SIZE-1])
    return true;
  else
    return false;
}

void send_packet() {
  OSCMessage msgOut("/pole_touch");
  uint8_t * blob = last_packet;
  msgOut.add(blob, PACKET_SIZE);

  Udp.beginPacket(broadcast_udp,port_udp);
  msgOut.send(Udp);
  Udp.endPacket();
  msgOut.empty();
}