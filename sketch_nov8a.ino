// Configurar WIFI ( ESP01 )
// Adicionar EEPROM
// Configurar Relé
// Criar App para comunicação com o circuito
// Criar funções Inicializar e Configurar em .h para importação

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include <Keypad.h>
#include <EEPROM.h>

#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_ADDR 0x3C

#define DAT_PIN 11
#define CLK_PIN 10
#define RST_PIN 12

#define sizeEEPROM 1000

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
ThreeWire myWire(DAT_PIN, CLK_PIN, RST_PIN);
RtcDS1302<ThreeWire> Rtc(myWire);
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);

char key;
int selected = 0;

const char* menuOptions[] = {
  "Inicializar",
  "Configurar",
  "Reiniciar"
};
const int totalOptions = sizeof(menuOptions) / sizeof(menuOptions[0]);

void setup() {

  Serial.begin(9600);
  Rtc.Begin();

  if (!Rtc.GetIsRunning()) {

    Rtc.SetIsRunning(true);
    Rtc.SetDateTime(RtcDateTime(__DATE__, __TIME__));

  }

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {

    Serial.println("Falha ao inicializar display OLED.");
    for (;;);

  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor((128 - 6 * strlen("Inicializando...")) / 2, 20);
  display.println("Inicializando...");
  display.display();
  delay(1500);

}

void loop() {

  key = keypad.getKey();

  if (key) {

    if (key == '8') {

      selected++;
      if (selected >= totalOptions) selected = 0;

    }

    else if (key == '2') {

      selected--;
      if (selected < 0) selected = totalOptions - 1;

    }

    else if (key == '#') {
      
      if (menuOptions[selected] == "Inicializar"){

        // Consultar banco de dados para saber se os dados de configuração existem, se sim, iniciar
        // Inicializar, deverá executar o processo de dosagem, verificando minuto por minuto e acompanhado por um botão de sair
        // Deverá mostrar o horário da próxima dosagem
        // Ao fazer os ciclos, ele irá contar , por exemplo, falta 6 ciclos para completar a dosagem

        display.clearDisplay();

        Serial.print("Inicializar");

      } else if (menuOptions[selected] == "Configurar") {

        // Acessa configurar
        // Configurar quantas doses diárias, se for 2, digite o 1 horário, digite o segundo, quantos ciclos

        display.clearDisplay();

        Serial.print("Configurar");

      } else if (menuOptions[selected] == "Reiniciar") {

        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor((128 - 6 * strlen("Reiniciando...")) / 2, 20);
        display.print("Reiniciando...");
        display.display();
        delay(1000);

        // Resetar todos os dados da memória

        for (int nL = 0; nL < sizeEEPROM; nL++) {

          EEPROM.write(nL, 0);

        }

        EEPROM.end();

        // Reiniciar o Arduino
        asm volatile("jmp 0");
      }
      
    }

  }

  RtcDateTime now = Rtc.GetDateTime();

  char buffer[20];

  snprintf(buffer, sizeof(buffer), "%02u/%02u/%04u %02u:%02u",
           now.Day(), now.Month(), now.Year(),
           now.Hour(), now.Minute());

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print(buffer);

  for (int i = 0; i < totalOptions; i++) {

    int y = 24 + i * 12;

    if (i == selected) {

      display.setCursor(0, y);
      display.print("> ");

    } else {

      display.setCursor(0, y);
      display.print("  ");

    }

    display.print(menuOptions[i]);
  }

  display.display();
  delay(100);
}

