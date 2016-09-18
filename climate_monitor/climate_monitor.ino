/*****************************************************************
Phant_Ethernet.ino
Post data to SparkFun's data stream server system (phant) using
an Arduino and an Ethernet Shield.
Jim Lindblom @ SparkFun Electronics
Original Creation Date: July 3, 2014

This sketch uses an Arduino Uno to POST sensor readings to 
SparkFun's data logging streams (http://data.sparkfun.com). A post
will be initiated whenever pin 3 is connected to ground.

Before uploading this sketch, there are a number of global vars
that need adjusting:
1. Ethernet Stuff: Fill in your desired MAC and a static IP, even
   if you're planning on having DCHP fill your IP in for you.
   The static IP is only used as a fallback, if DHCP doesn't work.
2. Phant Stuff: Fill in your data stream's public, private, and 
data keys before uploading!

Hardware Hookup:
  * These components are connected to the Arduino's I/O pins:
    * D3 - Active-low momentary button (pulled high internally)
    * A0 - Photoresistor (which is combined with a 10k resistor
           to form a voltage divider output to the Arduino).
    * D5 - SPDT switch to select either 5V or 0V to this pin.
  * A CC3000 Shield sitting comfortable on top of your Arduino.

Development environment specifics:
    IDE: Arduino 1.0.5
    Hardware Platform: RedBoard & PoEthernet Shield

This code is beerware; if you see me (or any other SparkFun 
employee) at the local, and you've found our code helpful, please 
buy us a round!

Much of this code is largely based on David Mellis' WebClient
example in the Ethernet library.

Distributed as-is; no warranty is given.
*****************************************************************/
#include <SPI.h> // Required to use Ethernet
#include <Ethernet.h> // The Ethernet library includes the client
//#include <Progmem.h> // Allows us to sacrifice flash for DRAM
#include "DHT.h"

// DHT Sensor Setup
#define DHTPIN 2 // We have connected the DHT to Digital Pin 2
#define DHTTYPE DHT22 // This is the type of DHT Sensor (Change it to DHT11 if you're using that model)
DHT dht(DHTPIN, DHTTYPE); // Initialize DHT object

///////////////////////
// Ethernet Settings //
///////////////////////
// Enter a MAC address for your controller below.
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(54,86,132,254);  // numeric IP for data.sparkfun.com
char server[] = "data.sparkfun.com";    // name address for data.sparkFun (using DNS)
// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192,168,0,177);

// Initialize the Ethernet client library
// with the IP address and port of the server 
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;

/////////////////
// Phant Stuff //
/////////////////
const String publicKey = "your_public_key";
const String privateKey = "your_private_key";
const byte NUM_FIELDS = 2;
const String fieldNames[NUM_FIELDS] = {"humidity", "temp"};
String fieldData[NUM_FIELDS];
double t;
double h;


String name = "Ether-anon";

void setup()
{
  Serial.begin(115200);


  // Set Up Ethernet:
  setupEthernet();
  dht.begin();


}

void loop()
{
  h = dht.readHumidity();
  t = dht.readTemperature();
  fieldData[0] = String(h);
  fieldData[1] = String(t);
  
  Serial.println("Posting!");
  postData(); // the postData() function does all the work, 
              // check it out below.
  delay(58000);
}

void postData()
{
  // Make a TCP connection to remote host
  if (client.connect(server, 80))
  {
    // Post the data! Request should look a little something like:
    // GET /input/publicKey?private_key=privateKey&light=1024&switch=0&name=Jim HTTP/1.1\n
    // Host: data.sparkfun.com\n
    // Connection: close\n
    // \n
    client.print("GET /input/");
    client.print(publicKey);
    client.print("?private_key=");
    client.print(privateKey);
    for (int i=0; i<NUM_FIELDS; i++)
    {
      client.print("&");
      client.print(fieldNames[i]);
      client.print("=");
      client.print(fieldData[i]);
    }
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(server);
    client.println("Connection: close");
    client.println();
  }
  else
  {
    Serial.println(F("Connection failed"));
  } 

  // Check for a response from the server, and route it
  // out the serial port.
  while (client.connected())
  {
    if ( client.available() )
    {
      char c = client.read();
      Serial.print(c);
    }      
  }
  Serial.println();
  client.stop();
}

void setupEthernet()
{
  Serial.println("Setting up Ethernet...");
  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println(F("Failed to configure Ethernet using DHCP"));
    // no point in carrying on, so do nothing forevermore:
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());
  // give the Ethernet shield a second to initialize:
  delay(1000);
}

