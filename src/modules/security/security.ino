
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Wire.h>

// -------------------- User Config --------------------
#define SS_PIN 5
#define RST_PIN 3

#define MAX_NUMBER_TRIES 3
#define LOCKED_OUT_SECONDS 10

#define MIN_PIN_LEN 4
#define MAX_PIN_LEN 8

#define READ_milliSECONDS 1000

// -------------------- Globals ------------------------
int triesLeft = MAX_NUMBER_TRIES;

LiquidCrystal_I2C lcd(0x27, 16, 2);

const byte numRows = 4;
const byte numCols = 4;

char keymap[numRows][numCols] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

String passcode = "0000";  // default PIN
String inputValue = "";
String validUID = "4f41e11e";  // default allowed card UID (lowercase hex string)
String readyText = "Ready, scan card"; // MAX 16 characters

bool isLocked = true;

byte rowPins[numRows] = { 13, 12, 14, 27 };
byte colPins[numCols] = { 26, 25, 33, 32 };

Keypad myKeypad = Keypad(makeKeymap(keymap), rowPins, colPins, numRows, numCols);
MFRC522 rfid(SS_PIN, RST_PIN);

// -------------------- Utility / Helpers --------------
bool isDigitKey(char k) {
  return (k >= '0' && k <= '9');
}

char waitForAnyKey() {
  char k = NO_KEY;
  while (k == NO_KEY) {
    k = myKeypad.getKey();
  }
  return k;
}

void lcdLineClear(uint8_t row) {
  lcd.setCursor(0, row);
  lcd.print("                ");  // 16 spaces
  lcd.setCursor(0, row);
}

bool compareCodes(const String &code1, const String &code2) {
  return (code1 == code2);
}

// -------------------- PIN Entry Functions ------------
// Blocking PIN capture (numeric only) with prompt.
// Masked output; '#' to submit with MIN length enforced
String GetCodeWithPrompt(const String &prompt) {
  String entered = "";
  lcd.clear();
  lcd.print(prompt);
  lcd.setCursor(0, 1);

  while (true) {
    char k = myKeypad.getKey();
    if (k == '#') {
      if (entered.length() >= MIN_PIN_LEN) {
        return entered;
      } else {
        // Too short
        lcdLineClear(1);
        lcd.print("Too short, min 4");
        delay(READ_milliSECONDS);
        lcd.clear();
        lcd.print(prompt);
        lcd.setCursor(0, 1);
        for (uint8_t i = 0; i < entered.length(); i++) lcd.print('*');
      }
    } else if (k == 'C') {
      // clear buffer
      entered = "";
      lcd.clear();
      lcd.print(prompt);
      lcd.setCursor(0, 1);
    } else if (isDigitKey(k)) {
      if (entered.length() < MAX_PIN_LEN) {
        entered += k;
        lcd.print('*');
      }
    }
  }
}

// Blocking PIN capture with the first key already provided as a digit.
// Masked output; '#' to submit with MIN length enforced.
String GetCodePrefill(char firstDigit) {
  String entered = "";
  if (isDigitKey(firstDigit)) {
    entered += firstDigit;
  }
  lcd.clear();
  lcd.print("Enter code:");
  lcd.setCursor(0, 1);
  for (uint8_t i = 0; i < entered.length(); i++) lcd.print('*');

  while (true) {
    char k = myKeypad.getKey();
    if (k == '#') {
      if (entered.length() >= MIN_PIN_LEN) {
        return entered;
      } else {
        // Too short
        lcdLineClear(1);
        lcd.print("Too short, min 4");
        delay(READ_milliSECONDS);
        lcd.clear();
        lcd.print("Enter code");
        lcd.setCursor(0, 1);
        for (uint8_t i = 0; i < entered.length(); i++) lcd.print('*');
      }
    } else if (k == 'C') {
      entered = "";
      lcd.clear();
      lcd.print("Enter code:");
      lcd.setCursor(0, 1);
    } else if (isDigitKey(k)) {
      if (entered.length() < MAX_PIN_LEN) {
        entered += k;
        lcd.print('*');
      }
    }
  }
}

// Convenience wrapper matching your earlier usage
String GetCode() {
  return GetCodeWithPrompt("Enter code:");
}

