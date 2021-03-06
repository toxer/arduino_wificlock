#include <EEPROM.h>




#include <Wire.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>

#define DEBUG false

#define SDA 1 //TX 
#define SCL 3 //RX
#define DEVICE 8 //Slave device address


#define SETUP_NET_SSID "SetupNet"
#define TX 1
#define RX 3
#define GPIO0 0
#define GPIO2 2
#define TIMEOUT 60000
#define EEPROMSIZE 4096
//Timeout di invio dei messaggi
#define MESSAGE_TIMEOUT 30000


#define BLUE_LED 0
#define CONTROL_PIN 2 //GIPO2

#define ESCAPE_SEQUENCE_STRING "\n"
#define ESCAPE_SEQUENCE_CHAR '\n'

//codici messaggi \t fa da carattere di split tra comando e body
#define NTP_PACKET_NOT_READY "10 "   //codice pacchetto ntp non rilevato
#define NTP_MESSAGE_CODE "11 "   //codice messaggio npt
#define WIFI_DOWN "00 " //rete wifi non presente
#define WIFI_UP "01 "
#define AD_HOC_NET "02 " //in modalità rete ad hoc


String ssid ;
String password ;
String ntpServerName;
String gtm = "0";

String serviceServer;
int networkMode = LOW;

WiFiServer server(23);
int ticker;
unsigned long currentNTPTime;
// Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
const unsigned long seventyYears = 2208988800UL;
String  localIP;



//metodo di trasmissione I2C non attivo in DEBUG
void I2CSend(String s) {
  if (!DEBUG) {
    Wire.beginTransmission(DEVICE);
    Wire.write(s.c_str());

    Wire.endTransmission();

  } else {
    Serial.println("I2C--> " + s);
  }

}


//parte relativa al server ntp
IPAddress timeServerIP;
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
WiFiUDP udp;
unsigned int localPort = 2390;

