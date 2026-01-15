#include <Keypad.h>
#include <LiquidCrystal_I2C.h>

const byte numRows = 4;  //number of rows on the keypad
const byte numCols = 4;  //number of columns on the keypad

char keymap[numRows][numCols] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

char keypressed;                       //Where the keys are stored it changes very often
char code[] = { '6', '6', '0', '1' };  //The default code, you can change it or make it a 'n' digits one

char code_buff1[sizeof(code)];  //Where the new key is stored
char code_buff2[sizeof(code)];  //Where the new key is stored again so it's compared to the previous one

short a = 0, i = 0, s = 0, j = 0;  //Variables used later

byte rowPins[numRows] = { 13, 12, 14, 27 };  //Rows 0 to 3 //if you modify your pins you should modify this too
byte colPins[numCols] = { 26, 25, 33, 32 };  //Columns 0 to 3

LiquidCrystal_I2C lcd(0x27, 16, 2);
Keypad myKeypad = Keypad(makeKeymap(keymap), rowPins, colPins, numRows, numCols);

void setup() {
  // initialize LCD
  lcd.init();
  // turn on LCD backlight
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Standby");  //What's written on the LCD you can change
}

void loop() {
  keypressed = myKeypad.getKey();  //Constantly waiting for a key to be pressed
  if (keypressed == '*') {         // * to open the lock
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enter code");  //Message to show
    GetCode();                //Getting code function
    if (a == sizeof(code))    //The GetCode function assign a value to a (it's correct when it has the size of the code array)
      OpenDoor();             //Open lock function if code is correct
    else {
      lcd.clear();
      lcd.print("Wrong");  //Message to print when the code is wrong
    }
    delay(2000);
    lcd.clear();
    lcd.print("Standby");  //Return to standby mode it's the message do display when waiting
  }

  if (keypressed == 'C') {  //To change the code it calls the changecode function
    ChangeCode();
    lcd.clear();
    lcd.print("Standby");  //When done it returns to standby mode
  }
}

void GetCode() {  //Getting code sequence
  i = 0;          //All variables set to 0
  a = 0;
  j = 0;

  while (keypressed != '#') {  //The user press # to confirm the code otherwise he can keep typing
    keypressed = myKeypad.getKey();
    if (keypressed != NO_KEY && keypressed != '#') {  //If the char typed isn't A and neither "nothing"
      lcd.setCursor(j, 1);                            //This to write "*" on the LCD whenever a key is pressed it's position is controlled by j
      lcd.print("*");
      j++;
      if (keypressed == code[i] && i < sizeof(code)) {  //if the char typed is correct a and i increments to verify the next caracter
        a++;                                            //Now I think maybe I should have use only a or i ... too lazy to test it -_-'
        i++;
      } else {
        a--;
      }  //if the character typed is wrong a decrements and cannot equal the size of code []
    }
  }
  keypressed = NO_KEY;
}

void ChangeCode() {  //Change code sequence
  lcd.clear();
  lcd.print("Changing code");
  delay(1000);
  lcd.clear();
  lcd.print("Enter old code");
  GetCode();  //verify the old code first so you can change it

  if (a == sizeof(code)) {  //again verifying the a value
    lcd.clear();
    lcd.print("Changing code");
    GetNewCode1();  //Get the new code
    GetNewCode2();  //Get the new code again to confirm it
    s = 0;
    for (i = 0; i < sizeof(code); i++) {  //Compare codes in array 1 and array 2 from two previous functions
      if (code_buff1[i] == code_buff2[i]) {
        s++;
      }  //again this how we verifiy, increment s whenever codes are matching
    }
    if (s == sizeof(code)) {  //Correct is always the size of the array

      for (i = 0; i < sizeof(code); i++) {
        code[i] = code_buff2[i];  //the code array now receives the new code
      }
      lcd.clear();
      lcd.print("Code Changed");
      delay(2000);
    } else {  //In case the new codes aren't matching
      lcd.clear();
      lcd.print("Codes are not");
      lcd.setCursor(0, 1);
      lcd.print("matching !!");
      delay(2000);
    }

  }

  else {  //In case the old code is wrong you can't change it
    lcd.clear();
    lcd.print("Wrong");
    delay(2000);
  }
}

void GetNewCode1() {
  i = 0;
  j = 0;
  lcd.clear();
  lcd.print("Enter new code");  //tell the user to enter the new code and press #
  lcd.setCursor(0, 1);
  lcd.print("and press #");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("and press #");  //Press # keep showing while the top row print ***

  while (keypressed != '#') {  //# to confirm and quits the loop
    keypressed = myKeypad.getKey();
    if (keypressed != NO_KEY && keypressed != '#') {
      lcd.setCursor(j, 1);
      lcd.print("*");              //On the new code you can show * as I did or change it to keypressed to show the keys
      code_buff1[i] = keypressed;  //Store caracters in the array
      i++;
      j++;
    }
  }
  keypressed = NO_KEY;
}

void GetNewCode2() {  //This is exactly like the GetNewCode1 function but this time the code is stored in another array
  i = 0;
  j = 0;

  lcd.clear();
  lcd.print("Confirm code");
  lcd.setCursor(0, 1);
  lcd.print("and press #");
  delay(3000);
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("and press #");

  while (keypressed != '#') {
    keypressed = myKeypad.getKey();
    if (keypressed != NO_KEY && keypressed != '#') {
      lcd.setCursor(j, 1);
      lcd.print("*");
      code_buff2[i] = keypressed;
      i++;
      j++;
    }
  }
  keypressed = NO_KEY;
}

void OpenDoor() {  //Lock opening function open for 3s
  lcd.clear();
  lcd.print("Welcome");  //With a message printed
}