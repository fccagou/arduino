/*

 Some code to interact between http server and hardware.
 
 Circuit:
 * Ethernet
 * Led connected on pin 8
 

 created Nov 09 2014
 by Francois Chenais

 This code is in the public domain.
 
 */

#include <SPI.h>
#include <Ethernet.h>


// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x01 };

IPAddress ip(192,168,1,20);

// initialize the library instance:
EthernetClient client;

const unsigned long requestInterval = 3000;  // delay between requests

char serverName[] = "192.168.1.2";
int serverPort = 8051;

unsigned long lastAttemptTime = 0;
String http_response = "";

int ledRouge = 8;



void setup() {
  pinMode(ledRouge, OUTPUT);
  // reserve space for the strings:
  http_response.reserve(512);

 // Open serial communications and wait for port to open:
  Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }


  // attempt a DHCP connection:
  Serial.println("Attempting to get an IP address using DHCP:");
  if (!Ethernet.begin(mac)) {
    // if DHCP fails, start with a hard-coded address:
    Serial.println("failed to get an IP address using DHCP, trying manually");
    Ethernet.begin(mac, ip);
  }
  Serial.print("My address:");
  Serial.println(Ethernet.localIP());

}



void loop()
{

  http_request(serverName, serverPort, "/arduino/led1/get");
  
  Serial.print("Led1: "); 
  Serial.println(http_response);

  switch (http_response[0]) {

    case '0':
      digitalWrite(ledRouge, LOW);
      break;

    case '1':
      digitalWrite(ledRouge, HIGH);
      break;
    
   }
    
  
  int time2wait = lastAttemptTime + requestInterval - millis() ;
  
  if ( time2wait > 0 ) {
    // if you're not connected, and two minutes have passed since
    // your last connection, then attempt to connect again:
    Serial.print("Waiting ");
    Serial.print( time2wait / 1000 );
    Serial.println (" sec before next request");
    
    delay(time2wait);
  }
}






void http_request(char * server, int port, char *path) {
  
  bool inBody = false;
  int nbNl = 0;

  // attempt to connect, and wait a millisecond:
  Serial.print("connecting to server...");
  Serial.print(server);
  Serial.print(" on port ");
  Serial.println(port);
  
  while ( ! client.connect(server, port) ) {
    Serial.print("connecting to server...");
    Serial.print(server);
    Serial.print(" on port ");
    Serial.println(port);
    
  }
  
  Serial.println("making HTTP request...");
  
  // make HTTP GET request to twitter:
  client.print("GET ");
  client.print(path);
  client.println(" HTTP/1.1");
  client.print("HOST: ");
  client.println(server);
  client.println("Connection: close");
  client.println();
  
  http_response = "";
  while (client.connected()) {
    while (client.available()) {
      // read incoming bytes:
      char inChar = client.read();
//      Serial.print("[");
      Serial.print(inChar);
//      Serial.print("]");
   
      if (inBody) {
          http_response += inChar;
      } else {
       
        switch(inChar) {
          
          case '\n':
          
            if (nbNl == 1) {
//              Serial.println("HEADER-END");
//              Serial.println("BODY-BEGIN");
              inBody = true;
            } else {
              nbNl++;
            }
         
//            Serial.print("NBnl: ");
//            Serial.println(nbNl);
            break;
            
         case '\r':
           break;
           
         default:   
            nbNl = 0;
            break;
        }
      }  
    }
   
    if (inBody) {
      
      Serial.print("\nGot VALUE (");
      Serial.print(http_response);
      Serial.println(")");
      inBody = false;
    }
  }
  
  client.stop();
  
  // note the time of this connect attempt:
  lastAttemptTime = millis();
}   

