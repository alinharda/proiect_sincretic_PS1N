#include <LiquidCrystal.h>

#define BUTTON_OK 6
#define BUTTON_CANCEL 7
#define BUTTON_PREV 8
#define BUTTON_NEXT 9

#define TP36_SENSOR_CHANNEL 0
#define ADC_REF_VOLTAGE 5.0

enum Buttons
{
  EV_OK,
  EV_CANCEL,
  EV_NEXT,
  EV_PREV,
  EV_NONE,
  EV_MAX_NUM
};

enum Menus
{
  MENU_MAIN = 0,
  MENU_KP,
  MENU_KI,
  MENU_KD,
  MENU_TI,
  MENU_TM,
  MENU_TR,
  MENU_TEMP,
  MENU_RUNTIME,
  MENU_MAX_NUM
};

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

double temp = 36.6;
double kp = 20;
double ki = 15;
double kd = 10;
double ti = 23.5;
double tm = 22;
double tr = 10;

float temp_c = 0;

Menus scroll_menu = MENU_MAIN;
Menus current_menu = MENU_MAIN;

void state_machine(enum Menus menu, enum Buttons button);
Buttons GetButtons(void);
void print_menu(enum Menus menu);

typedef void(state_machine_handler_t)(void);

void print_menu(enum Menus menu)
{
  lcd.clear();
  switch (menu)
  {
  case MENU_KP:
    lcd.print("KP = ");
    lcd.print(kp);
    break;
  case MENU_KI:
    lcd.print("KI = ");
    lcd.print(ki);
    break;
  case MENU_KD:
    lcd.print("KD = ");
    lcd.print(kd);
    break;
  case MENU_TI:
    lcd.print("Timp Incalzire: ");
    lcd.setCursor(1, 1);
    lcd.print(ti);
    break;
  case MENU_TM:
    lcd.print("Timp Mentinere: ");
    lcd.setCursor(1, 1);
    lcd.print(tm);
    break;
  case MENU_TR:
    lcd.print("Timp Racire: ");
    lcd.setCursor(1, 1);
    lcd.print(tr);
    break;
  case MENU_TEMP:
    lcd.print("TEMP = ");
    lcd.print(temp);
    break;
  case MENU_RUNTIME:
    lcd.setCursor(0, 0);
    lcd.print("Actual Temp = ");
    lcd.setCursor(1, 1);
    lcd.print(temp_c);
    break;
  case MENU_MAIN:
  default:
    lcd.print("PS 2023");
    break;
  }
  if (current_menu != MENU_MAIN)
  {
    lcd.setCursor(0, 1);
    lcd.print("modifica");
  }
}

void enter_menu(void)
{
  current_menu = scroll_menu;
}

void go_home(void)
{
  scroll_menu = MENU_MAIN;
  current_menu = scroll_menu;
}

void go_next(void)
{
  scroll_menu = (Menus)((int)scroll_menu + 1);
  scroll_menu = (Menus)((int)scroll_menu % MENU_MAX_NUM);
}

void go_prev(void)
{
  scroll_menu = (Menus)((int)scroll_menu - 1);
  scroll_menu = (Menus)((int)scroll_menu % MENU_MAX_NUM);
}

void save_kp(void)
{
}

void inc_kp(void)
{
  kp++;
}

void dec_kp(void)
{
  kp--;
}

void save_ki(void)
{
}

void inc_ki(void)
{
  ki++;
}

void dec_ki(void)
{
  ki--;
}

void save_kd(void)
{
}

void inc_kd(void)
{
  kd++;
}

void dec_kd(void)
{
  kd--;
}

void save_ti(void)
{
}

void inc_ti(void)
{
  ti++;
}

void dec_ti(void)
{
  ti--;
}

void save_tm(void)
{
}

void inc_tm(void)
{
  tm++;
}

void dec_tm(void)
{
  tm--;
}

void save_tr(void)
{
}

void inc_tr(void)
{
  tr++;
}

void dec_tr(void)
{
  tr--;
}

void save_temp(void)
{
}

void inc_temp(void)
{
  temp++;
}

void dec_temp(void)
{
  temp--;
}

state_machine_handler_t *sm[MENU_MAX_NUM][EV_MAX_NUM] =
    {
        // events: OK , CANCEL , NEXT, PREV
        {enter_menu, go_home, go_next, go_prev}, // MENU_MAIN
        {go_home, go_home, inc_kp, dec_kp},      // MENU_KP
        {go_home, go_home, inc_ki, dec_ki},
        {go_home, go_home, inc_kd, dec_kd},
        {go_home, go_home, inc_ti, dec_ti},
        {go_home, go_home, inc_tm, dec_tm},
        {go_home, go_home, inc_tr, dec_tr},
        {go_home, go_home, go_next, go_prev},
        {go_home, go_home, inc_temp, dec_temp}, // MENU_TEMP
};

void state_machine(enum Menus menu, enum Buttons button)
{
  sm[menu][button]();
}

Buttons GetButtons(void)
{
  enum Buttons ret_val = EV_NONE;
  if (digitalRead(BUTTON_OK))
  {
    ret_val = EV_OK;
  }
  else if (digitalRead(BUTTON_CANCEL))
  {
    ret_val = EV_CANCEL;
  }
  else if (digitalRead(BUTTON_NEXT))
  {
    ret_val = EV_NEXT;
  }
  else if (digitalRead(BUTTON_PREV))
  {
    ret_val = EV_PREV;
  }
  //   Serial.print(ret_val);

  return ret_val;
}

void init_adc()
{
  ADMUX |= (1 << REFS0);
  ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
  ADCSRA |= (1 << ADEN);
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

double eroare = 0;

double suma_erori = 0;

double eroare_anterioara = 0;

double derivativa = 0;

double output = 0;

double dt = 1000; // timp esantionare 1s

void PID()
{

  eroare = temp - temp_c;

  suma_erori = suma_erori + eroare * dt;

  derivativa = (eroare - eroare_anterioara) / dt;

  output = (kp * eroare) + (ki * suma_erori) + (kd * derivativa);

  eroare_anterioara = eroare;
}

void setup()
{
  Serial.begin(115200);
  DDRD = 0b00111100;
  lcd.begin(16, 2);
  init_adc();
}

void loop()
{

  temp_c = read_temperature();

  PID();

  volatile Buttons event = GetButtons();
  if (event != EV_NONE)
  {
    state_machine(current_menu, event);
  }
  print_menu(scroll_menu);
  Serial.print(temp);
  Serial.print(" | ");
  Serial.println(temp_c);
  Serial.println(output);
  delay(1000);
}