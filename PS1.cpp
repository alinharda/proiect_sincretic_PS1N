#include <LiquidCrystal.h>

#define TP36_SENSOR_CHANNEL 0
#define ADC_REF_VOLTAGE 5.0

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

unsigned long previousMillis = 0;
const long interval = 1000;

unsigned long initialTime = 999;

void init_adc()
{
    ADMUX |= (1 << REFS0);
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    ADCSRA |= (1 << ADEN);
}

void setup()
{
    DDRD = 0b00111100;
    lcd.begin(16, 2);
    lcd.print("Temp= ");
    lcd.setCursor(0, 1);
    lcd.print("Ora ");
    init_adc();
}

void loop()
{
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval)
    {
        previousMillis = currentMillis;
        float temperatureC = read_temperature();
        lcd.setCursor(6, 0);
        lcd.print((int)temperatureC);
        unsigned long currentTime = (currentMillis / 1000) + initialTime;
        int hours = currentTime / 3600;
        int minutes = (currentTime % 3600) / 60;
        int seconds = currentTime % 60;
        lcd.setCursor(6, 1);
        if (hours < 10)
            lcd.print("0");
        lcd.print(hours);
        lcd.print(":");
        if (minutes < 10)
            lcd.print("0");
        lcd.print(minutes);
        lcd.print(":");
        if (seconds < 10)
            lcd.print("0");
        lcd.print(seconds);
    }
}

float read_temperature()
{
    ADMUX &= 0xF0;
    ADMUX |= TP36_SENSOR_CHANNEL;
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC))
    {
    }
    uint16_t adc_value = ADC;
    float voltage = (float)adc_value * ADC_REF_VOLTAGE / 1024.0;
    float temperature = (voltage - 0.5) * 100.0;
    return temperature;
}
