#include <ESP32Servo.h>


////////////////////////////////////////////////////////
#include "AsyncUDP.h"
AsyncUDP udp;

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>

const char* host = "esp32_1";
const char* ssid = "dlink-A3F8";
const char* password = "mubfc34497";
String input;
// write a funciton here

/*
 * setup function
 */
void setup(void) {
//  pinMode(LED_BUILTIN, OUTPUT);
//  digitalWrite(LED_BUILTIN, LOW);
//  
  Serial.begin(115200);
  delay(1000);


  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");

  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  /*use mdns for host name resolution*/
  if (!MDNS.begin(host)) { //http://esp32.local
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
 

  if(udp.listen(9999)) {
        Serial.print("UDP Listening on IP: ");
        Serial.println(WiFi.localIP());
        udp.onPacket([](AsyncUDPPacket packet) {

           // Serial.write(packet.data(), packet.length());
             input = String( (char*) packet.data());
            Serial.println();
            //Serial.println(input);
            
        });
    }
}

void loop(void) 
{    Serial.print(input);


}
