
/**
 * @file TestSketch.ino
 * @brief A simple Arduino sketch used to test Doxygen documentation generation.
 *
 * This sketch demonstrates how to use various Doxygen comment styles,
 * including documentation for functions, variables, classes, and enums.
 *
 * @author Your Name
 * @date 2026
 */

/** @brief LED pin number. */
const int LED_PIN = 13;

/**
 * @enum BlinkSpeed
 * @brief Possible blink speeds for the status LED.
 */
enum BlinkSpeed {
    SLOW = 500,   /**< 500 ms delay */
    FAST = 150    /**< 150 ms delay */
};

/**
 * @class Blinker
 * @brief A simple class that blinks an LED.
 *
 * Example usage:
 * @code
 * Blinker b(13, FAST);
 * b.blink();
 * @endcode
 */
class Blinker {
public:
    /**
     * @brief Constructor.
     * @param pin The pin number to control.
     * @param speed The blink speed (ms).
     */
    Blinker(int pin, BlinkSpeed speed)
        : pinNumber(pin), blinkSpeed(speed) {}

    /** 
     * @brief Blink the LED once.
     */
    void blink() {
        digitalWrite(pinNumber, HIGH);
        delay(blinkSpeed);
        digitalWrite(pinNumber, LOW);
        delay(blinkSpeed);
    }

private:
    int pinNumber;      /**< The LED pin */
    int blinkSpeed;     /**< The selected blink speed */
};


/**
 * @brief Setup runs once at startup.
 *
 * Initializes the LED pin and creates a Blinker object.
 */
void setup() {
    pinMode(LED_PIN, OUTPUT);
    Serial.begin(9600);

    Serial.println("Starting Doxygen test sketch...");
}


/**
 * @brief Loop runs repeatedly.
 *
 * Demonstrates calling a documented helper function.
 */
void loop() {
    blinkStatusLED(3, FAST);
    delay(1000);
}

/**
 * @brief Blink the status LED a number of times.
 *
 * This is a stand-alone function (not part of the class).
 *
 * @param count Number of times to blink.
 * @param speed Blink speed in milliseconds.
 * @return True if the sequence completed successfully.
 */
bool blinkStatusLED(int count, BlinkSpeed speed) {
    for (int i = 0; i < count; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(speed);
        digitalWrite(LED_PIN, LOW);
        delay(speed);
    }
    return true;
}
