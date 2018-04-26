/*
 Created by Rui Santos
 
 All the resources for this project:
 http://randomnerdtutorials.com/
 http://randomnerdtutorials.com/door-status-monitor-using-the-esp8266/

 ARDUINO:
 https://polaridad.es/programar-un-modulo-wifi-esp8266-desde-el-ide-de-arduino/

 8266 
 http://www.instructables.com/id/ESP8266-Doorwindow-Sensor-With-SMS-Alarm/

 Pull_UP
 https://www.arduino.cc/reference/en/language/variables/constants/constants/
 https://miarduinounotieneunblog.blogspot.com.es/2015/12/conexiones-pull-up-y-pull-down.html [ARDUINO]

 INTERRUPCIONES.
 https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/
 
 PINES: 
 https://www.lightinthebox.com/es/nodemcu-esp8266-lua-wifi-internet-desarrollo-bordo_p6142020.html
 http://www.electronicwings.com/nodemcu/nodemcu-gpio-with-arduino-ide
 
 Based on some ESP8266 code examples 
*/

#include <ESP8266WiFi.h>

/* ARCHIVO "MyConfigLocal.h" 
   Contiene la configuración local que usamos: wifi, host, etc.
   Se puede substituir por las variables respectivas
   const char* ssid = "MY_SSID";
   const char* password = "MI_CLAVE";
*/
#include "MyConfigLocal.h"  



// Configuración servidor1
const char* host = "maker.ifttt.com";
const char* apiKey = "OaA9REbZgbOaXt9TMgNh0";
const int httpPort = 80;

// Configuración servidor2
const int port = 80;
const char* host2 = "192.168.1.1"; // ip or dns

const int timenOutServer = 5000;


//int pin = 12;  //GPIO12
//int pin = 5;  //GPIO5
int pin = D2;  //GPIO4

volatile int state = false;
volatile int flag = false;
// const char* door_state = "closed";
String door_state = "closed";

unsigned long previousMillis = 0; 
const long interval = 3000;

void changeDoorStatus() {

    unsigned long currentMillis = millis();
 
    if(currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;   
    
        state = !state;
        if(state) {
            door_state = "opened";
        }
        else{
            door_state = "closed";
        }
        flag = true;
        Serial.println(state);
        Serial.println(door_state);
    }
    
}


void conectaWifi() {
    
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
    Serial.println();
}

void setup() {

    delay(2000); // Esperamos cuando alimentamos el dispositivo por primera vez
    Serial.begin(9600);
    //delay(1000);

    Serial.setTimeout(2000);
    Serial.println("Preparing the Door Status Monitor project...");
    Serial.println("La puerta debe estar cerrada");

    conectaWifi();
        
    Serial.println("Preparando pines PULL_UP o no ");
    pinMode(pin, INPUT);
    //pinMode(pin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(pin), changeDoorStatus, CHANGE);
    Serial.println("Pines PULL_UP listos");
    

}


void enviarServidor() {
    Serial.print("connecting to ");
    Serial.println(host);
    
    WiFiClient client;
    // const int httpPort = 80;
    if (!client.connect(host, httpPort)) {
      Serial.println("connection failed");
      return;
    }

    String url = "/trigger/Puerta_estado/with/key/";
    url += apiKey;
    url += "?value1=" + door_state;
    
    Serial.print("Requesting URL: ");
    Serial.println(url);
    client.print(String("POST ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" + 
                 "Content-Type: application/x-www-form- encoded\r\n" + 
                 "Content-Length: 13\r\n\r\n" +
                 //client.print(postData.length());
                 "value1=" + door_state + "\r\n" +                   
                 "Connection: close\r\n\r\n");
    //client.stop();

    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > timenOutServer) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        return;
      }
    }
  
    // Read all the lines of the reply from server and print them to Serial
    while (client.available()) {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
  
    Serial.println();
    Serial.println("closing connection");
        
}



void enviarServidor1() {
    Serial.print("connecting to ");
    Serial.println(host);
    
    WiFiClient client;
    // const int httpPort = 80;
    if (!client.connect(host, httpPort)) {
      Serial.println("connection failed");
      return;
    }

    String url = "/trigger/Puerta_estado/with/key/";
    url += apiKey;
    url += "?value1=" + door_state;
    
    Serial.print("Requesting URL: ");
    Serial.println(url);
    client.print(String("POST ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" + 
                 "Content-Type: application/x-www-form- encoded\r\n" + 
                 "Content-Length: 13\r\n\r\n" +
                 "value1=" + door_state + "\r\n");
    
}


void enviarServidor2() {
// Debe estar dentro del loop
// https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/examples/WiFiClientBasic/WiFiClientBasic.ino

  Serial.print("connecting to ");
  Serial.println(host2);

  // Use WiFiClient class to create TCP connections
  WiFiClient client2;

  if (!client2.connect(host, port)) {
    Serial.println("connection failed");
    Serial.println("wait 5 sec...");
    delay(5000);
    return;
  }

  // This will send the request to the server
  client2.println("Send this data to server");

  //read back one line from server
  String line = client2.readStringUntil('\r');
  Serial.println(line);

  Serial.println("closing connection");
  client2.stop();

  Serial.println("wait 5 sec...");
  delay(5000);
}

void enviarServidor3() {
  delay(5000);
  //++value;

  Serial.print("connecting to ");
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  // We now create a URI for the request
  String url = "/input/";
  //url += streamId;
  url += "?private_key=";
  //url += privateKey;
  url += "&value=";
  //url += value;

  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  Serial.println();
  Serial.println("closing connection");
}


void loop() {

      if(flag){
          enviarServidor();
          flag = false;
      }  
      delay(10);
}
