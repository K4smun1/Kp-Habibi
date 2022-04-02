//Wifi & mqtt
#include <WiFi.h>
#include <PubSubClient.h>

//autentikasi wifi
#define WIFI_AP_SSID "WATER_QUALITY"
#define WIFI_AP_PASS "prodigy123"
#define WIFI_STATE_AP 0
#define WIFI_STATE_STA 1
#define CHECK_WIFI_TIME 10000

#define mqtt_server   "test.mosquitto.org"
#define mqtt_port     1883
#define mqtt_user     "kawaki"
#define mqtt_pass     "12345678"
#define mqtt_clientid "mqtt-explorer-6ada7f74"

//oled library
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

/* Uncomment the initialize the I2C address , uncomment only one, If you get a totally blank screen try the other*/
#define i2c_Address 0x3c //initialize with the I2C addr 0x3C Typically eBay OLED's
//#define i2c_Address 0x3d //initialize with the I2C addr 0x3D Typically Adafruit OLED's

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1   //   QT-PY / XIAO

#define TROUBLESHOOT
#define RX            16   //pin 1 MAX485
#define RE            4   //pin 2&3 MAX485
#define TX            17   //pin 4 MAX485
#define modbus        Serial1
#define led           25   //led
#define setWifi       26   //setWifi

//json-ing
#include <ArduinoJson.h>

const int capacity = JSON_OBJECT_SIZE(5);
StaticJsonDocument<capacity> doc_Send;
StaticJsonDocument<capacity> doc_Recv;
// Declare a buffer to hold the json doc
char mqtt_Msg[128];

const char* ssid = "HabibiGarden";
const char* pass = "prodigy123";

const char* Ap_ssid = WIFI_AP_SSID;
const char* Ap_pass = WIFI_AP_PASS;

int espMode = WIFI_STATE_STA;
bool wifiSet = true;
bool newValue = false; //for waiting new msg from header
bool unDone = true; //for determining to end AP mode

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header, newSsid, newPass;

