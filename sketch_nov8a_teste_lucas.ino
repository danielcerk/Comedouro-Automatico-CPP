// Configurar WIFI ( ESP01 )
// Adicionar EEPROM
// Configurar Relé


// Inicialização

// Ele deverá se a configuração existe, se não, apresenta mensagem de erro
// Após isso ele deverá verificar minuto por minuto ...

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>
#include <EEPROM.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>

#define P 490
#define V 0.05
#define T 100
#define DIR_PIN 3
#define STEP_PIN 2
#define TIME 1000

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
#define DAT_PIN 11
#define CLK_PIN 10
#define RST_PIN 12

ThreeWire myWire(DAT_PIN, CLK_PIN, RST_PIN);
RtcDS1302<ThreeWire> Rtc(myWire);

int ciclosDia = 0;
int horarios[10];
int cicloAtual = 0;
bool inicializado = false;
unsigned long ultimoCheck = 0;


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

int selected = 0;
const int totalOptions = 3;
int inAction = -1;

void setup() {

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setCursor((128 - 6 * strlen("Carregando...")) / 2, 20);
  display.println("Carregando...");
  display.display();

  Rtc.Begin();

  if (!Rtc.GetIsRunning() || !Rtc.IsDateTimeValid()) {
    Rtc.SetIsRunning(true);
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    Rtc.SetDateTime(compiled);
  }


  delay(1000);
}

void loop() {

  char key = keypad.getKey();

  if (key) {

    if (inAction == -1) {

      if (key == '8') {

        selected++;
        if (selected >= totalOptions) selected = 0;

      } else if (key == '2') {

        selected--;
        if (selected < 0) selected = totalOptions - 1;

      } else if (key == '#') {

        inAction = selected;

      }

    } else {

      if (key == '*') {

        inAction = -1;

      }

    }

  }

  if (inAction == -1) {

    drawMenu();

  } else {

    drawAction(inAction);

  }

  delay(100);
}

void drawMenu() {

  display.clearDisplay();

  const char* options[totalOptions] = {"Inicializar", "Configurar", "Reiniciar"};

  RtcDateTime now = Rtc.GetDateTime();
  printDateTime(now);

  for (int i = 0; i < totalOptions; i++) {
    int y = 16 + i * 12;

    if (i == selected) {
      display.setCursor(0, y);
      display.print("> ");
    } else {
      display.setCursor(0, y);
      display.print("  ");
    }

    display.print(options[i]);
  }

  display.display();
}


void drawAction(int action) {

  display.clearDisplay();
  display.setCursor(0, 0);

  switch (action) {

    case 0:

      inicializar();
      break;

    case 1:

      configurar();
      break;

    case 2:

      reiniciar();
      break;

  }

  display.display();

}

void inicializar() {

  if (!inicializado) {

    int valorExistente = EEPROM.read(0);

    display.clearDisplay();
    display.setCursor(0, 0);

    if (valorExistente > 0 && valorExistente < 10) {

      display.println("SISTEMA ATIVO");
      ciclosDia = EEPROM.read(0);

      for (int i = 0; i < ciclosDia; i++) {
        int hora = EEPROM.read(1 + i * 2);
        int minuto = EEPROM.read(1 + i * 2 + 1);
        horarios[i] = hora * 100 + minuto;
      }

      cicloAtual = 0;
      ultimoCheck = millis();
      inicializado = true;

    } else {

      display.println("CONFIGURE ANTES DE INICIAR");
      display.println("1 = Configurar");
      display.println("* = Sair");

      char k = keypad.getKey();
      if (k == '1') inAction = 1;
      else if (k == '*') inAction = -1;

    }

    display.display();

  } else {

    RtcDateTime now = Rtc.GetDateTime();
    int agora = now.Hour() * 100 + now.Minute();

    /*if (cicloAtual < ciclosDia && agora >= horarios[cicloAtual] && millis() - ultimoCheck >= 60000) {
      motor();
      cicloAtual++;
      ultimoCheck = millis();
    }*/

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("SISTEMA ATIVO");

    /*if (cicloAtual < ciclosDia) {
      int prox = horarios[cicloAtual];
      int hh = prox / 100;
      int mm = prox % 100;
      display.print("Prox dosagem: ");
      if (mm < 10) display.print(hh), display.print(":0"), display.println(mm);
      else display.print(hh), display.print(":"), display.println(mm);
      display.print("Faltam ");
      display.println(ciclosDia - cicloAtual);
      display.println(" ciclos");
    } else {
      display.println("Todos ciclos completos");
    }*/

    char k = keypad.getKey();
    if (k == '*') {
      inAction = -1;
      inicializado = false;
    }

    display.display();

  }

}
  

