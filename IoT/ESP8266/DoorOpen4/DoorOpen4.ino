/*
 Control de apertura por sensor con ESP8266
 Configurable:
  * WiFi
  * Request a web 
   
 Created by Rui Santos
 Modificado por jamesnr2, sin depurar
 Necesita de una resistencia para conectar a un pin GPIO
 
 All the resources for this project:
 http://randomnerdtutorials.com/
 http://randomnerdtutorials.com/door-status-monitor-using-the-esp8266/

 ARDUINO:
 https://polaridad.es/programar-un-modulo-wifi-esp8266-desde-el-ide-de-arduino/

 8266:
 http://www.instructables.com/id/ESP8266-Doorwindow-Sensor-With-SMS-Alarm/

 Pull_UP:
 https://www.arduino.cc/reference/en/language/variables/constants/constants/
 https://miarduinounotieneunblog.blogspot.com.es/2015/12/conexiones-pull-up-y-pull-down.html [ARDUINO]

 INTERRUPCIONES:
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
   
   const char* host = "maker.ifttt.com";
   const char* apiKey = "KEY";
   const int httpPort = 80;

   // Configuración servidor2
   const int port = 80;
   const char* host2 = "192.168.1.1"; // ip or dns
*/
#include "MyConfigLocal.h"  


const int timenOutServer = 10000;


//int pin = 12;  //GPIO12
//int pin = 5;  //GPIO5
int pin = D2;  //GPIO4

volatile int state = false;
volatile int flag = false;
String door_state = "closed";  // Inicialmente la puerta debe estar cerrada

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

boolean conectaWifi() {
    
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    
    WiFi.begin(ssid, password);

    unsigned long timeout = millis();
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      if (millis() - timeout > timenOutServer) {
        Serial.println(">>> WIFI Timeout !");
        return false;
      }
    }
   
    Serial.println("");
    Serial.println("WiFi connected");  
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println();
    return true;
}

void setup() {

    delay(2000); // Esperamos cuando alimentamos el dispositivo por primera vez
    Serial.begin(9600);
    //delay(1000);

    Serial.setTimeout(2000);
    Serial.println("Preparing the Door Status Monitor project...");
    Serial.println("La puerta debe estar cerrada");

    if (conectaWifi()== false){
      Serial.println("NO FUNCIONA EL WIFI");
    }
    
        
    //Serial.println("Preparando pines PULL_UP o no ");
    //pinMode(pin, INPUT_PULLUP);   
    pinMode(pin, INPUT);
    
    //Configuramos pin de interrupcion y el evento de llamada
    attachInterrupt(digitalPinToInterrupt(pin), changeDoorStatus, CHANGE);
    //Serial.println("Pines PULL_UP listos");
    

}


void enviarServidor(const char* myHost, int myHttpPort, const char* myApiKey, String path, String modo) {

 // 
    String url = path;
    
    Serial.print("connecting to ");
    Serial.println(myHost);
    
    WiFiClient client;
    // const int httpPort = 80;
    if (!client.connect(myHost, myHttpPort)) {
      Serial.println("connection server failed");
      return;
    }

    if (modo=="post"){
      
      url += myApiKey;
      url += "?value1=" + door_state;
      
      Serial.print("Requesting URL: ");
      Serial.println(url);
      client.print(String("POST ") + url + " HTTP/1.1\r\n" +
                   "Host: " + myHost + "\r\n" + 
                   "Content-Type: application/x-www-form- encoded\r\n" + 
                   "Content-Length: 13\r\n\r\n" +
                   //client.print(postData.length());
                   "value1=" + door_state + "\r\n" +                   
                   "Connection: close\r\n\r\n");
      // client.stop(); Si lo activas reinica el modulo
    }
    if (modo=="put"){
      //url = path;
      //url += myApiKey;
      //url += "?value1=" + door_state;
      url += door_state;
      
      Serial.print("Requesting URL: ");
      Serial.println(url);
      client.print(String("PUT ") + url + " HTTP/1.1\r\n" +
                   "Host: " + myHost + "\r\n" + 
                   "Content-Type: application/x-www-form-urlencoded\r\n" + 
                   "x-api-key: " + myApiKey + "\r\n" +  // "x-api-key: TU_KEY"
                   "Content-Length: 13\r\n\r\n" +
                   //client.print(postData.length());
                   "nombre=" + door_state + "\r\n" +                   
                   "Connection: close\r\n\r\n");
      // client.stop(); Si lo activas reinica el modulo
    }
    
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > timenOutServer) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        return;
      }
    }

    Serial.println();
    Serial.println("Recibiendo Datos");
    Serial.println("");
    // Read all the lines of the reply from server and print them to Serial
    while (client.available()) {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
  
    Serial.println();
    Serial.println("closing connection");
    Serial.println();
        
}

void loop() {

      if(flag){
          enviarServidor(host1, httpPort1, apiKey1, path1,"post");
          enviarServidor(host2, httpPort2, apiKey2, path2,"put");
          flag = false;
      }  
      delay(10);
}
