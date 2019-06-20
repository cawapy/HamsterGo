#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

const uint8_t wheelCircumferenceCm = 87;
const uint32_t speedometerTimeoutMs = 5000ul;

const uint8_t sensorPin = 3;

const uint8_t lcdColumns = 20;
const uint8_t lcdRows = 4;
const uint8_t lcdI2cAddress = 0x27;

LiquidCrystal_I2C lcd(lcdI2cAddress, lcdColumns, lcdRows);

void printRow(uint8_t rowNumber, const char* format, ...);

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
    printRow(0, "Runden: %12d", revolutionCount);

    uint16_t distanceCm = wheelCircumferenceCm * revolutionCount;
    printRow(1, "Strecke: %6d.%02d m", distanceCm / 100, distanceCm % 100);
 
    if (revolutionDurationMs != 0)
    {
      const uint32_t msPerHour = 1000ul * 60ul * 60ul;
      const uint32_t cmPerHundredMeters = 10000ul;
      uint16_t hundredMetersPerHour = wheelCircumferenceCm * msPerHour / revolutionDurationMs / cmPerHundredMeters;
      printRow(3, "Tempo:     %2d.%01d km/h", hundredMetersPerHour / 10, hundredMetersPerHour % 10);
    }
    else
    {
      printRow(3, "                    ");
    }
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

void printRow(uint8_t rowNumber, const char* format, ...)
{
    const size_t bufferSize = lcdColumns + 1;
    char buffer[bufferSize];
    va_list arglist;
    va_start(arglist, format);
    // vnsprintf(buffer, bufferSize, format, arglist);
    vsprintf(buffer, format, arglist);
    va_end(arglist);
    lcd.setCursor(0, rowNumber);
    lcd.print(buffer);
}
