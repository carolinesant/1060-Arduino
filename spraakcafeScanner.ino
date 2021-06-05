/*
SprakcafeScanner leser informasjon fra rfid chippen (bladet) i blokk 8, og "aktiverer" bladet ved å bytte verdien fra '0' til '1'.
Denne informasjonen brukes av progresjonsTreet for å se om bladet er aktivert eller ikke. (Om den har blitt scannet på språkkafeen eller ikke)

I tillegg sjekkes det om det finnes verdien '1' i blokk9 på bladet, dette signaliserer at bruker har gjennomført en 
hel iterasjon av progresjonsTreet sitt og skal få en premie. Hvis verdien er 1, skrives den over til 0 (for å null-stille) og
premie metoden kalles. 

Til slutt kalles lysRandom metoden og et tilfeldig lys lyser opp for å signaliserer hvilket bord brukeren skal sette seg på,
ved at lysene har ulike farger og man skal sette seg på bordet som matcher fargen.
*/
//--------------------------------------------------------------------------------------------------------------------------------

//Importerer SPI og MFRC522 bibloteket
#include <SPI.h>
#include <MFRC522.h>

//Definerer de to pin-ene fra rfid/mfrc55-en som trengs for å lage en instans
#define RST_PIN 9 //RST er på pin 9
#define SDA_PIN 10 //SDA er på pin 10

//Lager en instans av rfid
MFRC522 rfid(SDA_PIN, RST_PIN);

//Definerer lysenes porter
int yellowLED = 2;
int redLED = 3;
int greenLED = 4;
int blueLED = 5;

//Definerer piezoen sin port
int piezo = 6; 

int randomTall; //Variabel randomTall skal brukes senere for å random velge en av lysene

//variablene tilstand0 og tilstand1 skal brukes for å skrive over informasjon i blokken til chippen som blir scannet

byte tilstand0[] =   {0x30};
byte tilstand1[] =   {0x31};


