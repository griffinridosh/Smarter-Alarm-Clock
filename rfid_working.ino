#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 2   // Define the slave select pin
#define RST_PIN 4 // Define the reset pin

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create an MFRC522 instance

void setup() {
  Serial.begin(9600); // Initialize serial communication
  SPI.begin(); // Initialize SPI bus
  mfrc522.PCD_Init(); // Initialize MFRC522 module
}

void loop() {
  // Check if a card is present
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    // If a card is present, read its UID (unique identifier)
    String cardID = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      cardID += String(mfrc522.uid.uidByte[i], HEX);
    }

    // Print the card ID to the serial monitor
    Serial.println("Card detected: " + cardID);
  }

  // Delay for a short time before checking again
  delay(500);
}