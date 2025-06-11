#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define P 490
#define V 0.05
#define T 100
#define DIR_PIN 3
#define STEP_PIN 2
#define TIME 1000

#define T0 3000
#define T1 1000
#define T2 300
#define T3 100

LiquidCrystal_I2C lcd(0x27, 16, 2);

char key;
int num;
int cont = 0;
int contA = 0;
int contB = 0;
int dose = 0;
int intervalo = 6000;
int estadoMenu = 0;

//Configura pinos do teclado numerico
const uint8_t row_size = 4;
const uint8_t col_size = 4;
const uint8_t row_pin[row_size] = {6,7,8,9};
const uint8_t col_pin[col_size] = {10, 11, 12, 13};
const char keys[row_size][col_size] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

Keypad keypad = Keypad(makeKeymap(keys), row_pin, col_pin, row_size, col_size);

void setup() {

  lcd.clear();

  Serial.begin(9600);

  pinMode(DIR_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);

  lcd.init();
  lcd.backlight();
  lcd.clear();

  intro();

}


void loop() {

  key = keypad.getKey();

  lcd.clear();

  while (cont == 0) {

    inicio:

    key = keypad.getKey();

    lcd.setCursor(0, 0);
    lcd.print("A) Ligar Sistema");
    lcd.setCursor(0, 1);
    lcd.print("B) Configurar");

    // -------------------------------------- MENU SISTEMA LIGADO ---------------------------------------

    if (key == 'A') {

      cont = 1;

      // -------------------------------------- MENU LIGAR SISTEMA SEM ANTES CONFIGURAR ---------------------------------------

      if (cont == 1) {

        if (dose == 0 || intervalo == 0) {

            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Configure antes");
            lcd.setCursor(0, 1);
            lcd.print("de iniciar!");

            delay(T0);

            lcd.clear();

            key = keypad.getKey();

            cont = 0;

        } else {

            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("SISTEMA ATIVO!");
            lcd.setCursor(0, 1);
            lcd.print("#) Reiniciar");

            int doses = dose;

            while ( true ){

                for (int i = 0; i < doses; i++) {
                    
                    motor();
                    delay(3000);

                }

                delay(intervalo);

            }

        }

      } 

    }

    // --------------------------------------------- MENU DOSAGEM & TEMPO-----------------------------------------------

    if (key == 'B') {

      cont = 2;

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("C)Dosagem");
      lcd.setCursor(0, 1);
      lcd.print("D)Tempo #)Voltar");

    }

    delay(T3);
  }

  // -------------------------------------- MENU "INFORME DOSE" - "INFORME TEMPO" ---------------------------------------

  while (cont == 2) {

    key = keypad.getKey();

    if (key == 'C') {

      cont = 3;

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Informe o ");
      lcd.setCursor(0, 1);
      lcd.print("numero de doses:");

      delay(T0);

      lcd.clear();

    }

    if (key == 'D') {

      cont = 4;

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Informe o tempo");
      lcd.setCursor(0, 1);
      lcd.print("em minutos:");

      delay(T0);

      lcd.clear();

    }

    if ( key == '#'){

      cont=0;
      lcd.clear();

      break;

    }
    
    delay(T3);
  }

  // -------------------------------------- MENU OPCOES DOSES ---------------------------------------

  while (cont == 3) {

    lcd.setCursor(0, 0);
    lcd.print("1)1  2)5  3)10");
    lcd.setCursor(0, 1);
    lcd.print("4)15  5)20  6)25");

    key = keypad.getKey();

    if (key != NO_KEY) {

      cont = 0;

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Selecionado:");
      lcd.setCursor(0, 1);

      if (key == '1') {

        dose = 1;
        lcd.print("1 dose.");

      }

      if (key == '2') {
        
        dose = 5;
        lcd.print("5 doses.");

      }

      if (key == '3') {

        dose = 10;
        lcd.print("10 doses.");

      }

      if (key == '4') {

        dose = 15;
        lcd.print("15 doses.");

      }

      if (key == '5') {

        dose = 20;
        lcd.print("20 doses.");

      }

      if (key == '6') {

        dose = 25;
        lcd.print("25 doses.");

      }

      if ( key == '#'){

        cont=0;
        lcd.clear();

        break;

      }

      delay(T0);
      lcd.clear();

    }

    delay(T3);

  }

  // -------------------------------------- MENU OPCOES DE TEMPO ---------------------------------------

  while (cont == 4) {

    lcd.setCursor(0, 0);
    lcd.print("1)0.1  2)1  3)5");
    lcd.setCursor(0, 1);
    lcd.print("4)10 5)30 6)60");

    key = keypad.getKey();

    if (key != NO_KEY) {

      cont = 0;

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Selecionado:");
      lcd.setCursor(0, 1);

      if (key == '1') {

        intervalo = 6000;
        lcd.print("6 segundos.");

      }

      if (key == '2') {

        intervalo = 60000;
        lcd.print("1 minuto.");

      }

      if (key == '3') {

        intervalo = 300000;
        lcd.print("5 minutos.");

      }

      if (key == '4') {

        intervalo = 600000;
        lcd.print("10 minutos.");

      }

      if (key == '5') {

        intervalo = 1800000;
        lcd.print("30 minutos.");

      }

      if (key == '6') {

        intervalo = 3600000;
        lcd.print("60 minutos.");

      }

      if ( key == '#'){

        cont=0;
        lcd.clear();

        break;

      }

      delay(T0);
      lcd.clear();

    }

    delay(T3);
  }
}

