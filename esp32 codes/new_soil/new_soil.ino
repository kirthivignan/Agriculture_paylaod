#include <ESP32Servo.h>


////////////////////////////////////////////////////////
#include "AsyncUDP.h"
AsyncUDP udp;

#include <WiFi.h>
#include <WiFiClient.h>
#include "ThingSpeak.h"
#include <ESPmDNS.h>
#include <Update.h>



int sensorPin = 34; 



WiFiClient  client;
const char* host = "esp32_1";
const char* ssid = "The_hive";
const char* password = "12345678901";


String input;
// write a funciton here

/*
 * setup function
 */
int reads =0;
 unsigned long myChannelNumber = 1;
const char * myWriteAPIKey = "EP76FYILI4CTHQJR";


unsigned long lastTime = 0;
unsigned long timerDelay = 200;

void setup(void) {

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
  ThingSpeak.begin(client);  // Initialize ThingSpeak
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

           Serial.write(packet.data(), packet.length());
             input = String( (char*) packet.data());
            Serial.println();
            //Serial.println(input);
            
        });
    }
}

int sensorValue;
 




void loop(void) 
{   
   if(input=="sr"){
    reads=1;
    }
    if(input=="ss"||input=="us"){
    reads=0;  
      }

  if(reads==1){
 int sens = analogRead(sensorPin); 
 Serial.println("Analog Value : ");
 float fina= 100-((float(sens)/4095)*100);
 Serial.println(fina);
    ThingSpeak.setField(1,fina );
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(x == 200){
    Serial.println("Channel update successful.");
  }
 lastTime = millis();
}
}
