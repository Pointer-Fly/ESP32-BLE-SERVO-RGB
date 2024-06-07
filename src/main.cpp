/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updates by chegewara
*/

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
// #include <ESP32Servo.h>
#include <FastLED.h>

#define LED_PIN 15
#define NUM_LEDS 1

// #define MAX_WIDTH 2500
// #define MIN_WIDTH 500
// #define SERVO_PIN 12
#include <Servo.h>

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_SERVO_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHARACTERISTIC_RGB_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a9"

// Servo my_servo;
ServoPetals my_servo = ServoPetals();
CRGB leds[NUM_LEDS];

class BLECallbacks : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string value = pCharacteristic->getValue();
    // 区分不同的特征值
    if (pCharacteristic->getUUID().equals(BLEUUID(CHARACTERISTIC_RGB_UUID)))
    {
      if (value.length() > 0)
      {
        Serial.println("*********");
        if (value[0] == '0')
        {
          Serial.println("Servo down");
          my_servo.setPetalsOpenLevel(0);
        }
        else if (value[0] == '1')
        {
          Serial.println("Servo up");
          my_servo.setPetalsOpenLevel(100);
        }
        Serial.println();
        Serial.println("*********");
      }
    }
    else
    {
      if (value.length() > 0)
      {
        Serial.println("*********");
        Serial.print("New value: ");
        if (value[0] == 'R')
        {
          leds[0] = CRGB::Red;
          Serial.println("Red");
          FastLED.show();
        }
        else if (value[0] == 'G')
        {
          leds[0] = CRGB::Green;
          Serial.println("Green");
          FastLED.show();
        }
        else if (value[0] == 'B')
        {
          leds[0] = CRGB::Blue;
          Serial.println("Blue");
          FastLED.show();
        }
        Serial.println();
        Serial.println("*********");
      }
    }
  }
};


void setup()
{
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  BLEDevice::init("ESP32");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristicServo = pService->createCharacteristic(
      CHARACTERISTIC_SERVO_UUID,
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_WRITE);

  pCharacteristicServo->setValue("0");
  pCharacteristicServo->setCallbacks(new BLECallbacks());

  BLECharacteristic *pCharacteristicRGB = pService->createCharacteristic(
      CHARACTERISTIC_RGB_UUID,
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_WRITE);
  pCharacteristicRGB->setValue("0");
  pCharacteristicRGB->setCallbacks(new BLECallbacks());

  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");

  // 分配硬件定时器
  // ESP32PWM::allocateTimer(0);
  // 设置频率
  // my_servo.setPeriodHertz(50);
  // 关联 servo 对象与 GPIO 引脚，设置脉宽范围
  // my_servo.attach(SERVO_PIN, MIN_WIDTH, MAX_WIDTH);
  // 使能RGB灯
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);

  my_servo.init(true, false);

}

void loop()
{
  my_servo.update();
}