void setup() {

//Definerer pinMode for lysene
  pinMode(blueLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  
//Begynner SPI bus og initaliserer rfid scanneren instans
  SPI.begin();
  rfid.PCD_Init();
}

void loop() {
    
//Sjekker om det finnes noe som kan scannes (hvis ja prøver den å lese det) ellers avsluttes loopen
  if ( ! rfid.PICC_IsNewCardPresent() || ! rfid.PICC_ReadCardSerial()) {
    return;
  }
//Skal bruke StatusCode for å sjekke om lesing/skriving til rfid chippen er godkjent
  MFRC522::StatusCode status;

//Lager en byte-array som informasjonen som leses skal skrives til, og en lengde på antall byte som skal leses og 
//hvilken blokk informasjonen skal leses fra
  byte aktiveringsVerdi[18];
  byte lengde = 18;

//Sjekker om PICC typen til den leste chippen er MIFARE_1K, da er det enten brikken eller kortet som har blitt scannet
//For chipper med MIFARE_1K må man autentisere før man prøver å lese informasjon eller skrive informasjon
  if (rfid.PICC_GetType(rfid.uid.sak) == MFRC522::PICC_TYPE_MIFARE_1K) {
       
       MFRC522::MIFARE_Key key;
       for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

      //Autentiserer at nøkkelen funker for blokk 8 
      status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 8, &key, &(rfid.uid));
      if (status != MFRC522::STATUS_OK) {
           rfid.PICC_HaltA(); // Halt PICC
           rfid.PCD_StopCrypto1();  // Stop encryption on PCD
           return;
      }
  }
  
//Leser av informasjonen lagret i blokk 8 på chippen, som er bladets aktiverings informasjon.
  status = rfid.MIFARE_Read(8, aktiveringsVerdi, &lengde);
  if (status != MFRC522::STATUS_OK) { //Hvis den ikke klarer å lese av informasjonen stopper loopen
    rfid.PICC_HaltA(); // Halt PICC
    rfid.PCD_StopCrypto1();  // Stop encryption on PCD
    return;
  }


 
//Sjekker om aktiverings verdien er 0, dersom prøves det å skriv over til tilstand1 (aktiverings verdien blir '1'), hvis det ikke går brytes loopen
  if (char(aktiveringsVerdi[0] == '0')){
      
      //Her sjekkes det om chippen er av typen MIFARE_1K (brikken eller kortet) eller ikke, siden den og Ultralight chippen bruker ulike Write metoder.
      if (rfid.PICC_GetType(rfid.uid.sak) == MFRC522::PICC_TYPE_MIFARE_1K) status = rfid.MIFARE_Write(8, tilstand1, 16); //For brikke/kort
      else status = rfid.MIFARE_Ultralight_Write(8, tilstand1, 4); //For Ultralight chip
      
      if (status != MFRC522::STATUS_OK) {
          rfid.PICC_HaltA(); // Halt PICC
          rfid.PCD_StopCrypto1();  // Stop encryption on PCD
          return;
      }
      
  } else { //Hvis verdien ikke er '0', har ikke chippen riktig informasjon og loopen brytes
    rfid.PICC_HaltA(); // Halt PICC
    rfid.PCD_StopCrypto1();  // Stop encryption on PCD
    return;
  }

//Lager en ny byte-array for å ta vare på det som skal leses av i blokk 9
  byte vunnetVerdi[18];

//Leser verdien til blokk 9 og lagrer det i vunnetVerdi variabelen
  status = rfid.MIFARE_Read(9, vunnetVerdi, &lengde);
  if (status != MFRC522::STATUS_OK) {
    rfid.PICC_HaltA(); // Halt PICC
    rfid.PCD_StopCrypto1();  // Stop encryption on PCD
    return;
  }

//Sjekker om den leste verdien er 1, dermed skrives det over 0 på denne plassen og premie() metoden kalles
  if (char(vunnetVerdi[0]) == '1') {
      //Her sjekkes det igjen om PICC typen er MIFARE 1K eller ikke for å kalle riktig Write metode
      if (rfid.PICC_GetType(rfid.uid.sak) == MFRC522::PICC_TYPE_MIFARE_1K) status = rfid.MIFARE_Write(9, tilstand0, 16); //For brikke/kort
      else status = rfid.MIFARE_Ultralight_Write(9, tilstand0, 4); //For Ultralight chip
      
      if (status != MFRC522::STATUS_OK) {
          rfid.PICC_HaltA(); // Halt PICC
          rfid.PCD_StopCrypto1();  // Stop encryption on PCD
          return;
      }
      premie();
  }

//Så kalles lysRandom metoden som lyser et random lys for å vise hvilket bord man skal sitte på
  lysRandom();

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

//Lyser opp et random lys og spiller av en tone for å vise bruker bordet de skal sette seg på
void lysRandom() {
  randomTall = random(2,6); //Velger enten 2,3,4 eller 5 random
  digitalWrite(randomTall, HIGH); //Sender strøm til den porten
  tone(piezo, 262, 500);  //Spiller en tone
  delay(4000); //Har en forsinkelse så bruker har tid til å se fargen
  digitalWrite(randomTall, LOW); //Skrur lyset av
}

//Spiller en melodi og får lysene til å danse, for å signalisere at bruker har vunnet
void premie() {

  //Definerer int arrayer der en holder tonene som skal spilles, en hvor lenge de skal spilles og sku-av-på rekkefølgen til lysene 
  int melodi[] = {262, 196,196, 220, 196,0, 247, 262};
  int toneLengder[] = {250, 125, 125, 250, 250, 250, 250, 250};
  int lysRekke[] = {blueLED, greenLED, redLED, yellowLED, blueLED, greenLED, redLED, yellowLED};

  //Spiller melodien og lyser opp lysene ved å lage en loop 
   for (int gang = 0; gang < 4; gang++) {
      
      //For loopen itererer gjennom arrayene for å spille av hver tone og skrur av og på hvert lys i riktig rekkefølge
      for (int i = 0; i < 8; i++) {
        tone(piezo, melodi[i],toneLengder[i]);
        digitalWrite(lysRekke[i], HIGH);
        
        int pauseMellomToner = toneLengder[i] * 1.30;
        delay(pauseMellomToner);
        noTone(piezo);
        digitalWrite(lysRekke[i], LOW);
      }
   }
}
