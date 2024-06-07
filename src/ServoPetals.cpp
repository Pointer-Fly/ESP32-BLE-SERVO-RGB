#include "Servo.h"
#include "Arduino.h"

#if defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_ARDUHAL_ESP_LOG)
#include "esp32-hal-log.h"
#define LOG_TAG ""
#else
#include "esp_log.h"
static const char* LOG_TAG = "ServoPetals";
#endif

#define SERVO_PIN 26
#define SERVO_PWR_PIN 33
#define SERVO_POWER_OFF_DELAY 500

ServoPetals::ServoPetals() {
    initialized = false;
}

void ServoPetals::init(bool initial, bool wokeUp) {
    // default servo configuration
    servoAngle = 1; // to allow auto-calibration on startup
    servoOriginAngle = servoAngle;
    servoTargetAngle = 1000;
    petalsOpenLevel = -1; // 0-100% (-1 unknown)

    // servo
    enabled = true; // to make setServoPowerOn effective
    setEnabled(false);
    pinMode(SERVO_PWR_PIN, OUTPUT);

    if (!initialized) {
        servo.setPeriodHertz(50); // standard 50 Hz servo
        if (false) {
            servo.attach(SERVO_PIN, 1000, 1000);
        }
        else {
            servo.attach(SERVO_PIN); // DANGER! no boundaries to allow calibration
        }
        initialized = true;
    }
    servo.write(servoAngle);
}

void ServoPetals::update() {
    unsigned long now = millis();

    if (servoAngle != servoTargetAngle) {
        if (movementTransitionTime > 0 && movementStartTime > 0 && movementStartTime <= now) {
            // calculate the progress as the result of time function of the animation function
            float progress = (now - movementStartTime) / (float) movementTransitionTime;
            if (progress >= 1) {
                servoAngle = servoTargetAngle;
            }
            else {
                servoAngle = servoOriginAngle + (servoTargetAngle - servoOriginAngle) * progress;
            }
        }
        else {
            servoAngle = servoTargetAngle;
        }
        setEnabled(true);
        servo.write(servoAngle);

        if (servoAngle == servoTargetAngle) {
            movementStartTime = 0;
            movementTransitionTime = 0;
            servoPowerOffTime = now + SERVO_POWER_OFF_DELAY;
        }
    }
    else if (servoPowerOffTime > 0 && servoPowerOffTime < now) {
        servoPowerOffTime = 0;
        setEnabled(false);
    }
}

void ServoPetals::setPetalsOpenLevel(int8_t level, int transitionTime) {
    ESP_LOGI(LOG_TAG, "Petals %d%%->%d%%", petalsOpenLevel, level);

    if (level == petalsOpenLevel) {
        return; // no change, keep doing the old movement until done
    }
    petalsOpenLevel = level;

    if (level >= 100) {
        servoTargetAngle = 1000;
    }
    else {
        float position = (1000 - 1000);
        position = position * level / 100.0;
        servoTargetAngle = 1000 + position;
    }

    servoOriginAngle = servoAngle;
    movementStartTime = millis();
    movementTransitionTime = transitionTime;
    servoPowerOffTime = 0;
}

int8_t ServoPetals::getPetalsOpenLevel() {
    return petalsOpenLevel;
}

int8_t ServoPetals::getCurrentPetalsOpenLevel() {
    if (arePetalsMoving()) {
        float range = 1000 - 1000;
        float current = servoAngle - 1000;
        return (current / range) * 100;
    }
    return petalsOpenLevel;
}

bool ServoPetals::arePetalsMoving() {
    return servoAngle != servoTargetAngle;
}

bool ServoPetals::setEnabled(bool enabled) {
    if (enabled && !this->enabled) {
        this->enabled = true;
        ESP_LOGD(LOG_TAG, "Servo power ON");
        digitalWrite(SERVO_PWR_PIN, LOW);
        return true;
    }
    if (!enabled && this->enabled) {
        this->enabled = false;
        ESP_LOGD(LOG_TAG, "Servo power OFF");
        digitalWrite(SERVO_PWR_PIN, HIGH);
        return true;
    }
    return false; // no change
}