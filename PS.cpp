#include <LiquidCrystal.h>
#include <DHT.h>
#include <EEPROM.h>

#define BUTTON_OK 6
#define BUTTON_CANCEL 7
#define BUTTON_PREV 8
#define BUTTON_NEXT 9

// Adress
int addrs_kp = 100;
int addrs_ki = 108;
int addrs_kd = 116;
int addrs_temp = 124;
int addrs_ti = 132;
int addrs_tm = 140;
int addrs_tr = 148;

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

double temp_c = 0;
double temp = 29.0;
double kp = 9.6;
double ki = 20;
double kd = 5.44;

double ti = 100;
double tm = 20;
double tr = 100;

double eroare = 0;
double suma_erori = 0;
double eroare_anterioara = 0;
double derivativa = 0;
double output = 0;
double dt = 1.3, last_time = 0; // timp esantionare 1s

unsigned long now = 0;
unsigned long start_time = 0; // New variable to store the start time

struct temp_hum_sensor_components
{
  int DHTPIN = 13;
  int DHTTYPE = DHT22;

  int chk;
  float hum;
  float temp;
};

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

Menus scroll_menu = MENU_MAIN;
Menus current_menu = MENU_MAIN;
temp_hum_sensor_components dht_sensor;
DHT dht(dht_sensor.DHTPIN, dht_sensor.DHTTYPE);
void print_menu(enum Menus menu);
void state_machine(enum Menus menu, enum Buttons button);
Buttons GetButtons(void);
typedef void(state_machine_handler_t)(void);

unsigned long last_runtime_start_time = 0;

void save_memory(int address, double value)
{
  double storedValue;

  EEPROM.get(address, storedValue);
  if (value != storedValue)
  {
    EEPROM.put(address, value);
  }
}

double read_memory(int address)
{
  double storedValue;
  EEPROM.get(address, storedValue);
  return storedValue;
}

