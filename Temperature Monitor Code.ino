#include "DHT.h"

#define DHTPIN 2
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

const int G_LED = 4;
const int R_LED = 5;
const int button = 6;

bool lastbuttonstate = HIGH;

// Debounce variables
unsigned long debounce = 0;
const unsigned long debounceDelay = 50;

// System state
bool systemstate = false;

// Sensor timing
unsigned long previousMillis = 0;
const unsigned long sensorinterval = 2000; // Read sensor every 2 seconds

void shutdownAndReset() {
    digitalWrite(G_LED, LOW);
    digitalWrite(R_LED, LOW);
}

void setup() {
    pinMode(G_LED, OUTPUT);
    pinMode(R_LED, OUTPUT);
    pinMode(button, INPUT_PULLUP);

    shutdownAndReset();

    Serial.begin(9600);
    dht.begin();
}

void loop() {
    unsigned long currentMillis = millis();

    // Debounce button
    bool reading = digitalRead(button);

    if (reading != lastbuttonstate) {
        debounce = currentMillis;
        lastbuttonstate = reading;
    }

    if ((currentMillis - debounce) > debounceDelay) {
        static bool confirmedstate = HIGH;

        if (reading == LOW && confirmedstate == HIGH) {
            systemstate = !systemstate;

            if (systemstate) {
                Serial.println(F(">>> SYSTEM STARTED <<<"));

                // Force immediate first sensor read
                previousMillis = currentMillis - sensorinterval;
            } else {
                Serial.println(F(">>> SYSTEM STOPPED <<<"));
                shutdownAndReset();
            }
        }

        confirmedstate = reading;
    }

    // Run system only when ON
    if (systemstate) {
        if (currentMillis - previousMillis >= sensorinterval) {
            previousMillis = currentMillis;

            float Temp = dht.readTemperature();

            if (isnan(Temp)) {
                Serial.println(F("ERROR: Sensor failed!"));
                return;
            }

            Serial.print(F("Temperature: "));
            Serial.print(Temp, 0);
            Serial.println(F(" C"));

            if (Temp < 35) {
                digitalWrite(G_LED, HIGH);
                digitalWrite(R_LED, LOW);
            } else {
                digitalWrite(G_LED, LOW);
                digitalWrite(R_LED, HIGH);
            }
        }
    }
}