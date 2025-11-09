// Configurar WIFI ( ESP01 )
// Adicionar EEPROM
// Configurar Relé
// Criar App para comunicação com o circuito

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>
#include <EEPROM.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
#define DAT_PIN 11
#define CLK_PIN 10
#define RST_PIN 12

ThreeWire myWire(DAT_PIN, CLK_PIN, RST_PIN);
RtcDS1302<ThreeWire> Rtc(myWire);

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

  RtcDateTime now = Rtc.GetDateTime();

  printDateTime(now);

  const char* options[totalOptions] = {"Inicializar", "Configurar", "Reiniciar"};

  for (int i = 0; i < totalOptions; i++) {

    int y = 24 + i * 12;

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

  // Consultar dados na memória para saber se os dados de configuração existem, se sim, iniciar
  // Inicializar, deverá executar o processo de dosagem, verificando minuto por minuto e acompanhado por um botão de sair
  // Deverá mostrar o horário da próxima dosagem
  // Ao fazer os ciclos, ele irá contar , por exemplo, falta 6 ciclos para completar a dosagem

  display.println("\n* para voltar");
  display.display();
  
}

void configurar() {

  // Acessa configurar
  // Configurar quantas doses diárias, se for 2, digite o 1 horário, digite o segundo, quantos ciclos

  display.println("\n* para voltar");
  display.display();

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