void read_temperature()
{
  dht_sensor.temp = dht.readTemperature();
  if (!isnan(dht_sensor.temp))
  {
    temp_c = dht_sensor.temp;
  }
  else
  {
    // Handle the case when the temperature reading is not valid
    // You can choose to set a default value or take other appropriate actions.
    // For example, setting temp_c to a default value:
    temp_c = 0.0;
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

void inc_kp(void)
{
  kp++;
  save_memory(addrs_kp, kp);
}

void dec_kp(void)
{
  kp--;
  save_memory(addrs_kp, kp);
}

void inc_ki(void)
{
  ki++;
  save_memory(addrs_ki, ki);
}

void dec_ki(void)
{
  ki--;
  save_memory(addrs_ki, ki);
}

void inc_kd(void)
{
  kd++;
  save_memory(addrs_kd, kd);
}

void dec_kd(void)
{
  kd--;
  save_memory(addrs_kd, kd);
}

void inc_ti(void)
{
  ti++;
  save_memory(addrs_ti, ti);
}

void dec_ti(void)
{
  ti--;
  save_memory(addrs_ti, ti);
}

void inc_tm(void)
{
  tm++;
  save_memory(addrs_tm, tm);
}

void dec_tm(void)
{
  tm--;
  save_memory(addrs_tm, tm);
}

void inc_tr(void)
{
  tr++;
  save_memory(addrs_tr, tr);
}

void dec_tr(void)
{
  tr--;
  save_memory(addrs_tr, tr);
}

void inc_temp(void)
{
  temp++;
  save_memory(addrs_temp, temp);
}

void dec_temp(void)
{
  temp--;
  save_memory(addrs_temp, temp);
}

state_machine_handler_t *sm[MENU_MAX_NUM][EV_MAX_NUM] = {
    // events: OK , CANCEL , NEXT, PREV
    {enter_menu, go_home, go_next, go_prev}, // MENU_MAIN
    {go_home, go_home, inc_kp, dec_kp},      // MENU_KP
    {go_home, go_home, inc_ki, dec_ki},
    {go_home, go_home, inc_kd, dec_kd},
    {go_home, go_home, inc_ti, dec_ti},
    {go_home, go_home, inc_tm, dec_tm},
    {go_home, go_home, inc_tr, dec_tr},
    {go_home, go_home, inc_temp, dec_temp},
    {go_home, go_home, go_next, go_prev}, // MENU_TEMP
};

void PID()
{

  // dt = (now - last_time) / 1000;
  // last_time = now;

  eroare = temp - temp_c;

  if (eroare == 0)
  {
    suma_erori = 0;
  }

  suma_erori = suma_erori + eroare * dt;

  derivativa = (eroare - eroare_anterioara) / dt;

  output = (kp * eroare) + (ki * suma_erori) + (kd * derivativa);

  // Serial.print("\nEroare: ");
  // Serial.print(eroare);
  // Serial.print(" | Suma Erori: ");
  // Serial.print(suma_erori);
  // Serial.print(" | Derivativa: ");
  // Serial.print(derivativa);
  // Serial.print(" | dt: ");
  // Serial.print(dt);

  // Serial.println("");
  // Serial.print("IN OUTPUT:");
  // Serial.print(output);
  // Serial.print(" | ");
  output = constrain(output, -500, 500);

  // Serial.print("CONST OUTPUT:");
  // Serial.print(output);
  // Serial.print(" | ");

  output = map(output, -500, 500, 0, 255);

  // Serial.print("FINAL OUTPUT:");
  // Serial.println(output);

  eroare_anterioara = eroare;
}

void setup()
{
  Serial.begin(115200);
  // lcdDisplay
  lcd.begin(16, 2);
  dht.begin();

  kp = read_memory(addrs_kp);
  ki = read_memory(addrs_ki);
  kd = read_memory(addrs_kd);
  ti = read_memory(addrs_ti);
  tm = read_memory(addrs_tm);
  tr = read_memory(addrs_tr);
  temp = read_memory(addrs_temp);

  pinMode(6, INPUT);
  pinMode(7, INPUT);
  start_time = millis();

  // save_memory(addrs_kp, 9.6);
  // save_memory(addrs_ki, 20);
  // save_memory(addrs_kd, 5.44);
  // save_memory(addrs_temp, 29);
  // save_memory(addrs_ti, 100);
  // save_memory(addrs_tm, 20);
  // save_memory(addrs_tr, 100);
}

void loop()
{

  now = millis();

  read_temperature();
  delay(100);

  volatile Buttons event = GetButtons();
  if (event != EV_NONE)
  {
    state_machine(current_menu, event);
  }
  print_menu(scroll_menu);

  Serial.print(temp);
  Serial.print(" | ");
  Serial.println(temp_c);

  delay(300);
}

// FUNCTIONS

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

void afisare_runtime()
{
  unsigned long start_time_afisare = millis(); // Momentul de start al execuției funcției

  PID();
  int min = 0;
  int sec = 0;
  int remaining = 0;
  double setTemp = temp;

  unsigned long uptime = (now - start_time) / 1000;

  lcd.setCursor(0, 0);
  lcd.print("P:");
  lcd.print(temp);

  Serial.print("UPTIME: ");
  Serial.println(uptime);

  Serial.print("TI: ");
  Serial.println(ti);

  Serial.print("Tm: ");
  Serial.println(tm);

  Serial.print("Tr: ");
  Serial.println(tr);

  lcd.setCursor(0, 1);

  // Actualizează variabila globală pentru ultimul start time
  last_runtime_start_time = millis();
  unsigned long uptime_afisare = (millis() - start_time_afisare) / 1000; // Calculează uptime-ul pentru funcția curentă

  if (uptime <= ti)
  {
    lcd.print(" TInc=");
    remaining = ti - uptime;
    // temp = temp_c * (ti - remaining) / ti;
  }
  else if (uptime <= (ti + tm))
  {
    lcd.print(" TMen=");
    remaining = (ti + tm) - uptime;
  }
  else if (uptime <= (ti + tm + tr))
  {
    lcd.print(" tRac=");
    remaining = (ti + tm + tr) - uptime;
    temp = temp_c - temp_c * (tr - remaining) / tr;
  }
  else
  {
    lcd.print("Oprit: ");
    if ((millis() - last_runtime_start_time) >= 3000)
    {
      // Resetarea cronometrului
      last_runtime_start_time = millis();
    }
  }

  min = remaining / 60;
  sec = remaining % 60;
  lcd.print(min);
  lcd.print(":");
  lcd.print(sec);

  analogWrite(10, output);

  Serial.print("Time_afis: ");
  Serial.println(uptime_afisare);
}

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
    lcd.print("Timp Inc: ");
    lcd.print(ti);
    break;
  case MENU_TM:
    lcd.print("Timp Ment: ");

    lcd.print(tm);
    break;
  case MENU_TR:
    lcd.print("Timp Rac: ");

    lcd.print(tr);
    break;
  case MENU_TEMP:
    lcd.print("TEMP = ");
    lcd.print(temp);
    break;
  case MENU_RUNTIME:
    afisare_runtime();
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
