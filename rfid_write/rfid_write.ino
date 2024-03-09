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
  Serial.println(F("Writing to the NFC TAG..."));
}

void loop() {
  MFRC522::MIFARE_Key key;
  for(byte i = 0; i < 6;i++) key.keyByte[i] = 0xff;

  if(! rfid.PICC_IsNewCardPresent()){return;}
  if(! rfid.PICC_ReadCardSerial()){return;}

  Serial.println(F("card uid:"));
  for(byte i = 0; i < rfid.uid.size;i++){
    Serial.print(rfid.uid.uidByte[i] < 0x10 ? "0" : " ");
    Serial.print(rfid.uid.uidByte[i],HEX);
  }
  
  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type picctype = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(picctype));

  byte buffer[20];
  byte block;
  MFRC522::StatusCode status;
  byte len;

  Serial.setTimeout(20000L);
  len = Serial.readBytesUntil('#',(char*)buffer,16);
  for(byte i = len; i < 16;i++) buffer[i] = ' ';

  block = 4;
  status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, block, &key, &(rfid.uid));
  if(status != MFRC522::STATUS_OK){
    Serial.print(F("PICC authentication failed with key B."));
    Serial.println(rfid.GetStatusCodeName(status));
    return;
  }
  else Serial.println("reader authentication success with key B.");

  status = rfid.MIFARE_Write(block, buffer, 16);
  if(status != MFRC522::STATUS_OK){
    Serial.print(F("Write Operation failed."));
    Serial.print(rfid.GetStatusCodeName(status));
    return;
  }
  else Serial.println("writing authentication successful to the nfc tag at block 4.");

  rfid.PICC_DumpToSerial(&(rfid.uid));

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}
