
#include <SoftwareSerial.h>
SoftwareSerial espSerial(2,3);
#define ssid ""
#define PASSWORD ""

String fmsg;
String cmd;
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);

#include <SPI.h>
#include <RFID.h>
#define SS_PIN 10
#define RST_PIN 9
RFID rfid(SS_PIN, RST_PIN);
String rfidCard;

void setup() 
{
  espSerial.begin(115200);
  Serial.begin(115200);
  espSetup();

  Serial.println("Starting the RFID Reader...");

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Tap your id card");

  SPI.begin();
  rfid.init();
}

void loop() 
{ 
  if (rfid.isCard()) {
    if (rfid.readCardSerial()) {
      rfidCard = String(rfid.serNum[0]) + " " + String(rfid.serNum[1]) + " " + String(rfid.serNum[2]) + " " + String(rfid.serNum[3]);
      Serial.println(rfidCard);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Hi" );
      lcd.setCursor(0, 1);
      lcd.print("Show face to camera");
      delay(2000);
      
      int n=rfidCard.length();

      espSerial.print("AT+CIPSEND=0,"+String(n)+"\r\n");
      delay(250);
      espSerial.print(rfidCard);
      while (1){
        espSerial.print("AT+CIPSERVER=1,8080");
        delay(250);
        String mesg = espSerial.readString();
        if (mesg.indexOf("here")>=0){
          int n=mesg.indexOf("here");
          int byte=(mesg.substring(n-3,n-1)).toInt();
          Serial.println(mesg.substring(n+4,n+byte));
          String match=mesg.substring(n+4,n+5);
          String name=mesg.substring(n+6,n+byte);
          Serial.println(match);
          Serial.println(name);
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print(name);
          lcd.setCursor(0,1);
          if (match=="T"){
            lcd.print("Face recognized");
          }
          else{
            lcd.print("Not recognized");
          }
          delay(2500);
          break;
        }
      }
    }
    int counter=0;
    Serial.println("-------------SET PORT 8080----------");
    while(1){
     espSerial.println("AT+CIPSERVER=1,8080");
     delay(500);
     if(espSerial.find("OK")){
       counter+=1;
       }
     if(counter == 1){
       Serial.println("Setting Port 8080");
       counter = 0;
       break;
       }
     else{
       Serial.println("Waiting...");
       }
     }

    Serial.println("---------SETUP DONE--------");
    delay(500);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.noBacklight();
    delay(250);
    lcd.backlight();
    lcd.print("Tap your id card");
  }

}

void espSetup()
{
  int counter=0;
    Serial.println("-----------Setup Begin------------");
    
    // Step 1 Reset ESP8266
    espSerial.println("AT+RST");
    delay(1000);
    getResponse();
    
    // Step 2 Connection Test of ESP8266
    Serial.println("-------------TEST AT----------");
       espSerial.println("AT");
       delay(1000);      
      getResponse();

    // Step 3 Station Mode
    Serial.println("-------------STATION MODE----------");
    while(1){
      espSerial.println("AT+CWMODE=3");
      delay(250);
      if(espSerial.find("OK")){
        counter+=1;
        }
      if(counter == 1){
        Serial.println("Station Mode Setted 1");
        counter=0;
        break;
        }
      else{
        Serial.println("Waiting...");
        }
      }
    
    // Step 4 Connect to WiFi
    Serial.println("-------------CONNECT TO WIFI----------");
    while(1){
      cmd = "AT+CWJAP=\"";
      cmd += ssid;
      cmd += "\",\"";
      cmd += PASSWORD;
      cmd += "\"";
      espSerial.println(cmd);
      if(espSerial.find("OK")){
        counter+=1;
        }
      if(counter == 1){
        Serial.println("Connection Established");
        counter=0;
        break;
        }
      else{
        Serial.println("Waiting...");
        }
      }
      
    // Step 5 Find IP Address
    Serial.println("-------------FIND IP----------");
    espSerial.println("AT+CIFSR");
    delay(1000);
    getResponse();

    // Step 6 Setting Multiple Connections
    Serial.println("-------------SET MULTIPLE CONNECTIONS----------");
    while(1){
      espSerial.println("AT+CIPMUX=1");
      delay(250);
      if(espSerial.find("OK")){
        counter+=1;
        }
      if(counter == 1){
        Serial.println("Multiple Connections Enabled");
        counter = 0;
        break;
        }
      else{
        Serial.println("Waiting...");
        }
      }

    // Step 7 Setting Port 8080
    Serial.println("-------------SET PORT 8080----------");
    while(1){
     espSerial.println("AT+CIPSERVER=1,8080");
     delay(500);
     if(espSerial.find("OK")){
       counter+=1;
       }
     if(counter == 1){
       Serial.println("Setting Port 8080");
       counter = 0;
       break;
       }
     else{
       Serial.println("Waiting...");
       }
     }

    Serial.println("---------SETUP DONE--------");
    delay(500);
}

void getResponse()
{
  String response;
  
  while(espSerial.available()){
      char c = espSerial.read();
      response += c;
      }
  Serial.println(response);
}

String getString(String flmsg)
{ 
  int lng = 0,delimiter = 0;
  String lmsg;
  
  lng = flmsg.length();  
  for(int i=0;i<lng;i++){
     if(flmsg[i] == ':'){
      delimiter = i;
      }
     if(i > delimiter && delimiter != 0){
      lmsg += flmsg[i];
      }
    }
  fmsg = ""; 
  return lmsg;
}
