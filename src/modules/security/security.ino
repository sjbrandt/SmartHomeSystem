/**
 * @file security.ino
 * @brief Sketch for the security module
 *
 * This sketch is the code to be run on the ESP32 controlling the security module.
 *
 * Sensors:
 * - Keypad
 * - RFID reader and card(s)
 * Actuators:
 * - LCD screen
 *
 * @author Frederikke Biehe (s223981)
 * @author Sofus Brandt (s214972)
 * @date 2026
 */

#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Wire.h>
#include "sender_comm.h"

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

const int sensorID = 1;

// -------------------- Utility / Helpers --------------
/**
 * @brief Returns true if the given character is a digit
 * 
 * @param k character to check
 * @return bool
 * 
 * @author Frederikke Biehe (s223981)
 */
bool isDigitKey(char k) {
  return (k >= '0' && k <= '9');
}

/**
 * @brief Waits until a keypad key has been entered
 * 
 * @return char
 * 
 * @author Frederikke Biehe (s223981)
 */
char waitForAnyKey() {
  char k = NO_KEY;
  while (k == NO_KEY) {
    k = myKeypad.getKey();
  }
  return k;
}

/**
 * @brief Clears the given row of the LCD screen and sets the cursor to the first
 *        column of the same row
 * 
 * @param row row number to be cleared
 * 
 * @author Frederikke Biehe (s223981)
 */
void lcdLineClear(uint8_t row) {
  lcd.setCursor(0, row);
  lcd.print("                ");  // 16 spaces
  lcd.setCursor(0, row);
}

/**
 * @brief Prints a right-aligned message to the LCD screen
 * 
 * @param row row number to be printed on
 * @param msg message to print
 * 
 * @author Sofus Brandt (s214972)
 */
void lcdPrintRight(uint8_t row, String msg) {
  lcd.setCursor(16 - msg.length(), row);
  lcd.print(msg);
}

/**
 * @brief Compares two passcodes for equality
 * 
 * @param code1 first code for comparison
 * @param code2 second code for comparison
 * @return bool
 * 
 * @author Frederikke Biehe (s223981)
 */
bool compareCodes(const String &code1, const String &code2) {
  return (code1 == code2);
}

// -------------------- PIN Entry Functions ------------
/**
 * @brief Gets a numeric passcode of length >= 4 from the user. Provides
 *        masked output in the form of a * for each digit entered.
 * 
 * @param prompt text displayed before the input location
 * @return String
 * 
 * @author Frederikke Biehe (s223981)
 */
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

/**
 * @brief Gets a numeric passcode of length >= 4 from the user, where the
 *        first digit is given as a parameter. Is used for the main screen
 *        where input type is only determined when first digit is entered.
 *        Provides masked output in the form of a * for each digit entered.
 * 
 * @param firstDigit first digit of inputted passcode
 * @return String 
 * 
 * @author Frederikke Biehe (s223981)
 */
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

// -------------------- Admin Flows --------------------
/**
 * @brief Flow for changing password. Requires entering
 *        the current passcode first.
 * 
 * @author Frederikke Biehe (s223981)
 */
void changePasscodeFlow() {
  // Verify old PIN
  String oldPin = GetCodeWithPrompt("Old PIN:");
  if (oldPin != passcode) {
    lcd.clear();
    lcd.print("Wrong PIN");
    delay(READ_milliSECONDS);
    return;
  }

  // Get new passcode
  String newCode = newPasscodeFlow();
  if (passcode = "error") {
    return;
  }

  // Commit
  passcode = newCode;

  lcd.clear();
  lcd.print("PIN Updated");
  delay(READ_milliSECONDS);
  
}

/**
 * @brief Asks the user for and returns a new passcode. Requires entering
 *        the new passcode twice for confirmation.
 * 
 * @return String 
 * 
 * @author Frederikke Biehe (s223981)
 * @author Sofus Brandt (s214972)
 */
String newPasscodeFlow() {
  // New PIN
  String new1 = GetCodeWithPrompt("New PIN:");
  // Confirm
  String new2 = GetCodeWithPrompt("Confirm:");
  if (new1 != new2) {
    lcd.clear();
    lcd.print("Mismatch");
    delay(READ_milliSECONDS);
    return "error";
  }
  return new1;
}

// Wait for and store a new RFID UID as the valid card
/**
 * @brief Flow for changing the authorized RFID card, as well as password.
 *        Changes the global variables `validUID` and `passcode`.
 * 
 * @author Frederikke Biehe (s223981)
 * @author Sofus Brandt (s214972)
 */
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

  // Change passcode
  String newCode = newPasscodeFlow();
  if (newCode == "error") {
    return;
  }

  // Commit
  validUID = newUID;
  passcode = newCode;

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

/**
 * @brief Prints new locked status to the LCD on row 0.
 * 
 * @param isLocked locked state to be displayed.
 * 
 * @author Frederikke Biehe (s223981)
 * @author Sofus Brandt
 */
void printLockStatus(bool isLocked) {
  lcd.clear();
  lcd.setCursor(0, 0);
  String text = isLocked ? "LOCKED" : "UNLOCKED";
  lcd.print(text);
}

/**
 * @brief Prints a message to the LCD that the RFID scanner is ready.
 * 
 * @author Frederikke Biehe (s223981)
 * @author Sofus Brandt
 */
void showReady() {
  lcd.clear();
  lcd.print(readyText);
  String lockedText = isLocked ? "[Locked]" : "[Unlocked]";
  lcdPrintRight(1, lockedText);
}

// -------------------- Setup / Loop -------------------
/**
 * @brief Runs setup functions when the program is first loaded onto the MCU.
 * 
 * @author Frederikke Biehe (s223981)
 */
void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();

  SPI.begin();
  rfid.PCD_Init();

  showReady();
  delay(250);  // small delay

  // connect to hotspot
  initWifi();
}

/**
 * @brief Runs the main functionality in an infinite loop.
 * 
 * @author Frederikke Biehe (s223981)
 */
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
          // send data to central hub
          jsonInit(sensorID, "security");
          jsonAddBool("isLocked", isLocked);
          jsonSend();

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
        inputValue = GetCodeWithPrompt("Enter code:");
        if (compareCodes(inputValue, passcode)) {
          isLocked = !isLocked;
          // send data to central hub
          jsonInit(sensorID, "security");
          jsonAddBool("isLocked", isLocked);
          jsonSend();
          
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
