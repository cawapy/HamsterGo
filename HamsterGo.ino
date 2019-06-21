#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

const uint8_t wheelCircumferenceCm = 87;
const uint32_t speedometerTimeoutMs = 5000ul;

const uint8_t wheelMarkSensorPin = A3;

const uint8_t lcdColumns = 20;
const uint8_t lcdRows = 4;
const uint8_t lcdI2cAddress = 0x27;

LiquidCrystal_I2C lcd(lcdI2cAddress, lcdColumns, lcdRows);

void printRow(uint8_t rowNumber, const char* format, ...);

void setup()
{
    pinMode(wheelMarkSensorPin, INPUT);
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
    static uint16_t speed100mph = 0; // 100mph = 100 meters per hour
    static uint16_t maxSpeed100mph = 0;
    static uint32_t pauseStartTimestampMs = timestampMs;
    if (detectWheelRevolution())
    {
        revolutionCount++;

        if (pauseStartTimestampMs)
        {
            pauseStartTimestampMs = 0;
            speed100mph = 0;
        }
        else
        {
            uint32_t revolutionDurationMs = timestampMs - lastRevolutionTimestampMs;
            const uint32_t msPerHour = 1000ul * 60ul * 60ul;
            const uint32_t cmPerHundredMeters = 10000ul;
            speed100mph = revolutionDurationMs
                ? (wheelCircumferenceCm * msPerHour / revolutionDurationMs / cmPerHundredMeters)
                : 0;
            maxSpeed100mph = (speed100mph > maxSpeed100mph)
                ? speed100mph
                : maxSpeed100mph;
        }
        lastRevolutionTimestampMs = timestampMs;

        needUpdate = true;
    }

    if (pauseStartTimestampMs == 0 && (timestampMs - lastRevolutionTimestampMs) > speedometerTimeoutMs)
    {
        speed100mph = 0;
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

        printRow(2, "Max-Tempo: %2d,%01d km/h", maxSpeed100mph / 10, maxSpeed100mph % 10);

        if (speed100mph != 0)
        {
            printRow(3, "Tempo:     %2d,%01d km/h", speed100mph / 10, speed100mph % 10);
        }
        else
        {
            uint16_t pauseSec = (timestampMs - pauseStartTimestampMs) / 1000ul;
            printRow(3, "Inaktiv: %5d:%02d:%02d", pauseSec / 60 / 60, pauseSec / 60 % 60, pauseSec % 60);
        }
    }
}

// returns true when wheel mark just appeared at sensor (was not at sensor in previous call but is there now)
bool detectWheelRevolution()
{
    static bool isWheelMarkDetectedNow = false;
    bool wasWheelMarkDetectedBefore = isWheelMarkDetectedNow;
    isWheelMarkDetectedNow = digitalRead(wheelMarkSensorPin) == LOW;
    if (isWheelMarkDetectedNow != wasWheelMarkDetectedBefore) delay(5); // poor man's debounce; sufficient for hamster wheel
    return isWheelMarkDetectedNow && !wasWheelMarkDetectedBefore;
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