//logo declaration
const unsigned char PENS [] PROGMEM = {
  // 'LOGO PENS (2), 64x64px
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x03, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xf8, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x07, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x03, 0xf0, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xf8, 0x0f, 0xf8, 0x00, 0x00, 
  0x00, 0x00, 0x3f, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xfc, 0x0f, 0xfe, 0x00, 0x00, 
  0x00, 0x00, 0x7f, 0xe0, 0x03, 0xff, 0x00, 0x00, 0x00, 0x07, 0x3f, 0x80, 0x00, 0xfe, 0x00, 0x00, 
  0x00, 0x1f, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0xff, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x01, 0xff, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x03, 0xfe, 0x0f, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x03, 0xfc, 0x3f, 0xe0, 0x01, 0xfc, 0x00, 0x00, 
  0x03, 0xf8, 0x7f, 0xe0, 0x03, 0xff, 0x00, 0x00, 0x03, 0xf0, 0xff, 0xf0, 0x03, 0xff, 0x80, 0x00, 
  0x01, 0xc1, 0xff, 0xfc, 0x0f, 0xff, 0xc0, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 
  0x00, 0x03, 0xff, 0xfe, 0x3f, 0xff, 0xe0, 0x00, 0x00, 0x03, 0xff, 0xe0, 0x03, 0xff, 0xf0, 0x00, 
  0x00, 0x03, 0xff, 0x80, 0x01, 0xff, 0xf0, 0x00, 0x00, 0x03, 0xff, 0x00, 0x80, 0xff, 0xf0, 0x00, 
  0x00, 0x01, 0xfe, 0x07, 0xf0, 0x3f, 0xe0, 0x00, 0x00, 0x00, 0xf0, 0x0f, 0xfc, 0x1f, 0xc0, 0x00, 
  0x00, 0x00, 0x00, 0x1f, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xfe, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x3f, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x3f, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x3f, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xfe, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x1f, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xfc, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x07, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xe0, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char HABIBI [] PROGMEM = {
  // 'Logo-HG (2), 56x56px
  0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 
  0x7f, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x03, 0xfe, 0x00, 
  0x3f, 0xe0, 0x00, 0x00, 0x0f, 0xe0, 0x00, 0x07, 0xf0, 0x00, 0x00, 0x1f, 0x80, 0x1c, 0x01, 0xfc, 
  0x00, 0x00, 0x3e, 0x00, 0x18, 0xe0, 0x7e, 0x00, 0x00, 0x7c, 0x00, 0x08, 0x40, 0x1f, 0x00, 0x00, 
  0xf8, 0x60, 0x08, 0x43, 0x8f, 0x80, 0x01, 0xf0, 0xf0, 0x08, 0x43, 0x87, 0xc0, 0x03, 0xe0, 0x5c, 
  0x08, 0x41, 0x03, 0xc0, 0x07, 0xc4, 0x07, 0x08, 0x41, 0x01, 0xe0, 0x07, 0x8e, 0x03, 0x08, 0x41, 
  0x00, 0xf0, 0x0f, 0x0f, 0x83, 0x08, 0x41, 0x00, 0xf0, 0x0f, 0x00, 0xe3, 0x08, 0x47, 0x00, 0x78, 
  0x1e, 0x20, 0x23, 0x08, 0x7c, 0x06, 0x78, 0x1c, 0x70, 0x23, 0x08, 0xf0, 0x0e, 0x3c, 0x3c, 0x60, 
  0x23, 0x0b, 0x80, 0x3e, 0x3c, 0x3c, 0x20, 0x23, 0x0b, 0x00, 0xe0, 0x1c, 0x38, 0x20, 0x23, 0x0b, 
  0x03, 0x80, 0x1e, 0x78, 0x20, 0x23, 0x0b, 0x06, 0x00, 0x9e, 0x78, 0x20, 0x21, 0xcb, 0x04, 0x01, 
  0xce, 0x78, 0x30, 0x30, 0x7b, 0x04, 0x0f, 0x8e, 0x70, 0x1e, 0x1e, 0x1b, 0x1c, 0x3c, 0x0e, 0x71, 
  0x87, 0x87, 0x8b, 0x78, 0x20, 0x0f, 0x73, 0xc0, 0xe1, 0xfb, 0xc0, 0x20, 0x0f, 0x70, 0x78, 0x38, 
  0x3b, 0x00, 0x20, 0xcf, 0x70, 0x0e, 0x0f, 0x0b, 0x00, 0xe0, 0xcf, 0x70, 0x03, 0x83, 0xcb, 0x03, 
  0x80, 0x4f, 0x70, 0x00, 0x80, 0xcb, 0x1e, 0x00, 0x4f, 0x70, 0x00, 0x80, 0x4b, 0x78, 0x00, 0xce, 
  0x78, 0x00, 0x80, 0x4b, 0xe0, 0x03, 0x8e, 0x78, 0x00, 0x80, 0x4b, 0x00, 0x0e, 0x0e, 0x38, 0x00, 
  0xc0, 0x4b, 0x00, 0x78, 0x1e, 0x38, 0x00, 0x70, 0x4b, 0x01, 0xc0, 0x1e, 0x3c, 0x00, 0x1c, 0x4b, 
  0x0f, 0x00, 0x1c, 0x3c, 0x00, 0x07, 0xcb, 0x3c, 0x00, 0x3c, 0x1e, 0x00, 0x01, 0xcb, 0x70, 0x00, 
  0x3c, 0x1e, 0x00, 0x00, 0x4b, 0x40, 0x00, 0x78, 0x0f, 0x00, 0x00, 0x4b, 0x40, 0x00, 0x78, 0x0f, 
  0x00, 0x00, 0x4b, 0x40, 0x00, 0xf0, 0x07, 0x80, 0x00, 0x4b, 0x40, 0x01, 0xf0, 0x03, 0xc0, 0x00, 
  0x4b, 0x40, 0x01, 0xe0, 0x03, 0xe0, 0x00, 0x4b, 0x40, 0x03, 0xc0, 0x01, 0xf0, 0x00, 0x4b, 0x40, 
  0x0f, 0x80, 0x00, 0xf8, 0x00, 0x4b, 0x40, 0x1f, 0x80, 0x00, 0x7c, 0x00, 0x4b, 0x40, 0x3f, 0x00, 
  0x00, 0x3f, 0x00, 0x4b, 0x40, 0x7c, 0x00, 0x00, 0x1f, 0xc0, 0x4b, 0x41, 0xf8, 0x00, 0x00, 0x07, 
  0xf0, 0x4b, 0x6f, 0xf0, 0x00, 0x00, 0x03, 0xff, 0x6b, 0xff, 0xc0, 0x00, 0x00, 0x00, 0xff, 0xff, 
  0xff, 0x80, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xe0, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

//TFT_eSPI tft = TFT_eSPI();

uint8_t test=0,counter,crc_l,crc_h,address[20];
uint16_t crc;
int ec,ec2, ecCal, status;
double ppm;
double ph,temperature;
uint32_t i,j,data,timer1,timer2,timer3;
String message;

WiFiClient espClient;
PubSubClient client(espClient);

Adafruit_SH1106G lcd = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup()
{
  Serial.begin(115200);
  Serial.println("setrat");

  lcd.begin(i2c_Address, true); // Address 0x3C default
  
  modbus.begin(9600,SERIAL_8N1,RX,TX);

  init_Oled();

  connect_Wifi();
  
  pinMode(led, OUTPUT);
  pinMode(setWifi, INPUT);
  pinMode(RE,OUTPUT);
  digitalWrite(RE,LOW);
  digitalWrite(led, HIGH);
}

void loop()
{
  digitalWrite(led, HIGH);

  if (digitalRead(setWifi) == LOW) {
    //aaa
    if (millis()-timer3>=5000) espMode = WIFI_STATE_AP;
  } else timer3 = millis();

  switch (espMode) {
    case WIFI_STATE_STA:
    // if WiFi is down, try reconnecting every CHECK_WIFI_TIME seconds
    if ((WiFi.status() != WL_CONNECTED) && (millis() - timer2 >= CHECK_WIFI_TIME)) {
      Serial.println("Reconnecting to WiFi...");
      WiFi.disconnect();
      WiFi.reconnect();
      timer2 = millis();
    }
  
    if(millis()-timer1>=1000)
    {
      timer1=millis();
      digitalWrite(led, LOW);
      Modbus_request(0x01,0x03,0x00,0x02);
      delay(50);
      Modbus_request(0x02,0x03,0x00,0x02);
      lcdPrint();
      if (WiFi.status() == WL_CONNECTED) mqttPrint();
    }
    break;
    case WIFI_STATE_AP:
    Serial.println("Switch to AP");
    WiFi.softAP(Ap_ssid, Ap_pass);
    IPAddress IP = WiFi.softAPIP();
    Serial.println(IP);
    server.begin();
    unDone = true;
    lcd.clearDisplay();
    lcd.setTextSize(1);             // Normal 1:1 pixel scale
    lcd.setTextColor(SH110X_WHITE);        // Draw white text
    lcd.setCursor(0,0);             // Start at top-left corner
    lcd.println(IP);
    lcd.display();
    while(unDone){
     host(); 
    }
    server.stop();
    lcd.clearDisplay();
    lcd.setTextSize(1);             // Normal 1:1 pixel scale
    lcd.setTextColor(SH110X_WHITE);        // Draw white text
    lcd.setCursor(0,0);             // Start at top-left corner
    lcd.println("");
    lcd.println(ssid); 
    lcd.println(pass);
    lcd.display();
    delay(1000);
    espMode = WIFI_STATE_STA;
    break;
  }
}

void Modbus_request(unsigned char id,unsigned char code,unsigned int addr,unsigned int quantity)
{
  address[0]=id;
  address[1]=code;
  address[2]=addr>>8;
  address[3]=addr;
  address[4]=quantity>>8;
  address[5]=quantity;
  crc=CRC16(6);
  address[6]=crc&0xFF;
  address[7]=(crc>>8)&0xFF;

  #if defined (TROUBLESHOOT)
    for(i=0;i<8;i++)
    {
      if(address[i]<0x10) Serial.print("0");
      Serial.print(address[i],HEX);
      Serial.print(" ");
    }
    Serial.println();
  #endif

  digitalWrite(RE,HIGH);
  delay(10);
  for(i=0;i<8;i++) modbus.write(address[i]);
  delay(10);     
  digitalWrite(RE,LOW);

  if(modbus.available())
  {
    i=0;
    while(modbus.available()) address[i++]=modbus.read();

    while(address[0]==0)
    {
      for(j=0;j<i;j++) address[j]=address[j+1];
      i--;
    }

    #if defined (TROUBLESHOOT)
      if(i>1)
      {
        for(j=0;j<i;j++)
        {
          if(address[j]<0x10) Serial.print("0");
          Serial.print(address[j],HEX);
          Serial.print(" ");
        }
        Serial.println();  
      }
    #endif   

    if(i>2)
    {
      crc=CRC16(i-2);
      crc_l=crc&0xFF;
      crc_h=(crc>>8)&0xFF;      
    }
    
    if((address[i-2]==crc_l)&(address[i-1]==crc_h))
    {
      if (address[0] == 0x01) {
        ecCal=(256*address[3]+address[4])/10;
        ec=(256*address[5]+address[6])/10;
        ppm = ((double) ec / 1000) * 700;   //konversi ec -> ppm
        if (ppm > 0) ppm = (ppm * 1.6751) + 98.74;   //kalibrasi
//      Serial.println("Calibration Value = " + String(ecCal) +"us/cm");
        Serial.println("EC = " + String(ec) + " us/cm");
        Serial.println("ppm = "+ String(ppm) );
        Serial.println("");
      } else if(address[0] == 0x02) {
          temperature=0.1*(256*address[3]+address[4]);
          ph=(0.1*(256*address[5]+address[6])-0.4);
          Serial.println("Temperature = " + String(temperature) +" C , pH = " + String(ph));
        } else  Serial.println("Invalid address");
      delay (50);
    }
  }
}

void host() {
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // cek pesan dari header html
            if (header.indexOf("ssid=") > 1 && header.indexOf("pass=") > 7) {
              newSsid = header.substring((header.indexOf("ssid=") + 5), header.indexOf("&"));
              newPass = header.substring((header.indexOf("pass=") + 5), header.indexOf(" HTTP"));
              newValue = true;
            }
              
//              Serial.println(ssid);
//              Serial.println(pass);
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: black; padding: 8px 20px;");
            client.println("text-decoration: none; font-size: 20px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>Monitoring <br> Water Quality</h1><form>");
             
            client.println("<p>Ssid : </p>");
            client.println("<input type=\"text\" name=\"ssid\">");
               
            client.println("<p>Pass : </p>");
            client.println("<input type=\"text\" name=\"pass\">");

            client.println("<p><input type=\"submit\" value=\"Send\" class=\"submit-btn\"></p>");
            client.println("</form></body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
  if (newValue) {
    char buff_ssid[newSsid.length() + 1];
    char buff_pass[newPass.length() + 1];
    newSsid.toCharArray(buff_ssid, newSsid.length() + 1);
    const char* new_ssid = (char*)buff_ssid;
    newPass.toCharArray(buff_pass, (newPass.length() + 1));
    const char* new_pass = (char*)buff_pass;
    change_Wifi(new_ssid, new_pass);
//    Serial.println(ssid);
//    Serial.println(pass);
    newValue = false;
    unDone = false;
  }
  }

void lcdPrint() {
  lcd.clearDisplay();
  lcd.setTextSize(1);             // Normal 1:1 pixel scale
  lcd.setTextColor(SH110X_WHITE);        // Draw white text
  lcd.setCursor(0,0);             // Start at top-left corner

  if (WiFi.status() == WL_CONNECTED) 
  {
//    lcd.println(ssid + (String)WiFi.RSSI());
    lcd.println((String)ssid);
    signal_Cek(); 
  } else lcd.println("No Connection");
  
  lcd.println("");
  lcd.println("PPM  : " + (String)ppm + " PPM");
  lcd.println("");
  lcd.println("PH   : " + (String)ph);
  lcd.println("");
  lcd.println("SUHU : " + (String)temperature + " C");
  lcd.display();
}

void connect_Wifi() {
  int tries = 0;
  
  // attempt to connect to Wifi network:
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  if (wifiSet) {
    Serial.print("Connecting to " + (String)ssid);
    
    //oled display
    lcd.clearDisplay();
    lcd.setCursor(25, 0);
    lcd.println ("CONNECTING");
    lcd.print ("To : " + (String)ssid);
    lcd.display();
    
    while (WiFi.status() != WL_CONNECTED && tries <= 6) {
      Serial.print('.');
      tries++;
      delay(500);
    }
  
    if (WiFi.status() == WL_CONNECTED) {
      //oled display
      lcd.clearDisplay();
      lcd.setCursor(25, 0);
      lcd.print ("CONNECTED");
      lcd.display();
    
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
    
      //mqtt connect
      connect_Mqtt();
      client.loop();   
    } else {
      lcd.clearDisplay();
      lcd.setCursor(25, 0);
      lcd.print ("NO CONNECTION");
      lcd.display();
    
      Serial.println("No WiFi connection");
      delay(1000);
    }
  }
}

void connect_Mqtt() {
  int tries = 0;
  
  // set the server and callback data
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  
  // Try 5 times until we're connected
  while (!client.connected() && WiFi.status() == WL_CONNECTED && tries <= 4) {
    Serial.print((String)tries + " Attempting MQTT connection... ");
    // Attempt to connect
    if (client.connect(mqtt_clientid,mqtt_user,mqtt_pass)) {
      Serial.println("connected");
      // subscribe to event
      client.subscribe("change_Wifi");
      delay(200);
    }
    else 
    {
      Serial.print("failed, rc = ");
      Serial.print(client.state());
      tries++;
      delay(200);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String data = "";
  for(int i=0; i<length; i++) data+=(char)payload[i];
  Serial.println("Got: "+(String)topic+" value: "+data);

  if((String)topic=="change_Wifi")
  {
//    Serial.println("masuk");
    DeserializationError error = deserializeJson(doc_Recv, data);
    
    if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.f_str());
    }
    
    const char* new_ssid = doc_Recv["ssid"];
    const char* new_pass = doc_Recv["pass"];
    change_Wifi(new_ssid, new_pass);
  }
}

void change_Wifi(const char* new_ssid, const char* new_pass) {
  ssid = new_ssid;
  pass = new_pass;
  Serial.print("new ssid : ");
  Serial.println(ssid);
  Serial.print("new pass : ");
  Serial.println(pass);
  wifiSet = true;
//  WiFi.begin(ssid, pass);
  connect_Wifi();
  Serial.println("Switch Wifi to " + (String)ssid);
}

void mqttPrint() {
  if(!client.connected())
  {
    Serial.println("Mqtt not connected!");
    connect_Mqtt();
  }

  doc_Send["PPM"] = ppm;
  doc_Send["PH"] = ph;
  doc_Send["Temperature"] = temperature;

  // Produce a minified JSON document
  serializeJson(doc_Send, mqtt_Msg);
  Serial.println(mqtt_Msg);
  
  client.loop();
  client.publish("Water_Quality",mqtt_Msg);
  client.loop();
}

void init_Oled() {
  lcd.clearDisplay();
  lcd.setTextSize(1);
  lcd.setTextColor(SH110X_WHITE);
  lcd.setCursor(55, 0);
  lcd.print ("PENS");
  lcd.drawBitmap(32, 8, PENS, 64, 64, SH110X_WHITE);
  lcd.display();
  delay(1500);
  lcd.clearDisplay();
  lcd.setTextColor(SH110X_WHITE);
  lcd.setCursor(30, 0);
  lcd.print ("HABIBI GARDEN");
  lcd.drawBitmap(32, 8, HABIBI, 56, 56, SH110X_WHITE);
  lcd.display();
  delay(1500);
}

void signal_Cek() {
  int lvl;    //signal level (0 - 4) 0 -> bad
  //drawing purpose
  int steps = 2;
  int x, y;

  switch (WiFi.RSSI()) {
  case -20 ... 1:
    lvl = 4;
    break;
  case -40 ... -21:
    lvl = 3;
    break;
  case -60 ... -41:
    lvl = 2;
    break;
  case -80 ... -61:
    lvl = 1;
    break;
  default:
    lvl = 0;
    lcd.setCursor(120, 0);
    lcd.println("x");
    break;
  }

//  Serial.println((String)lvl);

  for (int i = 1; i <= lvl; i++) {
    y = 7;
    for (int j = 0; j < steps; j++) {
      x = 113 + i * 3;
      for (int k = 0; k < 2; k++) {
        lcd.drawPixel(x, y, SH110X_WHITE);
        x++; 
      }
      y--;
    }
    steps += 2;
  }
}

int CRC16(int DataLength)
{
  unsigned int i,j,CheckSum;
  CheckSum=0xFFFF;
  for (j=0;j<DataLength;j++){
    CheckSum=CheckSum^address[j];
    for(i=0;i<8;i++){
      if((CheckSum)&0x0001==1) CheckSum=(CheckSum>>1)^0xA001;
      else CheckSum=CheckSum>>1;}}
  return CheckSum;   
}
