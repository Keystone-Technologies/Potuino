/*************************************************** 
  Designed specifically to work with the Adafruit WiFi products:
  ----> https://www.adafruit.com/products/1469

  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
  
  Modified by Peter Dalmaris tas part of Udemy's Arduino step by
  Step course and modified again by One of Peter's students.
	
 ****************************************************/
 
#include <RFIDuino.h>
#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>
#include <string.h>
#include "utility/debug.h"

#define WLAN_SSID "XXXXXXXXXX"   // Use your own network here           
#define WLAN_PASS "XXXXXXXXXX"

#define WEBSITE   ""   // Use your website and webpage here, doesn't seem to like an IP here
//#define WEBSITEIP IPAddress(192,168,0,102)   // Use your website and webpage here, doesn't seem to like an IP here
#define WEBPORT   80
#define WEBPAGE   "/"               // It's basically a file with "led8=1\n" as the only content

#define ADAFRUIT_CC3000_IRQ   3                     
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10

Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIVIDER);
boolean reading = false; 
String  get_request  = "";   

#define WLAN_SECURITY   WLAN_SEC_WPA2
#define IDLE_TIMEOUT_MS  3000     

uint32_t ip;
Adafruit_CC3000_Client www;

RFIDuino myRFIDuino(1.2);   //initialize an RFIDuino object for hardware version 1.2

byte tagData[5]; //Holds the ID numbers from the tag  
int connectTimeout = 3000;
int repeat_counter = 0;  

void setup(void)
{

  Serial.begin (115200);
  Serial.println ("Hello, CC3000!\n"); 
  Serial.print ("Free RAM: "); 
  Serial.println (getFreeRam(), DEC);
  Serial.println ("\nInitializing...");
  if (!cc3000.begin())
  {
    Serial.println("Couldn't begin()! Check your wiring?");
    while(1);
  }

  listSSIDResults();
   
  Serial.print("\nAttempting to connect to "); 
  Serial.println(WLAN_SSID);
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) 
  {
    Serial.println("Failed!");
    while(1);
  }
  
  Serial.println("Connected!");
  
  Serial.println("Request DHCP");
  while (!cc3000.checkDHCP())
  {
    delay(1000);                                    // ToDo: Insert a DHCP timeout!  Why??
  }  
  while (! displayConnectionDetails()) 
  {
    delay(1000);
  }
  
  Serial.println("Welcome to the RFIDuino Serial Example. Please swipe your RFID Tag.");
  
}

void loop(void)
{
  //scan for a tag - if a tag is sucesfully scanned, return a 'true' and proceed
  if(myRFIDuino.scanForTag(tagData) == true)
  {
    Serial.print("RFID Tag ID:"); //print a header to the Serial port.
    char *tag;
    tag[0]=(char)tagData[0];
    tag[1]=(char)tagData[1];
    tag[2]=(char)tagData[2];
    tag[3]=(char)tagData[3];
    tag[4]=(char)tagData[4];
		for(int n=0;n<5;n++)
    {
      Serial.print(tagData[n],DEC);  //print the byte in Decimal format
      if(n<4)//only print the comma on the first 4 nunbers
      {
        Serial.print(",");
      }
    }
		Serial.print("\n\r");//return character for next line
    //txInput(tag);
    digitalWrite(myRFIDuino.led2,HIGH);     //turn green LED on
    //digitalWrite(myRFIDuino.buzzer, HIGH);   //turn the buzzer on
    delay(250);                             //wait for 1 second
    //digitalWrite(myRFIDuino.buzzer, LOW);    //turn the buzzer off
    digitalWrite(myRFIDuino.led2,LOW);      //turn the green LED off
  }
}

