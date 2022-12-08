#include <ESP32Servo.h>
#include "DFRobot_ESP_PH.h"
#include <EEPROM.h>
#include <OneWire.h>


////////////////////////////////////////////////////////
#include "AsyncUDP.h"
AsyncUDP udp;
#include <HTTPClient.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <WiFiAP.h>
#include "ThingSpeak.h"


const char* host = "esp32_1";
const char* ssid = "The_hive";
const char* password = "12345678901";
String input;
// write a funciton here

#define CHANNEL_ID 1696047
#define CHANNEL_API_KEY "Z2XAJZWBVPKQ9SRT"


String serverName = "https://api.thingspeak.com/update?api_key=Z2XAJZWBVPKQ9SRT";

DFRobot_ESP_PH ph;
#define ESPADC 4096.0   //the esp Analog Digital Convertion value
#define ESPVOLTAGE 3300 //the esp voltage supply value
#define PH_PIN 34  //the esp gpio data pin number
#define TdsSensorPin 32
#define VREF 3.3      // analog reference voltage(Volt) of the ADC
#define SCOUNT  30           // sum of sample point
int analogBuffer[SCOUNT];    // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0,copyIndex = 0;
float averageVoltage = 0,tdsValue = 0;

//******
float phvoltage, phValue, temperature;
//******

int DS18S20_Pin = 26; //DS18S20 Signal pin on digital 2
OneWire ds(DS18S20_Pin);  // on digital pin 2





/*
 * setup function
 */
void setup(void) {

  Serial.begin(115200);
  delay(1000);

  pinMode(TdsSensorPin,INPUT);
    ph.begin();
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
int readw =0;

void loop(void) 
{   
   if(input=="wr"){
    readw=1;
    }
    if(input=="ws"||input=="us"){
      readw=0;
      }
if(readw==1){
  static unsigned long analogSampleTimepoint = millis();
   if(millis()-analogSampleTimepoint > 40U)     //every 40 milliseconds,read the analog value from the ADC
   {
     analogSampleTimepoint = millis();
     analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);    //read the analog value and store into the buffer
     analogBufferIndex++;
     if(analogBufferIndex == SCOUNT) 
         analogBufferIndex = 0;
   }   
   static unsigned long printTimepoint = millis();
   if(millis()-printTimepoint > 800U)
   {
      float temperature = getTemp();
      delay(100); //just here to slow down the output so it is easier to read
      printTimepoint = millis();
      
      //*****
      phvoltage = analogRead(PH_PIN) / ESPADC * ESPVOLTAGE;
      phValue = ph.readPH(phvoltage, temperature);
      //*****
      
      for(copyIndex=0;copyIndex<SCOUNT;copyIndex++)
        analogBufferTemp[copyIndex]= analogBuffer[copyIndex];
      averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 4096.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
     
      float compensationCoefficient=1.0+0.02*(temperature-25.0);    //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
      
      float compensationVolatge=averageVoltage/compensationCoefficient;  //temperature compensation
      
      
      tdsValue=(133.42*compensationVolatge*compensationVolatge*compensationVolatge - 255.86*compensationVolatge*compensationVolatge + 857.39*compensationVolatge)*0.5; //convert voltage value to tds value
      
      //Serial.print("voltage:");
      //Serial.print(averageVoltage,2);
      //Serial.print("V   ");
//      Serial.print("TDS Value:");
      Serial.print(temperature);
      Serial.print(",");      
      Serial.print(tdsValue);
      Serial.print(","); 
      Serial.print(phValue, 4);
      Serial.print(",");
      Serial.println("");

      if(WiFi.status()== WL_CONNECTED){ // Check to make sure wifi is still connected
        sendData(temperature, tdsValue, phValue); // Call the sendData function defined below
      }
      else {
        Serial.println("WiFi Disconnected");
      }
   }

   //*****
   ph.calibration(phvoltage, temperature);
   //*****
   
}

}

int getMedianNum(int bArray[], int iFilterLen) 
{
      int bTab[iFilterLen];
      for (byte i = 0; i<iFilterLen; i++)
      bTab[i] = bArray[i];
      int i, j, bTemp;
      for (j = 0; j < iFilterLen - 1; j++) 
      {
      for (i = 0; i < iFilterLen - j - 1; i++) 
          {
        if (bTab[i] > bTab[i + 1]) 
            {
        bTemp = bTab[i];
            bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
         }
      }
      }
      if ((iFilterLen & 1) > 0)
    bTemp = bTab[(iFilterLen - 1) / 2];
      else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
      return bTemp;
}

void sendData(double temper, double pres, double alt){
  HTTPClient http; 


  String url = serverName + "&field1=" + temper + "&field2=" + pres + "&field3=" + alt; // Define our entire url
      
  http.begin(url.c_str()); // Initialize our HTTP request
      
  int httpResponseCode = http.GET(); // Send HTTP request
      
  if (httpResponseCode > 0){ // Check for good HTTP status code
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
  }else{
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

float getTemp(){
  //returns the temperature from one DS18S20 in DEG Celsius

  byte data[12];
  byte addr[8];

  if ( !ds.search(addr)) {
      //no more sensors on chain, reset search
      ds.reset_search();
      return -1000;
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return -1000;
  }https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js

  if ( addr[0] != 0x10 && addr[0] != 0x28) {
      Serial.print("Device is not recognized");
      return -1000;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44,1); // start conversion, with parasite power on at the end

  byte present = ds.reset();
  ds.select(addr);
  ds.write(0xBE); // Read Scratchpad


  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = ds.read();
  }

  ds.reset_search();

  byte MSB = data[1];
  byte LSB = data[0];

  float tempRead = ((MSB << 8) | LSB); //using two's compliment
float   TemperatureSum = tempRead / 16;

  return TemperatureSum;

}
