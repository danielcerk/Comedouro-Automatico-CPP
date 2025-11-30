#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include <EEPROM.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define DAT_PIN 11
#define CLK_PIN 10
#define RST_PIN 12
ThreeWire myWire(DAT_PIN, CLK_PIN, RST_PIN);
RtcDS1302<ThreeWire> Rtc(myWire);

#define RELE 13

const uint8_t X1 = 2;
const uint8_t X2 = 3;
const uint8_t X3 = 4;
const uint8_t X4 = 5;
const uint8_t X5 = 6;
const uint8_t X6 = 7;
const uint8_t X7 = 8;

const byte row_size = 4;
const byte col_size = 3;

const uint8_t row_pin[4] = { X2, X7, X6, X4 };
const uint8_t col_pin[3] = { X3, X1, X5 };

const char keys[4][3] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
Keypad keypad = Keypad(makeKeymap(keys), row_pin, col_pin, row_size, col_size);

int ciclosDia = 0;
int horarios[10];
int doses = 0;

int menuIndex = 0;
bool emAcao = false;
int cicloAtual = 0;
bool iniciado = false;
unsigned long ultimoMinCheck = 0;

void salvarEEPROM() {
  EEPROM.update(0, ciclosDia);
  EEPROM.update(1, doses);
  int addr = 10;
  for (int i = 0; i < ciclosDia; i++) {
    EEPROM.put(addr, horarios[i]);
    addr += sizeof(int);
  }
}

void carregarEEPROM() {
  ciclosDia = EEPROM.read(0);
  doses = EEPROM.read(1);
  int addr = 10;
  for (int i = 0; i < ciclosDia; i++) {
    EEPROM.get(addr, horarios[i]);
    addr += sizeof(int);
  }
}

void setup() {
  pinMode(RELE, OUTPUT);
  digitalWrite(RELE, LOW);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,20);
  display.println("Inicializando...");
  display.display();

  Rtc.Begin();
  if (!Rtc.GetIsRunning() || !Rtc.IsDateTimeValid()) {
    Rtc.SetIsRunning(true);
    Rtc.SetDateTime(RtcDateTime(__DATE__, __TIME__));
  }

  carregarEEPROM();
  delay(800);
}

void loop() {
  if (!emAcao) menu();
  else {
    if (menuIndex == 0) inicializar();
    if (menuIndex == 1) configurar();
    if (menuIndex == 2) reiniciar();
  }
}

void menu() {
  RtcDateTime now = Rtc.GetDateTime();

  int hh = now.Hour();
  int mm = now.Minute();
  int dd = now.Day();
  int mo = now.Month();
  int yy = now.Year();

  char key = keypad.getKey();

  if (key == '8') {
    menuIndex++;
    if (menuIndex > 2) menuIndex = 0;
  }

  if (key == '2') {
    menuIndex--;
    if (menuIndex < 0) menuIndex = 2;
  }

  if (key == '#') emAcao = true;

  display.clearDisplay();

  display.setCursor(0,0);
  if (hh < 10) display.print("0");
  display.print(hh);
  display.print(":");
  if (mm < 10) display.print("0");
  display.print(mm);
  display.print("  ");
  if (dd < 10) display.print("0");
  display.print(dd);
  display.print("/");
  if (mo < 10) display.print("0");
  display.print(mo);
  display.print("/");
  display.print(yy);

  display.setCursor(0,16);
  display.println("Menu:");

  display.setCursor(0,32);
  if (menuIndex == 0) display.print("> "); else display.print("  ");
  display.println("Inicializar");

  display.setCursor(0,44);
  if (menuIndex == 1) display.print("> "); else display.print("  ");
  display.println("Configurar");

  display.setCursor(0,56);
  if (menuIndex == 2) display.print("> "); else display.print("  ");
  display.println("Reiniciar");

  display.display();
}

bool deveSair() {
  char k = keypad.getKey();
  if (k == '*') {
    emAcao = false;
    iniciado = false;
    digitalWrite(RELE, LOW);
    return true;
  }
  return false;
}

void inicializar() {
  if (deveSair()) return;

  if (ciclosDia == 0 || doses == 0) {
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("Nao configurado");
    display.setCursor(0,56);
    display.println("* Voltar");
    display.display();
    if (keypad.getKey() == '*') { emAcao = false; iniciado = false; }
    return;
  }

  if (!iniciado) {
    iniciado = true;
    cicloAtual = 0;
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("Sistema ativo");
    display.setCursor(0,56);
    display.println("* Voltar");
    display.display();
    delay(500);
    if (deveSair()) return;
  }

  if (millis() - ultimoMinCheck < 60000) {
    if (deveSair()) return;
    return;
  }

  ultimoMinCheck = millis();

  if (cicloAtual >= ciclosDia) {
    if (deveSair()) return;
    return;
  }

  RtcDateTime now = Rtc.GetDateTime();

  int agora = now.Hour()*100 + now.Minute();
  int alvo = horarios[cicloAtual];

  if (deveSair()) return;

  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Proximo ciclo:");

  int hh = alvo / 100;
  int mm = alvo % 100;

  display.setCursor(0,16);
  if (hh < 10) display.print("0");
  display.print(hh);
  display.print(":");
  if (mm < 10) display.print("0");
  display.print(mm);

  display.setCursor(0,56);
  display.println("* Voltar");
  display.display();

  if (agora >= alvo) {
    executarDoses();
    cicloAtual++;
  }
}