// -------------------------------------- MENSAGENS INICIAS (EQUIPE - FACULDADE - ETC) ---------------------------------------

void rolagem1() {

  String texto = "Alimentador automatizado para animais domesticos  ";
  int tamanho = texto.length();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Projeto:");

  for (int i = 0; i < tamanho - 15; i++) {
    lcd.setCursor(0, 1);
    lcd.print(texto.substring(i, i + 16));
    delay(300);
  }

  delay(1000);

}


void rolagem2() {

  String texto = "Daniel Gomes, Edson Silva, Gabriel Barreto, Kaua Pinheiro.";
  int tamanho = texto.length();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Alunos:");

  for (int i = 0; i < tamanho - 15; i++) {

    lcd.setCursor(0, 1);
    lcd.print(texto.substring(i, i + 16));
    delay(300);

  }

  delay(1000);

}

void rolagem3() {

  String textoDisciplina = "Disciplina: Prototipagem e Desenvolvimento de Produtos Aplicados a Domotica";
  int tamanhoDisciplina = textoDisciplina.length();

  String textoProfessorNome = "Professor: Lucas Torres";
  int tamanhoProfessorNome = textoProfessorNome.length();

  lcd.clear();

  for (int i = 0; i < tamanhoDisciplina - 15; i++) {
    
    lcd.setCursor(0, 0);
    lcd.print(textoDisciplina.substring(i, i + 16));

    lcd.setCursor(0, 1);

    if (i > tamanhoProfessorNome - 16) {

      lcd.print("                ");

    } else{

      lcd.print(textoProfessorNome.substring(i, i + 16));

    }
    

    delay(300);

  }

  delay(1000);
}

void intro() {

  rolagem1();
  rolagem2();
  rolagem3();

  String textoProjeto = "Projeto de Extensao";
  int tamanhoProjeto = textoProjeto.length();

  String textoUniversidadeNome = "Universidade Salvador - UNIFACS";
  int tamanhoUniversidadeNome = textoUniversidadeNome.length();

  lcd.clear();

  int maxRolagem = max(tamanhoProjeto, tamanhoUniversidadeNome) - 15;

  for (int i = 0; i <= maxRolagem; i++) {

    lcd.setCursor(0, 0);
    if (i > tamanhoProjeto - 16) {
      lcd.print("                ");
    } else {
      lcd.print(textoProjeto.substring(i, i + 16));
    }

    lcd.setCursor(0, 1);
    if (i > tamanhoUniversidadeNome - 16) {
      lcd.print("                ");
    } else {
      lcd.print(textoUniversidadeNome.substring(i, i + 16));
    }

    delay(300);
  }

  delay(1000);

}


// -------------------------------------- ACIONAMENTO MOTOR DE PASSO ---------------------------------------

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