unsigned long sendNTPpacket(IPAddress& address)
{
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

void getNTPTime() {
  WiFi.hostByName(ntpServerName.c_str(), timeServerIP);
  //Serial.println("NTP SERVER:"+timeServerIP);
  sendNTPpacket(timeServerIP); // send an NTP packet to a time server
  delay(1000);
  int cb = udp.parsePacket();
  if (!cb) {
    Serial.println("no packet yet");
    I2CSend(NTP_PACKET_NOT_READY);
    return;
  }
  Serial.print("packet received, length=");
  Serial.println(cb);
  // We've received a packet, read the data from it
  udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
  unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
  unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
  // combine the four bytes (two words) into a long integer
  // this is NTP time (seconds since Jan 1 1900):
  unsigned long secsSince1900 = highWord << 16 | lowWord;
  Serial.print("Seconds since Jan 1 1900 = " );
  Serial.println(secsSince1900);
  unsigned long epoch = secsSince1900 - seventyYears;
  // print Unix time:
  Serial.println(epoch);
  String currentTime = "";

  currentTime = currentTime + String((((epoch  % 86400L) / 3600)) + gtm.toInt()) + ":";
  if ( ((epoch % 3600) / 60) < 10 ) {
    currentTime = currentTime + "0";
  }
  currentTime = currentTime + String((epoch  % 3600) / 60);
  currentTime = currentTime + ":";
  if ( (epoch % 60) < 10 ) {
    currentTime = currentTime + "0";
  }
  currentTime = currentTime + String((epoch % 60));

  Serial.println(currentTime);

  I2CSend(NTP_MESSAGE_CODE + currentTime);



}

void connectToLocalWiFi() {
  Serial.println("SSID:" + ssid);
  boolean status = false;
  while ( status != WL_CONNECTED) {

    for (int i = 0; i < 3; i++) {
      controlLed(BLUE_LED, HIGH);
      delay(200);
      controlLed(BLUE_LED, LOW);
      delay(200);
    }
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    if (password != "") {
      status = WiFi.begin(ssid.c_str(), password.c_str());
    }
    else {
      status = WiFi.begin(ssid.c_str());
    }
    // wait 10 seconds for connection:
    I2CSend(WIFI_DOWN); //Newtwork down
    for (int i = 0; i < 5; i++) {
      controlLed(BLUE_LED, HIGH);
      delay(1000);
      controlLed(BLUE_LED, LOW);
      delay(1000);
    }

  }
  Serial.println("Network connected");
  I2CSend(WIFI_UP); //Network connect;
  controlLed(BLUE_LED, HIGH);
  IPAddress myAddr = WiFi.localIP();

  byte first_octet = myAddr[0];
  byte second_octet = myAddr[1];
  byte third_octet = myAddr[2];
  byte fourth_octet = myAddr[3];
  localIP =  String(first_octet)  + "." + second_octet  + "." + third_octet  + "." + fourth_octet;
  I2CSend("IP" + localIP);
  Serial.println(localIP);

  //richiedo il pacchetto dopo 5 sec
  delay(5000);
  getNTPTime();

}


void controlLed(int led, int status) {
  if (DEBUG) {
    return;
  }
  digitalWrite(led, status);
}

void setup() {
  if (DEBUG) {
    Serial.begin(57600);
  } else {

    pinMode(BLUE_LED, OUTPUT);
  }

  Serial.println("Begin");
  EEPROM.begin(EEPROMSIZE);
  delay(5000);
  Serial.println("Read mode");
  pinMode(CONTROL_PIN, INPUT);
  networkMode = digitalRead(CONTROL_PIN);

  //attivo la modalità i2c se non in debug
  if (!DEBUG) {
    Wire.begin(SDA, SCL);
  }

  if (networkMode == LOW) {
    I2CSend(AD_HOC_NET);

    controlLed(BLUE_LED, HIGH);
    //programmazione tramite rete ad hoc
    Serial.println("RETE AD HOC");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(SETUP_NET_SSID);
    server.begin();




  } else {
    Serial.println("NETWORK CONNECTION");




    //attivo la rete

    //se sono in modalità programmazione (pin tx-1 a hight) attivo la rete ssid




    //modalità NETWORK
    WiFi.mode(WIFI_STA);

    readConfiguration();
    if (ssid == "") {
      Serial.println("NO NETWORK FOUND");
      while (true) {

        controlLed(BLUE_LED, HIGH);
        delay(200);

        controlLed(BLUE_LED, LOW);
        delay(200);
      }
    } else {
      connectToLocalWiFi();
      udp.begin(localPort);

    }
  }
}




void clearEEPROM() {
  for (int i = 0 ; i < EEPROMSIZE ; i++) {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
  delay(1000);
  Serial.println("EEPROM CLEARED");
}
void saveConfiguration() {
  clearEEPROM();

  String toSave = ssid + ESCAPE_SEQUENCE_STRING + password + ESCAPE_SEQUENCE_STRING + ntpServerName + ESCAPE_SEQUENCE_STRING+gtm+ESCAPE_SEQUENCE_STRING;
  Serial.println("String to save");
  Serial.println(toSave);
  int len = toSave.length() + 1;
  char toSaveBuffer [len];
  toSave.toCharArray(toSaveBuffer, len);
  for (int i = 0; i < len; i++) {
    EEPROM.write(i, toSaveBuffer[i]);
  }
  EEPROM.commit();
  delay(1000);
  Serial.println("EEPROM SAVED");
}


void readConfiguration() {
  ssid = "";
  password = "";
  ntpServerName = "";
  gtm="";
  int currentIndex = 0;
  int offset = 0;
  for (int i = 0; i < EEPROMSIZE; i++) {
    char c = (char)EEPROM.read(i);
    if (c != ESCAPE_SEQUENCE_CHAR) {
      ssid = ssid + c;
    } else {
      break;
    }
    currentIndex++;
  }
  currentIndex++;
  offset = currentIndex;

  for (int i = offset; i < EEPROMSIZE; i++) {
    char c = (char)EEPROM.read(i);
    if (c != ESCAPE_SEQUENCE_CHAR) {
      password = password + c;
    } else {
      break;
    }
    currentIndex++;
  }
  currentIndex++;
  offset = currentIndex;
  for (int i = offset; i < EEPROMSIZE; i++) {
    char c = (char)EEPROM.read(i);
    if (c != ESCAPE_SEQUENCE_CHAR) {
      ntpServerName = ntpServerName + c;
    } else {
      break;
    }
    currentIndex++;
  }
  currentIndex++;
  offset = currentIndex;

  for (int i = offset; i < EEPROMSIZE; i++) {
    char c = (char)EEPROM.read(i);
    if (c != ESCAPE_SEQUENCE_CHAR) {
      gtm = gtm + c;
    } else {
      break;
    }
    currentIndex++;
  }
  currentIndex++;
  offset = currentIndex;

  Serial.println("SSID:" + ssid + "\n PASS:" + password + "\nNTP:" + ntpServerName);
}




void loop() {

  //modalità ad-hoc
  if (networkMode == LOW) {
    WiFiClient client = server.available();
    IPAddress ip = WiFi.localIP();
    // an http request ends with a blank line
    boolean connected = false;
    String tmp = "";
    while (client.connected()) {
      if (!connected) {
        //to TEST for empty chip
        readConfiguration();
        connected = true;
        Serial.println("Client connected");
        for (int i = 0; i < 3; i++) {
          controlLed(BLUE_LED, HIGH);
          delay(200);
          controlLed(BLUE_LED, LOW);
          delay(200);
        }
      }
      int index = 0;

      if (client.available()) {
        ticker = 0;
        char c = client.read();
        tmp = tmp + c;
      } else {
        tmp.trim();
        if (tmp != "") {
          Serial.println(" " + tmp);

          if (tmp == "close") {
            client.stop();

            ticker = 0;

          } else if (tmp.startsWith("ssid")) {
            ssid = tmp.substring(4);
            ssid.trim();

            client.println(ssid);



          } else if (tmp.startsWith("pass")) {
            password = tmp.substring(4);
            password.trim();
            client.println(password);


          }
          else if (tmp.startsWith("ntp")) {
            ntpServerName = tmp.substring(3);
            ntpServerName.trim();
            client.println(ntpServerName);


          }
           else if (tmp.startsWith("gtm")) {
            gtm = tmp.substring(3);
            gtm.trim();
            client.println(gtm);
          }
          else if (tmp.startsWith("save")) {
            saveConfiguration();
            client.println("Saved");
            //controllo qunato salvato


          }
          else if (tmp.startsWith("clear")) {
            clearEEPROM();
            client.println("Cleared EEPROM");
            //controllo qunato salvato
            client.println("DONE");

          }

          else if (tmp.startsWith("status")) {
            readConfiguration();
            client.println("SSID: " + ssid);
            client.println("PASS: " + password);
            client.println("NPT: " + ntpServerName);
            client.println("LOCAL_IP: " + localIP);

            //controllo qunato salvato


          }

          else {
            Serial.println("Command unknow");

          }
          client.flush();
          tmp = "";
        }

        delay (1);
        ticker++;
        if (ticker > TIMEOUT) {
          client.stop();

          tmp = "";
          ticker = 0;
        }
      }
    }
    if (connected) {
      Serial.println("Client disconnect");
      connected = false;

    }


  } else {
    if (WiFi.status() == WL_CONNECTED) {
      //ora dall'ntp
      getNTPTime();
      if (!DEBUG) {
        delay(MESSAGE_TIMEOUT);
      } else {
        delay(5000);
      }
    } else {
      connectToLocalWiFi();
    }
  }


}
