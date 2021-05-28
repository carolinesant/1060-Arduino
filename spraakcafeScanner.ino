/*
SprakcafeScanner leser informasjon fra rfid chippen (bladet) , og "aktiverer" bladet ved å bytte verdien fra '0' til '1'.
Denne informasjonen brukes av progresjonsTreet for å se om bladet er aktivert eller ikke. (Om den har blitt scannet på språkkafeen eller ikke)

I tillegg sjekkes det om det finnes verdien '1' i blokk5 på bladet, dette signaliserer at bruker har gjennomført en 
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

byte tilstand0[] =   {0x30,0x20,0x20,0x20,
                      0x20,0x20,0x20,0x20,
                      0x20,0x20,0x20,0x20,
                      0x20,0x20,0x20,0x20};

byte tilstand1[] =   {0x31,0x20,0x20,0x20,
                      0x20,0x20,0x20,0x20,
                      0x20,0x20,0x20,0x20,
                      0x20,0x20,0x20,0x20};


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
    
//Sjekker om det finnes noe som kan scannes ellers avsluttes loopen
  if ( ! rfid.PICC_IsNewCardPresent()) {
    return;
  }

//Prøver å lese kortet som er funnet, hvis ikke så avsluttes loopen
  if ( ! rfid.PICC_ReadCardSerial()) {
    return;
  }

//Skal bruke StatusCode for å sjekke om lesing/skriving til rfid chippen er godkjent
  MFRC522::StatusCode status;

//Lager en byte-array som informasjonen som leses skal skrives til, og en lengde på antall byte som skal leses og 
//hvilken blokk informasjonen skal leses fra
  byte aktiveringsVerdi[18];
  byte lengde = 18;
  byte blokk = 4;

//Leser av informasjonen lagret i blokk 4 på chippen, som er bladets aktiverings informasjon
  status = rfid.MIFARE_Read(blokk, aktiveringsVerdi, &lengde);
  if (status != MFRC522::STATUS_OK) {
    rfid.PICC_HaltA(); // Halt PICC
    rfid.PCD_StopCrypto1();  // Stop encryption on PCD
    return;
  }

  
//Sjekker om aktiverings verdien er 0, dersom prøves det å skriv over til tilstand1 (aktiverings verdien blir '1'), hvis det ikke går brytes loopen
  if (char(aktiveringsVerdi[0]) == '0') {
      status = rfid.MIFARE_Write(blokk, tilstand1, 16);
      if (status != MFRC522::STATUS_OK) {
        rfid.PICC_HaltA(); // Halt PICC
        rfid.PCD_StopCrypto1();  // Stop encryption on PCD
        return;
      }

//Hvis verdien ikke er '0', har ikke chippen riktig informasjon og loopen brytes
  } else {
    rfid.PICC_HaltA(); // Halt PICC
    rfid.PCD_StopCrypto1();  // Stop encryption on PCD
    return;
  }

//Forandrer verdien til blokk, og lager en ny byte-array for å ta vare på det som skal leses av 
  blokk = 5;
  byte vunnetVerdi[18];


//Leser verdien til blokk 5 og lagrer det i andreLestVerdi variabelen
  status = rfid.MIFARE_Read(blokk, vunnetVerdi, &lengde);
  if (status != MFRC522::STATUS_OK) {
    rfid.PICC_HaltA(); // Halt PICC
    rfid.PCD_StopCrypto1();  // Stop encryption on PCD
    return;
  }


//Sjekker om den leste verdien er 1, dermed skrives det over 0 på denne plassen og premie() metoden kalles
  if (char(vunnetVerdi[0]) == '1') {
      status = rfid.MIFARE_Write(blokk, tilstand0, 16);
      if (status != MFRC522::STATUS_OK) {
          rfid.PICC_HaltA(); // Halt PICC
          rfid.PCD_StopCrypto1();  // Stop encryption on PCD
          return;
      }
      premie();
  }
//Så kalles lysRandom metoden som lyser et random lys
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