void configurar() {
  if (ciclosDia > 0 && doses > 0) {
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("Configurar?");
    display.println("# Reconfigurar");
    display.println("* Voltar");
    display.display();
    while (true) {
      char k = keypad.getKey();
      if (k == '#') break;
      if (k == '*') { emAcao = false; return; }
    }
  }

  ciclosDia = lerNumero("Ciclos por dia:");
  if (ciclosDia <= 0) { emAcao = false; return; }

  for (int i = 0; i < ciclosDia; i++) {
    while (true) {
      char txt[20];
      sprintf(txt, "Horario %d:", i + 1);
      int h = lerNumero(txt);
      if (i == 0) {
        if (validarHorario(0, h)) { horarios[i] = h; break; }
      } else {
        if (validarHorario(horarios[i - 1], h)) { horarios[i] = h; break; }
      }
      display.clearDisplay();
      display.setCursor(0,0);
      display.println("Horario invalido");
      display.println("* repetir");
      display.display();
      while (keypad.getKey() != '*');
    }
  }

  doses = lerNumero("Doses:");

  salvarEEPROM();

  emAcao = false;
}

bool validarHorario(int anterior, int atual) {
  int hh = atual / 100;
  int mm = atual % 100;
  if (hh < 0 || hh > 23) return false;
  if (mm < 0 || mm > 59) return false;
  if (anterior <= 0) return true;
  int ah = anterior / 100;
  int am = anterior % 100;
  if (hh > ah) return true;
  if (hh == ah && mm > am) return true;
  return false;
}

int lerNumero(const char* titulo) {
  bool ehHorario = strstr(titulo, "Horario") != NULL;
  display.clearDisplay();
  display.setCursor(0,0);
  display.println(titulo);
  display.println("Digite e #");
  display.display();
  char buff[6];
  int len = 0;
  while (true) {
    char k = keypad.getKey();
    if (k >= '0' && k <= '9') {
      if (len < (ehHorario ? 4 : 5)) {
        buff[len++] = k;
        buff[len] = '\0';
        display.clearDisplay();
        display.setCursor(0,0);
        display.println(titulo);
        if (ehHorario) {
          if (len == 1) {
            display.print("0");
            display.print(buff[0]);
            display.print(":__");
          } else if (len == 2) {
            display.print(buff[0]);
            display.print(buff[1]);
            display.print(":__");
          } else if (len == 3) {
            display.print(buff[0]);
            display.print(buff[1]);
            display.print(":");
            display.print("0");
            display.print(buff[2]);
          } else if (len == 4) {
            display.print(buff[0]);
            display.print(buff[1]);
            display.print(":");
            display.print(buff[2]);
            display.print(buff[3]);
          }
        } else display.print(buff);
        display.display();
      }
    }
    if (k == '#') {
      if (len == 0) continue;
      if (!ehHorario) return atoi(buff);
      int value = 0;
      if (len == 1) {
        int h = buff[0] - '0';
        value = h * 100;
      } else if (len == 2) {
        int h = (buff[0]-'0')*10 + (buff[1]-'0');
        value = h * 100;
      } else if (len == 3) {
        int h = (buff[0]-'0')*10 + (buff[1]-'0');
        int m = (buff[2]-'0');
        value = h * 100 + m;
      } else {
        int h = (buff[0]-'0')*10 + (buff[1]-'0');
        int m = (buff[2]-'0')*10 + (buff[3]-'0');
        value = h * 100 + m;
      }
      return value;
    }
    if (k == '*') return -1;
  }
}

void reiniciar() {

  for (int i = 0; i < EEPROM.length(); i++) {

    EEPROM.write(i, 0);

  }

  display.clearDisplay();
  display.setCursor(0,20);
  display.println("Reiniciando...");
  display.display();
  delay(1000);

  asm volatile ("jmp 0");
  
}
void executarDoses() {
  for (int i = 0; i < doses; i++) {
    if (deveSair()) return;
    display.clearDisplay();
    display.setCursor(0,0);
    display.print("Fazendo dosagem:");
    display.setCursor(0,16);
    display.print(i + 1);
    display.print("/");
    display.print(doses);
    display.setCursor(0,56);
    display.println("* para sair");
    display.display();
    digitalWrite(RELE, HIGH);
    for (int t = 0; t < 1000; t += 50) {
      if (deveSair()) return;
      delay(50);
    }
    digitalWrite(RELE, LOW);
    for (int t = 0; t < 800; t += 50) {
      if (deveSair()) return;
      delay(50);
    }
  }
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("OK");
  display.display();
  delay(500);
}
