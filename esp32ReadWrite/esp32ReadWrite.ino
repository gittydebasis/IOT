#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 0                         /* GPIO 0 of esp32 selected as reset pin */
#define SS_PIN 5                          /* GPIO 5 of esp32 selected as slave select pin */

MFRC522 rfid(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(9600);                                                       /* Begin the Serial in 9600 baud rate */
  while(! Serial){};                                                        /* if Serial will not start then do nothing */
  SPI.begin();                                                              /* begin the SPI communication */
  rfid.PCD_Init();                                                          /* initiate the rfid */
  Serial.println(F("performing Read and Write operation."));
}

void loop() {
  /* select whether you want to read and write */
  Serial.println(F("what do you want? (Select the number)"));
  Serial.println(F("1. Read or 2. Write"));
  while(Serial.available()==0){};
  int select = Serial.parseInt();
  if(select == 1){
    Serial.println(F("reading data from the tag."));
    data_read();
  }
  else if(select == 2){
    Serial.println(F("writing data to the tag."));
    data_write();
  }
  else{
    Serial.println(F("No option was selected."));
  }

}

void data_read(){
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
  while(1){};
}

void data_write(){
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

  Serial.setTimeout(50000L);
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
  else Serial.println("writing authentication successful to the nfc tag at block 1.");
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  while(1){};
}
