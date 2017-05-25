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


// the setup routine runs once when you press reset:
void setup() {                
  Serial.begin(9600);
  Serial.setTimeout(100);
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);     
  WiFiManager wifiManager;
  //first parameter is name of access point, second is the password
  wifiManager.autoConnect("Connect_FW_WiFi", "Connect_FW_WiFi");
}

// the loop routine runs over and over again forever:
void loop() {
  byte buffer_datagram[PACKET_SIZE];
  if (Serial.available())
  {
    Serial.readBytes(buffer_datagram, PACKET_SIZE);
      if (checkCRC(buffer_datagram))
      {
        OSCMessage msgOut("/pole_touch");
        uint8_t * blob = buffer_datagram;
        msgOut.add(blob, PACKET_SIZE);
        
        Udp.beginPacket(broadcast_udp,port_udp);
        msgOut.send(Udp);
        Udp.endPacket();
        msgOut.empty();
        Serial.println("Datagram checksum OK");
      }
    else
      Serial.println("Datagram checksum ERROR");
    
    Serial.flush();
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

