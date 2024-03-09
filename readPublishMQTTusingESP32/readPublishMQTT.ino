#include <WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <MFRC522.h>

const char* ssid = "Jsnr1999-2.4G";
const char* pass = "9963066008";

const char* ID = "ESP32Publish";      //name of the device
const char* TOPIC = "mfrc522/tagid";  //Topic to subscribe

const char* broker = "192.168.1.23";
WiFiClient espclient;
PubSubClient client(espclient);

#define RST_PIN 0
#define SS_PIN 5
MFRC522 rfid(SS_PIN, RST_PIN);

byte buffer[18];
char data[18];
byte block;
byte len;
MFRC522::StatusCode status;
MFRC522::MIFARE_Key key;

void wifi_setup(){
  Serial.print(F("Connecting to "));Serial.println(ssid);
  WiFi.begin(ssid,pass);
  while(WiFi.status() != WL_CONNECTED){delay(1000);Serial.print(F("."));}
  Serial.println();
  Serial.print(F("wifi connected to "));Serial.println(ssid);
  Serial.print(F("Connected to IP address "));Serial.println(WiFi.localIP());
}

void mqtt_reconnect(){
  while(!client.connected()){
    Serial.println(F("Trying to connect to MQTT again..."));
    if(client.connect(ID)){
      Serial.println(F("connected"));
      Serial.print(F("publishing to "));Serial.println(TOPIC);
      delay(500);
    }
  }
}

void setup(){
  Serial.begin(115200);
  while(! Serial){};
  wifi_setup();
  client.setServer(broker, 1883);
  SPI.begin();
  rfid.PCD_Init();
  Serial.println(F("reading from the NFC TAG..."));
}

void loop(){
  for(byte i = 0; i < 6;i++) key.keyByte[i] = 0xff;

  if(! rfid.PICC_IsNewCardPresent()){return;}
  if(! rfid.PICC_ReadCardSerial()){return;}
  Serial.println(F("Card Detected..."));
  rfid.PICC_DumpDetailsToSerial(&(rfid.uid));

  Serial.println(F("**Card Detected:**"));
  Serial.print(F("ID is: "));

  block = 4;
  len = 18;
  status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, block, &key, &(rfid.uid));
  if(status != MFRC522::STATUS_OK){
    Serial.print(F("authentication successful by key B"));
    Serial.println(rfid.GetStatusCodeName(status));
    return;
  }

  status = rfid.MIFARE_Read(block, buffer, &len);
  if(status != MFRC522::STATUS_OK){
    Serial.print(F("reading failed."));
    Serial.println(rfid.GetStatusCodeName(status));
    return;
  }

  for(uint8_t i = 0; i < 16;i++){
    if(buffer[i] != 16)
    {
      Serial.write(buffer[i]);
      data[i] = buffer[i];
    }
  }
  Serial.println();

  Serial.println(F("publishing the tag data...")); 
  if(!client.connected()){mqtt_reconnect();}
  client.loop();
  client.publish(TOPIC, data);
  delay(1000);
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}