// -------------------- Admin Flows --------------------
// Change the stored passcode after verifying the old one
void changePasscodeFlow() {
  // Verify old PIN
  String oldPin = GetCodeWithPrompt("Old PIN:");
  if (oldPin != passcode) {
    lcd.clear();
    lcd.print("Wrong PIN");
    delay(READ_milliSECONDS);
    return;
  }
  // New PIN
  String new1 = GetCodeWithPrompt("New PIN:");
  // Confirm
  String new2 = GetCodeWithPrompt("Confirm:");
  if (new1 != new2) {
    lcd.clear();
    lcd.print("Mismatch");
    delay(READ_milliSECONDS);
    return;
  }
  passcode = new1;
  lcd.clear();
  lcd.print("PIN Updated");
  delay(READ_milliSECONDS);
}

// Wait for and store a new RFID UID as the valid card
void changeCardFlow() {
  // Verify PIN
  String pin = GetCodeWithPrompt("Enter code:");
  if (pin != passcode) {
    lcd.clear();
    lcd.print("Wrong PIN");
    delay(READ_milliSECONDS);
    return;
  }
  lcd.clear();
  lcd.print("Scan new card");

  // Stop any current comms
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  // Wait for a new card and read UID
  while (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    // spin until a card is read
  }

  // Build UID string (lowercase hex)
  String newUID = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    char buf[3];
    snprintf(buf, sizeof(buf), "%02x", rfid.uid.uidByte[i]);
    newUID += buf;
  }

  validUID = newUID;

  lcd.clear();
  lcd.print("Card updated");
  lcd.setCursor(0, 1);
  // Display last 8 chars if you want to fit; here we show whole (may wrap)
  lcd.print(newUID);
  delay(READ_milliSECONDS);

  // Cleanup RFID
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

void printLockStatus(bool isLocked) {
  lcd.clear();
  lcd.setCursor(0, 0);
  String text = isLocked ? "LOCKED" : "UNLOCKED";
  lcd.print(text);
}


void showReady() {
  lcd.clear();
  lcd.print(readyText);
}

// -------------------- Setup / Loop -------------------
void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();

  SPI.begin();
  rfid.PCD_Init();

  lcd.clear();
  lcd.print(readyText);
  delay(250);  // small delay
}

void loop() {
  // No new card? nothing to do now.
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  // Build UID string in lowercase hex
  String readUID = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    char buf[3];
    snprintf(buf, sizeof(buf), "%02x", rfid.uid.uidByte[i]);
    readUID += buf;
  }

  if (readUID == validUID) {
    // Prompt for admin or PIN
    lcd.clear();
    lcd.print("A=Card B=PIN");
    lcd.setCursor(0, 1);
    lcd.print("Enter code:");

    char firstKey = waitForAnyKey();

    // Admin options
    if (firstKey == 'A') {
      changeCardFlow();
    } else if (firstKey == 'B') {
      changePasscodeFlow();
    } else {
      // 3-try PIN verification workflow
      int localTries = MAX_NUMBER_TRIES;
      bool granted = false;

      // First attempt: if first key is a digit, use as prefill
      if (isDigitKey(firstKey)) {
        inputValue = GetCodePrefill(firstKey);
        if (compareCodes(inputValue, passcode)) {
          isLocked = !isLocked;
          printLockStatus(isLocked);
          delay(READ_milliSECONDS);
          granted = true;
        } else {
          localTries--;
          lcd.clear();
          lcd.print("Invalid code");
          lcd.setCursor(0, 1);
          lcd.print("Tries left: ");
          lcd.print(localTries);
          delay(READ_milliSECONDS);
        }
      }
      // Remaining attempts if not granted yet
      while (!granted && localTries > 0) {
        inputValue = GetCode();  // blocking capture; enforces min len
        if (compareCodes(inputValue, passcode)) {
          isLocked = !isLocked;
          printLockStatus(isLocked);
          delay(READ_milliSECONDS);
          granted = true;
          break;
        } else {
          localTries--;
          lcd.clear();
          lcd.print("Invalid code");
          lcd.setCursor(0, 1);
          lcd.print("Tries left: ");
          lcd.print(localTries);
          delay(READ_milliSECONDS);
        }
      }

      // Lockout if necessary
      if (!granted && localTries == 0) {
        int waitSeconds = LOCKED_OUT_SECONDS;
        while (waitSeconds > 0) {
          lcd.clear();
          lcd.print("Wait");
          lcd.setCursor(0, 1);
          lcd.print(waitSeconds);
          waitSeconds--;
          delay(READ_milliSECONDS);
        }
      }
    }
  } else {
    // Invalid card
    lcd.clear();
    lcd.print("Access denied");
    delay(READ_milliSECONDS);
  }

  // Cleanup RFID per cycle
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  delay(250);  // small debounce / pacing delay

  showReady();
}
