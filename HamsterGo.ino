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
    delay(1000);
    printRow(0, "      Hamster       ");
    delay(1000);
    printRow(1, "       G O !        ");
    delay(1000);
    printRow(3, "         3          ");
    delay(1000);
    printRow(3, "         2          ");
    delay(1000);
    printRow(3, "         1          ");
    delay(1000);
}

void loop()
{
    uint32_t timestampMs = millis() | 1; // nonzero timestamp = valid timestamp
    bool needUpdate = false;

    static uint32_t revolutionCount = 0;
    static uint32_t lastRevolutionTimestampMs = 0;
    static uint16_t hundredMetersPerHour = 0;
    static uint16_t maxSpeedHundredMetersPerHour = 0;
    static uint32_t pauseStartTimestampMs = timestampMs;
    if (detectStep())
    {
        revolutionCount++;

        if (pauseStartTimestampMs)
        {
            pauseStartTimestampMs = 0;
            hundredMetersPerHour = 0;
        }
        else
        {
            uint32_t revolutionDurationMs = timestampMs - lastRevolutionTimestampMs;
            const uint32_t msPerHour = 1000ul * 60ul * 60ul;
            const uint32_t cmPerHundredMeters = 10000ul;
            hundredMetersPerHour = revolutionDurationMs
                ? (wheelCircumferenceCm * msPerHour / revolutionDurationMs / cmPerHundredMeters)
                : 0;
            maxSpeedHundredMetersPerHour = (hundredMetersPerHour > maxSpeedHundredMetersPerHour)
                ? hundredMetersPerHour
                : maxSpeedHundredMetersPerHour;
        }
        lastRevolutionTimestampMs = timestampMs;

        needUpdate = true;
    }

    if (pauseStartTimestampMs == 0 && (timestampMs - lastRevolutionTimestampMs) > speedometerTimeoutMs)
    {
        hundredMetersPerHour = 0;
        pauseStartTimestampMs = timestampMs;
        needUpdate = true;
    }

    static uint32_t secondsTick = 0;
    uint32_t seconds = timestampMs / 1000ul;
    if (secondsTick != seconds)
    {
        secondsTick = seconds;
        needUpdate = true;
    }

    if (needUpdate)
    {
        printRow(0, "Runden: %12d", revolutionCount);

        uint16_t distanceCm = wheelCircumferenceCm * revolutionCount;
        printRow(1, "Strecke: %6d,%02d m", distanceCm / 100, distanceCm % 100);

        printRow(2, "Max-Tempo: %2d,%01d km/h", maxSpeedHundredMetersPerHour / 10, maxSpeedHundredMetersPerHour % 10);

        if (hundredMetersPerHour != 0)
        {
            printRow(3, "Tempo:     %2d,%01d km/h", hundredMetersPerHour / 10, hundredMetersPerHour % 10);
        }
        else
        {
            uint16_t pauseSec = (timestampMs - pauseStartTimestampMs) / 1000ul;
            printRow(3, "Inaktiv: %5d:%02d:%02d", pauseSec / 60 / 60, pauseSec / 60 % 60, pauseSec % 60);
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
