/*
* Ultrasonic Sensor HC-SR04 and Arduino Tutorial
*
* Crated by Dejan Nedelkovski,
* www.HowToMechatronics.com
*
*/
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>

// -- Start OTA
const char* ssid = "yourwifissid";
const char* password = "yourwifipassword";

// -- Start Webserver
ESP8266WebServer webServer(80);
String headerHTML = ""
  "<!DOCTYPE html><html> <meta http-equiv='refresh' content='5'><head><title>Electric Car Space</title></head><body>"
  "<h1>Electric Car Space Status</h1><h2 font-color='blue'>";
String bodyHTML = " . . . status unknown ... ";  
String footerHTML = "</h2></body></html>";
String responseHTML = headerHTML + bodyHTML + footerHTML;

// -- Start Ultra Sonic Variables
// defines pins numbers
const int trigPin = 5;
const int echoPin = 4;
// defines variables
long duration;
int distance;
// dataset 
int rawIdx = 0;
unsigned int rawAvg = 0;
int rawSampleCnt = 256;

void setup() {
// -- Setup OTA - Over the air updates
  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  // Port defaults to 8266
  ArduinoOTA.setPort(8266);
  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("SonicTheDistance");
  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());


  // replay to all requests with same HTML
  webServer.onNotFound([]() {
    webServer.send(200, "text/html", responseHTML);
  });
  webServer.begin();

// -- Start Ultra Sonic   
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input

// -- Serial port initialization  
  Serial.begin(115200); // Starts the serial communication
}

void loop() {
  // -- Capture OTA events
  ArduinoOTA.handle();
  
  // -- Handle web server requests
  webServer.handleClient();

  // -- Start Ultrasonic
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance= duration*0.034/2;
  
  if(rawIdx<rawSampleCnt) {
    rawIdx++;
    rawAvg += distance;
  } else {
    rawIdx = 0;
    rawAvg = rawAvg / rawSampleCnt; 
    // Prints the distance on the Serial Monitor
    Serial.print("Distance: "); Serial.print(distance); Serial.print("  ");
    if(rawAvg != 0 && (rawAvg < 45 || rawAvg > 66)) { // these values came after a lot of testing. When something is really close to the sensor a high value above 65 is returned.
      bodyHTML = "OCCUPIED       ";
    } else {
      bodyHTML = "AVAILABLE      ";      
    }
    responseHTML = headerHTML + bodyHTML + footerHTML;
    Serial.print(bodyHTML);    
    Serial.print(rawAvg);
    Serial.println();
    delay(3);  
    rawAvg = 0;
  }
}



