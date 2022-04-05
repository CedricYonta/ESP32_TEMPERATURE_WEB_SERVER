/*
  Titre      : Titre du programme
  Auteur     : Cedric Yonta
  Date       : 05/04/2022
  Description: envoi des données de temperatures sur un serveur web
  Version    : 0.0.1

*/



#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <Adafruit_I2CDevice.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_BMP280.h>
#include <SPI.h>
#include "RTClib.h"


// Identifiants du reseau utilisé
/*const char* ssid = "237HomeTyga";
const char* password = "Overdose2021";*/

const char* ssid = "IDO-OBJECTS";
const char* password = "42Bidules!";


RTC_DS3231 rtc;
/*#define OLED_RESET -1
Adafruit_SSD1306 display(OLED_RESET);*/


#define BMP_SCK  (13)
#define BMP_MISO (12)
#define BMP_MOSI (11)
#define BMP_CS   (10)

Adafruit_BMP280 bmp; // I2C
//Adafruit_BMP280 bmp(BMP_CS); // hardware SPI
//Adafruit_BMP280 bmp(BMP_CS, BMP_MOSI, BMP_MISO,  BMP_SCK);

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// convertir la temperature en chaine de caractère  pour l'envoyer
String gettemperature()
{
  float temperature = bmp.readTemperature();
   String temp = String(temperature) + '°C';
  Serial.println(temp);
 return temp ;
 
}

// obtenir la date et l'heure en chaine de caractère
String getDate()
{
  char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
  DateTime now = rtc.now();
  String dateH ="";
   
    dateH += String(now.year(), DEC); 
    dateH += '/';
    dateH += String(now.month(), DEC);
    dateH += '/';
    dateH += String(now.day(), DEC);
    dateH += (" (");
    dateH += String(daysOfTheWeek[now.dayOfTheWeek()]);
    dateH += (") ");
    dateH += String(now.hour(), DEC);
    dateH += (':');
    dateH += String(now.minute(), DEC);
    dateH += (':');
    dateH += String(now.second(), DEC);
    Serial.println(dateH);
  
 return dateH ;
 
}


//code de la page avec les requetes
const char index_html[] PROGMEM = R"rawliteral(

<!DOCTYPE html>
<html>
    <head>
        <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/css/bootstrap.min.css" rel="stylesheet" integrity="sha384-1BmE4kWBq78iYhFldvKuhfTAU6auU8tT94WrHftjDbrCEXSU1oBoqyl2QvZ6jIW3" crossorigin="anonymous">
        <script src="https://kit.fontawesome.com/7bd22ada10.js" crossorigin="anonymous"></script>
        <style> 
                    body { 
                    text-align: center;
                    font-family: \"Trebuchet MS\", Arial;}

                    table { border-collapse: collapse; 
                    width:35%;
                    margin-left:auto; 
                    margin-right:auto; }

                    .th { padding: 12px;
                    background-color: rgb(22, 146, 146);
                    color: rgb(22, 146, 146); }

                    .tr { border: 1px solid #ddd;
                    padding: 12px; }

                    tr:hover { background-color: #bcbcbc; }

                    .td { border: none; padding: 12px;color: rgb(22, 146, 146); }

                    .color { 
                    color: rgb(10, 97, 228); }

                    .container
                {
                  
                    margin: 20px auto;
                    width: 20%;
                    padding: 15px;
                      
                }


                .button { background-color: #242b6b; border: none; color: white; padding: 16px 40px;
        </style>
    </head>
    <body>

     <h1>ESP32 WEB SERVER</h1> 
        <div class="container">
            <table class="table table-borderless ">
                <thead class=" bg-primary">
                  <tr class=" text-white tr">
                   
                    <th scope="col" class=" text-white">Data</th>
                    <th scope="col" class=" text-white">LastUpdate</th>
                    <th scope="col" class=" text-white">Value</th>
                  </tr>
                </thead>
                <tbody>
                  <tr class="tr">
                   
                    <td ><i class="fa-solid fa-temperature-half fa-3x color2 icone "></i></td>
                    <td id="date" class="border: " >%DATEHEURE%</td>
                    <td id="temperature" >%TEMPERATURE%<sup>&deg;C</sup></td>
                  </tr>
            
                </tbody>
              </table>
            
        </div>
        

        
    </body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("date").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/date", true);
  xhttp.send();
}, 10000 ) ;
</script>
</html>)rawliteral";

// Replaces placeholder with BMP values
String processor(const String& var){
  //Serial.println(var);
  if(var == "TEMPERATURE"){
    return gettemperature();
  }
  else if(var == "DATEHEURE"){
    return getDate();
  }
  return String();
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(9600);
  Serial.println(F("BMP280 Forced Mode Test."));

  //if (!bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID)) {
  if (!bmp.begin(0x76)) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
    while (1) delay(10);
  }

 /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_FORCED,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
   
    #ifndef ESP8266
  while (!Serial); // wait for serial port to connect. Needed for native USB
#endif

//RTC start
if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");

    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  }

  
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain",gettemperature().c_str());
  });
  server.on("/date", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", getDate().c_str());
  });

  // Start server
  server.begin();
}
 
void loop(){

  
  
}

