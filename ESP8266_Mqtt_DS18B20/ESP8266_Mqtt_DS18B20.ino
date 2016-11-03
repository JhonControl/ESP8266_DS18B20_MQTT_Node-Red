/*
 Basic ESP8266 MQTT example

 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.

 It connects to an MQTT server then:
  - publishes "hello world" to the topic "  " every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off

 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.

 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"

    More information about projects PDAControl
    Mas informacion sobre proyectos PDAControl
    Blog PDAControl English   http://pdacontrolenglish.blogspot.com.co/   
    Blog PDAControl Espanol   http://pdacontrol.blogspot.com.co/
    Channel  Youtube          https://www.youtube.com/c/JhonValenciaPDAcontrol/videos 

*/


#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <OneWire.h>
OneWire  ds(2); 


// Update these with values suitable for your network.

const char* ssid = "#############";
const char* password = "#############";
const char* mqtt_server = "#############";     /// MQTT  Broker

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup() { 
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  
}

void temperature(){

  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius, fahrenheit;
  
  if ( !ds.search(addr)) {
  /// Serial.println("No more addresses.");
   /// Serial.println();
    ds.reset_search();
    delay(250);
    return;
  }

  
  for( i = 0; i < 8; i++) {  
    addr[i];
  }

  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return;
  }
//  Serial.println();
 
  // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:

      type_s = 1;
      break;
    case 0x28:

      type_s = 0;
      break;
    case 0x22:
    //  Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    default:
    //  Serial.println("Device is not a DS18x20 family device.");
      return;
  } 

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);      
  
  delay(1000);    

//  delay(1000);    
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         

  for ( i = 0; i < 9; i++) {       
    data[i] = ds.read();
  }
  OneWire::crc8(data, 8); 
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; 
    if (data[7] == 0x10) {      
      raw = (raw & 0xFFF0) + 12 - data[6];    }
  } else {
    byte cfg = (data[4] & 0x60);

    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    
  }

 
  celsius = (float)raw / 16.0;

   Serial.println(celsius);    
     
   char temperaturenow [15];
   dtostrf(celsius,7, 3, temperaturenow);  //// convert float to char
      
   client.publish("temp/sensor", temperaturenow);    /// send char
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {

      Serial.println("connected");
      // client.subscribe("event");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {

 temperature();   
 
  if (!client.connected()) {
    reconnect();

  }
  client.loop();

 delay(100);

  
}

