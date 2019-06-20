#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

const int irPin = 3;
const int ledPin = 13;

LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup()
{
  pinMode(irPin, INPUT);
  pinMode(ledPin, OUTPUT);
  lcd.begin();

  lcd.backlight();
  lcd.clear();
  lcd.print("Hamster, go!");
}

uint32_t count = 0;
uint32_t lastMs = 0;
void loop()
{
  const uint8_t circumFerenceCm = 87;
  bool update = false;
  uint32_t tNow = millis();
  uint32_t deltaMs = 0;
  if (detectStep()) {
    count++;
    deltaMs = tNow - lastMs;
    lastMs = tNow;
    update = true;
  }
  if ((tNow - lastMs) > 5000ul) {
    deltaMs = 0;
    update = true;
  }
  if (update)
  {
    char buff[21];
    sprintf(buff, "Runden: %12d", count);
    lcd.setCursor(0, 0);
    lcd.print(buff);
 
    lcd.setCursor(0, 3);
    if (deltaMs) {
      const uint32_t msPerHour = 1000ul * 60ul * 60ul;
      const uint32_t cmPerHundredMeters = 10000ul;
      uint16_t hundredMetersPerHour = circumFerenceCm * msPerHour / deltaMs / cmPerHundredMeters;
      sprintf(buff, "Tempo:     %2d.%01d km/h", hundredMetersPerHour / 10, hundredMetersPerHour % 10);
 
      lcd.print(buff);
    }
    else {
      lcd.print("Tempo:      0.0 km/h");
    }
    uint16_t distanceCm = circumFerenceCm * count;
    sprintf(buff, "Strecke: %6d.%02d m", distanceCm / 100, distanceCm % 100);
    lcd.setCursor(0, 1);
    lcd.print(buff);
  }
}

bool current = true;
bool detectStep()
{  
  bool previous = current;
  current = digitalRead(irPin) == HIGH;
  if (current != previous) delay(5);
  return current && !previous;
}
