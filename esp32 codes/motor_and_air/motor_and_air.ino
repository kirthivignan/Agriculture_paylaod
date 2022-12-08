#include <ESP32Servo.h>


////////////////////////////////////////////////////////
#include "AsyncUDP.h"
AsyncUDP udp;
#include "ThingSpeak.h"
#include "DHT.h"
#include <WiFi.h>
#include <WiFiClient.h>

#include <ESPmDNS.h>
#include <Update.h>

int motor1pin1 = 12;//soilpins
int motor1pin2 = 14;
int motor2pin1 = 27;//waterpins
int motor2pin2 =26;

//sisr sensor
#define DHTPIN 4 
#define DHTTYPE DHT11 
WiFiClient  client;
const char* host = "esp32_1";
const char* ssid = "The_hive";
const char* password = "12345678901";
String input;
// write a funciton here

/*
 * setup function
 */


unsigned long myChannelNumber = 1;
const char * myWriteAPIKey = "CHUEE2RPVK2XMK9V";

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 200;

// Variable to hold temperature readings
float temperatureC;
//uncomment if you want to get temperature in Fahrenheit
//float temperatureF;


DHT dht(DHTPIN, DHTTYPE);



void setup(void) {

 pinMode(motor1pin1, OUTPUT);
  pinMode(motor1pin2, OUTPUT);
   pinMode(motor2pin1, OUTPUT);
  pinMode(motor2pin2, OUTPUT);
  
 digitalWrite(motor1pin1, LOW);
  digitalWrite(motor1pin2, LOW);
 digitalWrite(motor2pin1, LOW);
  digitalWrite(motor2pin2, LOW);
  
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_STA);   
    Serial.println(F("DHTxx test!"));

  dht.begin();
  ThingSpeak.begin(client);  // Initialize ThingSpeak

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

            Serial.write(packet.data(), packet.length());
             input = String( (char*) packet.data());
            Serial.println();
            //Serial.println(input);
            
        });
    }
}

void loop(void) 
{   
  
  if ((millis() - lastTime) > timerDelay){
   float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

   if (isnan(h) || isnan(t) ) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
      float hic = dht.computeHeatIndex(t, h, false);
    
    // Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
  
  ThingSpeak.setField(1, h);
  ThingSpeak.setField(2, t);
  ThingSpeak.setField(3, hic);

Serial.println(h);
Serial.println(t);
Serial.println(hic);

  
  // write to the ThingSpeak channel
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(x == 200){
    Serial.println("Channel update successful.");
  }
  else{
    //Serial.println("Problem updating channel. HTTP error code " + String(x));
  }

    if(x == 200){
      Serial.println("Channel update successful.");
    }
    else{
     // Serial.println("Problem updating channel. HTTP error code " + String(x));
    }
    lastTime = millis();
  }
  
 
  
  if(input=="sd")//soil sensor down
    { 
      digitalWrite(motor1pin1, HIGH);
      digitalWrite(motor1pin2, LOW);
      
      }

      
     if(input=="su")//soil sensor up
      { 
      digitalWrite(motor1pin1, LOW);
      digitalWrite(motor1pin2, HIGH);
      
        
        }
    if(input=="wd") {
         digitalWrite(motor2pin1, HIGH);
      digitalWrite(motor2pin2, LOW);
      
   }
   if(input=="wu")//soil sensor up
      { 
      digitalWrite(motor2pin1, LOW);
      digitalWrite(motor2pin2, HIGH);
      
        
        }
   if (input=="sm") {
    
    digitalWrite(motor1pin1, LOW);
      digitalWrite(motor1pin2, LOW);}

       if (input=="wm") {
    
    digitalWrite(motor2pin1, LOW);
      digitalWrite(motor2pin2, LOW);}
           
   else if (input=="us") {
    
    digitalWrite(motor1pin1, LOW);
      digitalWrite(motor1pin2, LOW);
    digitalWrite(motor2pin1, LOW);
      digitalWrite(motor2pin2, LOW);
    }     

}
