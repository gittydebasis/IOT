#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 0
#define SS_PIN 5
MFRC522 rfid(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(9600);
  while(! Serial){};
  SPI.begin();
  rfid.PCD_Init();
  Serial.println(F("reading from the NFC TAG..."));
}

void loop() {
  MFRC522::MIFARE_Key key;
  for(byte i = 0; i < 6;i++) key.keyByte[i] = 0xff;

  byte block;
  byte len;
  MFRC522::StatusCode status;

  if(! rfid.PICC_IsNewCardPresent()){return;}
  if(! rfid.PICC_ReadCardSerial()){return;}
  Serial.println(F("Card Detected..."));
  rfid.PICC_DumpDetailsToSerial(&(rfid.uid));

  Serial.println(F("**Card Detected:**"));
  Serial.print(F("ID is: "));
  
  byte buffer[18];
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
    }
  }
  Serial.print(" ");
  delay(1000);
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  
}
