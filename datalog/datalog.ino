#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <SPI.h>
#include <ArduinoJson.h>

const char* WIFI_SSID = "wifiName";
const char* WIFI_PASS = "wifiPassword";

const char* URL = "http://maker.ifttt.com/trigger/EVENT_NAME_HERE/with/key/YOUR_KEY_HERE";

const int CS = D8; 


StaticJsonDocument<300> doc;

const char msg_buffer[300] = "";

void setup() {

  Serial.begin(115200);
  // Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();
  SPI.begin();

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println(" OK!");
  
  pinMode(CS, OUTPUT);
}

void loop() {

  for(short i = 0; i < 8; i++){
    Serial.print( read(i) );
    Serial.print(","); 
  }
    Serial.println();

    // send request with 3 vars
    // you could also do multiple, by shifting by ten, IE: 
    // (read(0) << 10) | read(1); 
    // or putting in an array. 
    // but you would have to process it on the google sheets side.
    // limitations ahoy; 

    // Maker/IFTTT expect something literally called "value1"
    doc["value1"] = read(0);
    doc["value2"] = read(1);
    doc["value3"] = read(2);


    //send vars, first convert to string, then POST out. 
    serializeJson(doc, (char*) msg_buffer, 300); 
    
    Serial.println(msg_buffer);
    
    HTTPClient http;

    http.begin(URL);
    http.addHeader("Content-Type", "application/json"); //tell server it is json
    int r = http.POST(msg_buffer);
    
    if (r < 0){

      Serial.println(http.errorToString());

    }else{

      http.writeToStream(&Serial); //write response back to serial, 

    }

    http.end();
    Serial.printf("\n[%d] status code\n\n", r);

    delay(60000);
}

int read(short chan){

  if ((chan > 7) || (chan < 0))
    return -1; 
  
  //from page 22 of the MCP3008 datasheet.
  short command = 0b11000000 | (chan << 3);

  //SPI.beginTransaction( SPISettings(1000000, MSBFIRST, SPI_MODE0) );
  digitalWrite(CS, LOW);
  
  SPI.transfer(command);
  int ret = SPI.transfer16(0);

  digitalWrite(CS, HIGH);
  //SPI.endTransaction();

  return ret & 0x3FF;
}
