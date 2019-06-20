#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

const uint8_t wheelCircumferenceCm = 87;
const uint32_t speedometerTimeoutMs = 5000ul;

const uint8_t sensorPin = 3;

const uint8_t lcdColumns = 20;
const uint8_t lcdRows = 4;
const uint8_t lcdI2cAddress = 0x27;

LiquidCrystal_I2C lcd(lcdI2cAddress, lcdColumns, lcdRows);

void setup()
{
    pinMode(sensorPin, INPUT);
    lcd.begin();
    lcd.backlight();
    lcd.clear();
    lcd.print("Hamster, go!");
}

uint32_t revolutionCount = 0;
uint32_t lastRevolutionTimestampMs = 0;
void loop()
{
  bool needUpdate = false;
  uint32_t timestampMs = millis();

  uint32_t revolutionDurationMs;

  if (detectStep())
  {
    revolutionCount++;
    revolutionDurationMs = timestampMs - lastRevolutionTimestampMs;
    lastRevolutionTimestampMs = timestampMs;
    needUpdate = true;
  }

  if ((timestampMs - lastRevolutionTimestampMs) > speedometerTimeoutMs)
  {
    revolutionDurationMs = 0;
    needUpdate = true;
  }

  if (needUpdate)
  {
    char buff[21];
    sprintf(buff, "Runden: %12d", revolutionCount);
    lcd.setCursor(0, 0);
    lcd.print(buff);
 
    lcd.setCursor(0, 3);
    if (revolutionDurationMs != 0) {
      const uint32_t msPerHour = 1000ul * 60ul * 60ul;
      const uint32_t cmPerHundredMeters = 10000ul;
      uint16_t hundredMetersPerHour = wheelCircumferenceCm * msPerHour / revolutionDurationMs / cmPerHundredMeters;
      sprintf(buff, "Tempo:     %2d.%01d km/h", hundredMetersPerHour / 10, hundredMetersPerHour % 10);
      lcd.print(buff);
    }
    else {
      lcd.print("Tempo:      0.0 km/h");
    }
    uint16_t distanceCm = wheelCircumferenceCm * revolutionCount;
    sprintf(buff, "Strecke: %6d.%02d m", distanceCm / 100, distanceCm % 100);
    lcd.setCursor(0, 1);
    lcd.print(buff);
  }
}


bool current = true;
bool detectStep()
{  
  bool previous = current;
  current = digitalRead(sensorPin) == HIGH;
  if (current != previous) delay(5);
  return current && !previous;
}
