/*************************************************** 
  This is an example for the Adafruit CC3000 Wifi Breakout & Shield

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
#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>
#include <string.h>
#include "utility/debug.h"

#define WLAN_SSID "XXXXXXXXXXXXX"   // Use your own network here           
#define WLAN_PASS "XXXXXXXXXXXXX"

#define WEBSITE   "XXXXXXXXXXXXX"   // Use your website and webpage here, doesn't seem to like an IP here
#define WEBPORT   80
#define WEBPAGE   "/"               // It's basically a file with "led8=1\n" as the only content

#define ADAFRUIT_CC3000_IRQ   3                     
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10

Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIVIDER);
boolean reading = false; 
String   get_request  = "";   

#define WLAN_SECURITY   WLAN_SEC_WPA2
#define IDLE_TIMEOUT_MS  3000     

uint32_t ip;
Adafruit_CC3000_Client www;

int connectTimeout = 3000;
int repeat_counter = 0;  

void setup(void)
{

  Serial.begin (115200);
  Serial.println (F("Hello, CC3000!\n")); 
  Serial.print ("Free RAM: "); 
  Serial.println (getFreeRam(), DEC);
  Serial.println (F("\nInitializing..."));
  if (!cc3000.begin())
  {
    Serial.println(F("Couldn't begin()! Check your wiring?"));
    while(1);
  }

  listSSIDResults();
   
  Serial.print(F("\nAttempting to connect to ")); 
  Serial.println(WLAN_SSID);
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) 
  {
    Serial.println(F("Failed!"));
    while(1);
  }
  
  Serial.println(F("Connected!"));
  
  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP())
  {
    delay(100);                                    // ToDo: Insert a DHCP timeout!  Why??
  }  
  while (! displayConnectionDetails()) 
  {
    delay(1000);
  }
  
  ip = 0;
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
    Serial.print(F(" HTTP/1.1\r\n"));
    Serial.print(F("Host: ")); 
    Serial.print(WEBSITE); 
    Serial.print(F("\r\n"));
    Serial.print(F("\r\n"));
    Serial.println();
    boolean sentContent = false;
    www.fastrprint(F("GET "));
    www.fastrprint(WEBPAGE);
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

void loop(void)
{
 delay(1000);
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
