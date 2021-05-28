
#include <SPI.h>            //"Importerer" SPI og MFRC522 bibloteker
#include <MFRC522.h>

#define RST_PIN 9           
#define SDA_PIN 10          

MFRC522 rfid(SDA_PIN, RST_PIN);   // Lager en instans av MFRC522 (som er RFID scanneren)

void setup() {
  Serial.begin(9600);        // Begynner kommunikasjon med serial
  SPI.begin();               // Begynner SPI bus
  rfid.PCD_Init();        // Begynner MFRC522 kort
  Serial.println(F("Setter opp kort ved å skrive aktiv-status til den og skrive ut UID info"));
}

void loop() {

  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  //Starter loopen på nytt dersom det ikke er et nytt kort som vises
  if ( ! rfid.PICC_IsNewCardPresent()) {
    return;
  }

  //Prøver å lese kort fra scanner, dersom det ikke er et kort starter loopen igjen
  if ( ! rfid.PICC_ReadCardSerial()) {
    return;
  }

  //Printer ut UID informasjonen til kortet
  Serial.print(F("Kortet sin UID:"));
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i], HEX);
  }
  Serial.println();


  MFRC522::StatusCode status;
  byte block = 4;
  

  byte tilstand0[] = {0x30,0x20,0x20,0x20,
                    0x20,0x20,0x20,0x20,
                    0x20,0x20,0x20,0x20,
                    0x20,0x20,0x20,0x20};


  //Sjekker/autentiserer at det finnes en "tom" nøkkel inni block 4, der vi skal skrive over informasjon
  //Gjør dette for å være sikker på at det finnes plass der vi skal skrive over informasjon
  
  status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(rfid.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.println(F("Autentisering ikke godkjent "));
    Serial.println(rfid.GetStatusCodeName(status));
    return;
  }
  else Serial.println(F("Autentisering godkjent "));


  // Prøver å skrive over tilstand0 sin informasjon i blokk 4, og skriver ut om det var velykket eller ikke
  status = rfid.MIFARE_Write(block, tilstand0, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Informasjonsskriving feilet "));
    Serial.println(rfid.GetStatusCodeName(status));
    return;
  }
  else Serial.println(F("Informasjonsskriving velykket! "));

  rfid.MIFARE_Write(5, tilstand0, 16);

  Serial.println(" ");
  rfid.PICC_HaltA(); // Halt PICC
  rfid.PCD_StopCrypto1();  // Stop encryption on PCD

}
