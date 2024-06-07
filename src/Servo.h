#pragma once
#include <Arduino.h>
#include <ESP32Servo.h>

class ServoPetals {
    public:
        ServoPetals();
        void init(bool initial, bool wokeUp);
        void update();

        void setPetalsOpenLevel(int8_t level, int transitionTime = 0);
        int8_t getPetalsOpenLevel();
        int8_t getCurrentPetalsOpenLevel();
        bool arePetalsMoving();
        bool setEnabled(bool enabled);

    private:
        // Config *config;

        // servo config
        Servo servo;

        // servo state
        int8_t petalsOpenLevel; // 0-100% (target angle in percentage)
        int16_t servoAngle; // current angle, keep signed to be able to calculate closing
        int16_t servoOriginAngle; // angle before animation, keep signed to be able to calculate closing
        int16_t servoTargetAngle; // angle after animation, keep signed to be able to calculate closing
        unsigned long movementStartTime;
        uint16_t movementTransitionTime;
        bool enabled;
        unsigned long servoPowerOffTime; // time when servo should power off (after animation is finished)
        bool initialized;
};