void configurar() {

  int valorExistente = EEPROM.read(0);

  if (valorExistente > 0 && valorExistente < 10) {

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Ja configurado");
    display.println("Configurar novamente?");
    display.println("1=Sim   *=Voltar");
    display.display();

    while (true) {
      char k = keypad.getKey();
      if (!k) continue;
      if (k == '1') break;
      if (k == '*') { inAction = -1; return; }
    }
  }

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("CONFIGURAR");
  display.setCursor(0, 56);
  display.println("* Voltar");
  display.display();

  int ciclosDia = -1;
  int doses[5];
  int ciclosHelice = -1;

  ciclosDia = readNumberFromKeypad("Quantos ciclos/dia?");
  if (ciclosDia == -1) { inAction = -1; return; }
  EEPROM.write(0, ciclosDia);

  for (int i = 0; i < ciclosDia; i++) {

    char msg[20];
    sprintf(msg, "Horario %d (HHMM):", i + 1);

    int horario = readNumberFromKeypad(msg);
    if (horario == -1) { inAction = -1; return; }

    EEPROM.write(1 + i * 2,   horario / 100);
    EEPROM.write(1 + i * 2 + 1, horario % 100);

    doses[i] = horario;
  }

  ciclosHelice = readNumberFromKeypad("Qtde de doses?");
  if (ciclosHelice == -1) { inAction = -1; return; }
  EEPROM.write(20, ciclosHelice);

  display.clearDisplay();
  display.setCursor(0, 20);
  display.println("Configurado!");
  display.setCursor(0, 56);
  display.println("* Voltar");
  display.display();

  delay(1500);
  inAction = -1;
}

int readNumberFromKeypad(const char* mensagem) {

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(mensagem);
  display.println("Digite e pressione #");
  display.setCursor(0, 56);
  display.println("* Voltar");
  display.display();

  char number[5] = "";
  int length = 0;

  while (true) {

    char key = keypad.getKey();
    if (!key) continue;

    if (key >= '0' && key <= '9') {
      if (length < 4) {
        number[length] = key;
        length++;
        number[length] = '\0';

        display.clearDisplay();
        display.setCursor(0, 0);
        display.println(mensagem);
        display.println("Digite e pressione #");
        display.setCursor(0, 24);

        if (length <= 2) {
          display.print(number);
        } else {
          display.print(number[0]);
          display.print(number[1]);
          display.print(":");
          display.print(number[2]);
          if (length == 4) display.print(number[3]);
        }

        display.setCursor(0, 56);
        display.println("* Voltar");
        display.display();
      }
    }

    if (key == '#') {
      if (length > 0) return atoi(number);
    }

    if (key == '*') {
      return -1;
    }
  }
}




void reiniciar() {

  display.setCursor((128 - 6 * strlen("Reiniciando...")) / 2, 20);
  display.print("Reiniciando...");
  display.display();

  for (int i = 0; i < EEPROM.length(); i++) {

    EEPROM.write(i, 0);

  }

  asm volatile ("jmp 0");

}

void printDateTime(const RtcDateTime& dt) {

  char buffer[20];

  snprintf_P(buffer, 
    sizeof(buffer),
    PSTR("%02u/%02u/%04u %02u:%02u"),
    dt.Day(), dt.Month(), dt.Year(),
    dt.Hour(), dt.Minute());

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print(buffer);

}

void motor() {
    passo_positivo();
    delay(T);
    passo_negativo();
    delay(T);
}

void rotate(int steps, float speed) {
    int dir = (steps > 0) ? HIGH : LOW;
    steps = abs(steps);
    digitalWrite(DIR_PIN, dir);
    float usDelay = (1 / speed) * 70;

    for (int i = 0; i < steps; i++) {
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(usDelay);
        digitalWrite(STEP_PIN, LOW);
        delayMicroseconds(usDelay);
    }
}

void rotateDeg(float deg, float speed) {
    int dir = (deg > 0) ? HIGH : LOW;
    digitalWrite(DIR_PIN, dir);
    int steps = abs(deg) * (1 / 0.225);
    float usDelay = (1 / speed) * 70;

    for (int i = 0; i < steps; i++) {
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(usDelay);
        digitalWrite(STEP_PIN, LOW);
        delayMicroseconds(usDelay);
    }
}

void passo_positivo() {
    rotate(P, V);
}

void passo_negativo() {
    rotate(-P, V);
}

