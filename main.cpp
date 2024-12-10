#include "mbed.h"
#include "DHT22.h"  // Library for DHT22 temperature and humidity sensor
#include "motordriver.h"  // Library for motor control
#include "Adafruit_GFX/Adafruit_SSD1306.h"  // Library for OLED display

// Define pin connections
#define JOYSTICK_X_PIN PC_2        // Joystick X-axis analog pin
#define FIRST_BTN_PIN PA_14        // Button for incrementing count
#define SECOND_BTN_PIN PC_4        // Button for saving current count
#define THIRD_BTN_PIN PB_7         // Button for decrementing count
#define MOTOR_A_PWM_PIN D11        // PWM pin for Motor A
#define MOTOR_A_DIR_PIN PC_8       // Direction control pin for Motor A
#define BUZZER_PIN PC_9            // Buzzer control pin
#define GREEN_LED_PIN PA_13        // Green LED pin

// Initialize peripherals
AnalogIn xAxis(JOYSTICK_X_PIN);            // Joystick X-axis analog input
DigitalOut greenLed(GREEN_LED_PIN);        // Green LED output
DHT22 dht22(PB_2);                         // DHT22 sensor for temperature and humidity
DigitalIn incCountBtn(FIRST_BTN_PIN);      // Increment button
DigitalIn saveCountBtn(SECOND_BTN_PIN);    // Save button
DigitalIn decCountBtn(THIRD_BTN_PIN);      // Decrement button
PwmOut buzzerPwm(BUZZER_PIN);              // Buzzer PWM output

Motor motorA(MOTOR_A_PWM_PIN, MOTOR_A_DIR_PIN);  // Motor A driver
Ticker ctrlTicker;        // Ticker for periodic control tasks
Ticker jsTicker;          // Ticker for joystick updates
volatile int x;           // Joystick X-axis position value

I2C myI2C(I2C_SDA, I2C_SCL);  // I2C interface for OLED
Adafruit_SSD1306_I2c myOled(myI2C, D13, 0x78, 64, 128);  // OLED display configuration

// Central value for joystick calibration
#define CENTERAL_VALUE 74

// Function to read joystick position
void readJoystick() {
    x = xAxis * 100;                // Read joystick X-axis value (scaled to 0-100)
    x = CENTERAL_VALUE - x;         // Adjust relative to center
    if (abs(x) <= 2) x = 0;         // Treat near-center values as zero
}

// Function to display information on the OLED
void displayOled(const char* password, int count, float temperature, float humidity, bool locked) {
    myOled.clearDisplay();
    myOled.setTextSize(1);
    myOled.setTextCursor(0, 0);
    myOled.printf("Temp: %.1f C", temperature);  // Display temperature
    myOled.setTextCursor(0, 10);
    myOled.printf("Humidity: %.1f %%", humidity);  // Display humidity
    myOled.setTextCursor(0, 20);
    myOled.printf("Password: %s", password);  // Display current password
    myOled.setTextCursor(0, 30);
    myOled.printf("Count: %d", count);  // Display count value
    myOled.setTextCursor(0, 40);
    myOled.printf("Locked");  // Display lock status
    myOled.display();
}

// Function to display access status on the OLED
void displayAccessStatus(const char* status, bool locked) {
    myOled.clearDisplay();
    myOled.setTextSize(1);
    myOled.setTextCursor(0, 0);
    myOled.printf(status);  // Display access status
    myOled.setTextCursor(0, 10);
    myOled.printf(locked ? "Locked" : "Unlocked");  // Display lock state
    myOled.display();
}

// Function to check if the input password matches the saved password
bool checkPassword(const char* password, const char* input) {
    for (int i = 0; i < 4; i++) {
        if (password[i] != input[i]) {
            return false;  // Password mismatch
        }
    }
    return true;  // Password match
}

// System setup function
void setup() {
    jsTicker.attach(&readJoystick, 0.1);  // Attach joystick reading function to ticker

    // Initialize OLED display with default values
    myOled.clearDisplay();
    myOled.setTextSize(1);
    myOled.setTextCursor(0, 0);
    myOled.printf("Temp: 0.0 C");
    myOled.setTextCursor(0, 10);
    myOled.printf("Humidity: 0.0 %%");
    myOled.display();
}

// Main function
int main() {
    float frequency[] = {659.26, 440.00, 392.00, 493.88};  // Frequencies for buzzer
    int pcount = 0;                // Current count
    float humidity, temperature;   // Humidity and temperature values
    const char SavePassword[4] = {'1', '2', '3', '4'};  // Saved password
    char InputPassword[4] = {0};   // User input password
    int passwordIndex = 0;         // Password input index
    bool locked = true;            // Lock status

    setup();  // Call setup function

    while (1) {
        // Button state detection
        bool isIncCountBtnPressed = !incCountBtn;  // Increment button pressed
        bool isSaveCountBtnPressed = !saveCountBtn;  // Save button pressed
        bool isDecCountBtnPressed = !decCountBtn;  // Decrement button pressed

        // Handle count increment
        if (isIncCountBtnPressed && pcount < 9) {
            pcount++;
        }

        // Handle count saving
        if (isSaveCountBtnPressed && passwordIndex < 4) {
            InputPassword[passwordIndex] = '0' + pcount;  // Save count to password
            passwordIndex++;
        }

        // Handle count decrement
        if (isDecCountBtnPressed && pcount > 0) {
            pcount--;
        }

        // Update temperature and humidity values
        if (dht22.sample()) {
            temperature = dht22.getTemperature() / 10.0;  // Read temperature
            humidity = dht22.getHumidity() / 10.0;        // Read humidity
        }

        // Display current status on OLED
        displayOled(InputPassword, pcount, temperature, humidity, locked);

        // Handle joystick input for password verification
        if (x > 50) {
            if (checkPassword(SavePassword, InputPassword)) {
                displayAccessStatus("Unlocked", false);  // Display unlocked status
                locked = false;

                // Play unlock sound
                for (int i = 0; i < 4; i++) {
                    buzzerPwm.period(1.0 / frequency[i]);
                    buzzerPwm = 0.5;
                    wait(0.5);
                    buzzerPwm = 0;
                    wait(0.2);
                }

                // Open motor and wait
                motorA.forward(0.3);
                wait(1.5);
                motorA.stop();
                wait(30.0);

                // Play lock sound
                for (int i = 3; i >= 0; i--) {
                    buzzerPwm.period(1.0 / frequency[i]);
                    buzzerPwm = 0.5;
                    wait(0.5);
                    buzzerPwm = 0;
                    wait(0.2);
                }

                // Close motor
                motorA.backward(0.3);
                wait(1.5);
                motorA.stop();
            } else {
                displayAccessStatus("Wrong Password", true);  // Display error
                for (int i = 0; i < 2; i++) {
                    buzzerPwm.period_us(3830);
                    buzzerPwm = 0.5;
                    wait(0.2);
                    buzzerPwm = 0;
                    wait(0.2);
                }
                wait(3.0);
            }

            // Reset password inputs
            displayOled(InputPassword, pcount, temperature, humidity, locked);
            passwordIndex = 0;
            memset(InputPassword, 0, sizeof(InputPassword));
            pcount = 0;
        }

        wait(1.0);  // Wait before the next loop iteration
    }
}
