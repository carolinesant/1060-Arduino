
#include <SPI.h>            //"Importerer" SPI og MFRC522 bibloteker
#include <MFRC522.h>

#define RST_PIN 9           
#define SDA_PIN 10          

MFRC522 rfid(SDA_PIN, RST_PIN);   // Lager en instans av MFRC522 (som er RFID scanneren)

//variablene tilstand0 og tilstand1 skal brukes for å skrive over informasjon i blokken til chippen som blir scannet
byte tilstand0[] =   {0x30};
byte tilstand1[] =   {0x31};

void setup() {
  Serial.begin(9600);        // Begynner kommunikasjon med serial
  SPI.begin();               // Begynner SPI bus
  rfid.PCD_Init();        // Begynner MFRC522 kort
  Serial.println(F("Setter opp kort ved å skrive aktiv-status til den og skrive ut UID info"));
}

void loop() {


  //Sjekker om det finnes noe som kan scannes (hvis ja prøver den å lese det) ellers avsluttes loopen
  if ( ! rfid.PICC_IsNewCardPresent() || ! rfid.PICC_ReadCardSerial()) {
    return;
  }

//Skal bruke StatusCode for å sjekke om lesing/skriving til rfid chippen er godkjent
  MFRC522::StatusCode status;

//Sjekker om PICC typen til den leste chippen er MIFARE_1K, da er det enten brikken eller kortet som har blitt scannet
//For chipper med MIFARE_1K må man autentisere før man prøver å lese informasjon eller skrive informasjon
  if (rfid.PICC_GetType(rfid.uid.sak) == MFRC522::PICC_TYPE_MIFARE_1K) {
       
       MFRC522::MIFARE_Key key;
       for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

      //Autentiserer at nøkkelen funker for blokk 8 
      status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 8, &key, &(rfid.uid));
      if (status != MFRC522::STATUS_OK) {
           Serial.print(" -Autentisering godkjent ");
           Serial.println(rfid.GetStatusCodeName(status));
           rfid.PICC_HaltA(); // Halt PICC
           rfid.PCD_StopCrypto1();  // Stop encryption on PCD
           return;
      }
      else Serial.print((" -Autentisering godkjent "));
  } else Serial.print("Ingen autentisering trengs");


  //Skriver over verdien '0' til blokk 8
  if (rfid.PICC_GetType(rfid.uid.sak) == MFRC522::PICC_TYPE_MIFARE_1K) status = rfid.MIFARE_Write(8, tilstand0, 16); //For brikke/kort
  else status = rfid.MIFARE_Ultralight_Write(8, tilstand0, 4); //For Ultralight chip
      
      if (status != MFRC522::STATUS_OK) {
          Serial.println("Blokk8 skriving feilet");
          rfid.PICC_HaltA(); // Halt PICC
          rfid.PCD_StopCrypto1();  // Stop encryption on PCD
          return;
      } else Serial.println("Blokk8 skriving velykket");  


   //Skriver over verdien '0' til blokk 9
  if (rfid.PICC_GetType(rfid.uid.sak) == MFRC522::PICC_TYPE_MIFARE_1K) status = rfid.MIFARE_Write(9, tilstand0, 16); //For brikke/kort
  else status = rfid.MIFARE_Ultralight_Write(9, tilstand0, 4); //For Ultralight chip
      
      if (status != MFRC522::STATUS_OK) {
          Serial.println("Blokk9 skriving feilet");
          rfid.PICC_HaltA(); // Halt PICC
          rfid.PCD_StopCrypto1();  // Stop encryption on PCD
          return;
      } else Serial.println("Blokk9 skriving velykket"); 


  Serial.println(" ");
  rfid.PICC_HaltA(); // Halt PICC
  rfid.PCD_StopCrypto1();  // Stop encryption on PCD

}