void txInput (char *str)
{
  #ifdef WEBSITEIP
  ip = WEBSITEIP;
  #else
  ip = 0;
  #endif
  Serial.print(WEBSITE); 
  Serial.print(F(" -> "));
  while (ip == 0) 
  {
    //if (!isIP(WEBSITE, &ip) && !cc3000.getHostByName(WEBSITE, &ip))  // Something like this would allow WEBSITE to be an IP
    if (!cc3000.getHostByName(WEBSITE, &ip))
      {
        Serial.println(F("Couldn't resolve!"));
      }
    delay(500);
  }
  cc3000.printIPdotsRev(ip);

  Adafruit_CC3000_Client www = cc3000.connectTCP(ip, WEBPORT);
  
  if (www.connected()) 
  {
    Serial.println();
    Serial.println(F("HTTP Request:"));
    Serial.print(F("GET "));
    Serial.print(WEBPAGE);
    Serial.print(F("?"));
    Serial.print(str);
    Serial.print(F(" HTTP/1.1\r\n"));
    Serial.print(F("Host: ")); 
    Serial.print(WEBSITE); 
    Serial.print(F("\r\n"));
    Serial.print(F("\r\n"));
    Serial.println();
    boolean sentContent = false;
    www.fastrprint(F("GET "));
    www.fastrprint(WEBPAGE);
    www.fastrprint(F("?"));
    www.fastrprint(str);
    www.fastrprint(F(" HTTP/1.1\r\n"));
    www.fastrprint(F("Host: ")); 
    www.fastrprint(WEBSITE); 
    www.fastrprint(F("\r\n"));
    www.fastrprint(F("\r\n"));
    www.println();
    Serial.println(F("Request sent"));
  } 
  else 
  {
    Serial.println(F("Connection failed"));    
    return;
  }

  Serial.println(F("---------------------------------------------"));
  Serial.println(F("HTTP Response:"));
  
  unsigned long lastRead = millis();
  while (www.connected() && (millis() - lastRead < IDLE_TIMEOUT_MS)) 
  {
    boolean currentLineIsBlank = true;
    get_request = "";     
    boolean sentContent = false;
    
    while (www.available()) 
    { 
      char c = www.read();
      Serial.print(c);
      lastRead = millis ();

      if(reading && c == '\n') 
      { 
        reading = false;  
        parseGetRequest(get_request);
        break;
      }        

      if(reading)
      { 
        get_request += c;
      }   
           
      if (reading && c=='\n')
      {
        break; 
      }  
           
      if (c == '\n' && currentLineIsBlank) 
      {
        reading = true; 
      }
      
      if (c == '\n') 
      {
        currentLineIsBlank = true;
      }
       
      else if (c != '\r') 
      {
        currentLineIsBlank = false;
      }
    }
  }
   www.close();
   Serial.println(F("----------------------------------------------"));
   cc3000.disconnect();
   Serial.println(F("\n\nDisconnecting"));
   delay(3000);  
}

bool displayConnectionDetails (void)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;
  
  if(!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    Serial.println(F("Unable to retrieve the IP Address!\r\n"));
    return false;
  }
  else
  {
    Serial.print(F("\nIP Addr: ")); cc3000.printIPdotsRev(ipAddress);
    Serial.print(F("\nNetmask: ")); cc3000.printIPdotsRev(netmask);
    Serial.print(F("\nGateway: ")); cc3000.printIPdotsRev(gateway);
    Serial.print(F("\nDHCPsrv: ")); cc3000.printIPdotsRev(dhcpserv);
    Serial.print(F("\nDNSserv: ")); cc3000.printIPdotsRev(dnsserv);
    Serial.println();
    return true;
  }
}

String parseGetRequest (String &str) 
  {
  Serial.print (F ("Parsing this string:"));
  Serial.println (str);
  int led_index = str.indexOf ("led");
  int led_pin = str [led_index + 3] - '0';
  int led_val = str [led_index + 5] - '0';
  //executeInstruction (led_pin, led_val);
  executeInstruction (13, led_val); // Code above looks to pull just one character for the pin
}

void executeInstruction (int pin, int val)
{   Serial.println(F("Executing instruction"));
    pinMode(pin, OUTPUT);
    digitalWrite(pin, val);
    delay(5000);
    Serial.println(F("Done!"));
}

void listSSIDResults(void)
{
  uint32_t index;
  uint8_t valid, rssi, sec;
  char ssidname[33]; 

  if (!cc3000.startSSIDscan(&index))
  {
    Serial.println(F("SSID scan failed!"));
    return;
  }

  Serial.print(F("Networks found: ")); Serial.println(index);
  Serial.println(F("================================================"));

  while (index) 
  {
    index--;
    valid = cc3000.getNextSSID(&rssi, &sec, ssidname);
    Serial.print(F("SSID Name    : ")); Serial.print(ssidname);
    Serial.println();
    Serial.print(F("RSSI         : "));
    Serial.println(rssi);
    Serial.print(F("Security Mode: "));
    Serial.println(sec);
    Serial.println();
  }
  Serial.println(F("================================================"));

  cc3000.stopSSIDscan();
}
