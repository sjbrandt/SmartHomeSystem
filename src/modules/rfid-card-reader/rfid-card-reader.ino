#include <SPI.h>
#include <MFRC522.h>

// --- Pin configuration ---
#define SS_PIN    5
#define RST_PIN   3
#define GREEN_LED 14
#define RED_LED   12
#define BLUE_LED  27
#define LEARN_BTN 26   // Button to update valid UID

MFRC522 rfid(SS_PIN, RST_PIN);

// Current valid card UID
String validUID = "";

// Lock state
bool isLocked = true;

// When true, next card scanned becomes the new validUID
bool learnMode = false;

// Helper: update state LEDs based on isLocked
void updateStateLeds() {
  if (isLocked) {
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
  } else {
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(LEARN_BTN, INPUT_PULLUP); // Button active LOW

  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(BLUE_LED, LOW);

  SPI.begin();
  rfid.PCD_Init();

  updateStateLeds();

  Serial.println("RFID system ready...");
  if (validUID == "") {
    Serial.println("Add card");
    learnMode = true;
    delay(300); // Prevent multiple triggers
  }
}

void loop() {
  digitalWrite(BLUE_LED, LOW);

  // --- Check learn button ---
  if (digitalRead(LEARN_BTN) == LOW) {
    learnMode = true;
    Serial.println("Learn mode activated. Scan a card to set new valid UID...");
    delay(300); // Prevent multiple triggers
  }

  // --- Check for card ---
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    updateStateLeds();
    return;
  }

  // Build UID string
  String readUID = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    char buf[3];
    snprintf(buf, sizeof(buf), "%02x", rfid.uid.uidByte[i]);
    readUID += buf;
  }

  Serial.print("Card detected with UID: ");
  Serial.println(readUID);

  // --- If in learn mode, update validUID ---
  if (learnMode) {
    validUID = readUID;
    learnMode = false;

    Serial.print("New valid UID stored: ");
    Serial.println(validUID);

    // Visual confirmation: blink green LED
    for (int i = 0; i < 3; i++) {
      digitalWrite(GREEN_LED, HIGH);
      delay(150);
      digitalWrite(GREEN_LED, LOW);
      delay(150);
    }

    updateStateLeds();
  }
  // --- Normal operation ---
  else {
    if (readUID.equalsIgnoreCase(validUID)) {
      // Valid card toggles lock state
      isLocked = !isLocked;
      updateStateLeds();

      if (isLocked) {
        Serial.println("State changed: LOCKED");
      } else {
        Serial.println("State changed: UNLOCKED");
      }

    } else {
      // Invalid card
      Serial.println("Access denied");
      digitalWrite(BLUE_LED, HIGH);
      updateStateLeds();
    }
  }

  // Cleanup RFID
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  delay(250);
}
