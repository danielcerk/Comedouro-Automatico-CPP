#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>

#define P 490
#define V 0.05
#define T 100
#define DIR_PIN 3
#define STEP_PIN 2
#define TIME 1000

#define T0 3000 // Atraso para mensagens informativas (3 segundos)
#define T1 1000 // Não usado no código, mas mantido das suas definições
#define T2 300  // Não usado no código, mas mantido das suas definições
#define T3 100  // Atraso de debounce para teclado/LCD

LiquidCrystal_I2C lcd(0x27, 16, 2);

char key;
int num;
int cont = 0; // Estado principal do menu
int contA = 0; // Não usado no código, mas mantido
int contB = 0; // Não usado no código, mas mantido
int dose = 5; // Configuração Padrão de doses por acionamento
int intervalo = 60; // Configuração Padrão de intervalo em minutos (60 minutos)

// Configura pinos do teclado numérico
const uint8_t row_size = 4;
const uint8_t col_size = 4;
const uint8_t row_pin[row_size] = {6, 7, 8, 9};
const uint8_t col_pin[col_size] = {10, 11, 12, 13};
const char keys[row_size][col_size] = {
    { '1', '2', '3', 'A' },
    { '4', '5', '6', 'B' },
    { '7', '8', '9', 'C' },
    { '*', '0', '#', 'D' }
};

Keypad keypad = Keypad(makeKeymap(keys), row_pin, col_pin, row_size, col_size);

// Pinos para o DS1302
const int IO_PIN = 4;
const int SCLK_PIN = 5;
const int CE_PIN = 2;

ThreeWire myWire(IO_PIN, SCLK_PIN, CE_PIN);
RtcDS1302<ThreeWire> Rtc(myWire);

RtcDateTime ultima_dose_momento; // Armazena a data e hora da última dose

void printDateTime(const RtcDateTime& dt);
void motor();
void rotate(int steps, float speed);
void rotateDeg(float deg, float speed);
void passo_positivo();
void passo_negativo();
void rolagem1();
void rolagem2();
void rolagem3();
void intro();


void setup() {
    Wire.begin();
    Serial.begin(9600);

    pinMode(DIR_PIN, OUTPUT);
    pinMode(STEP_PIN, OUTPUT);

    lcd.init();
    lcd.backlight();
    lcd.clear();

    // Inicializa o RTC
    Rtc.Begin();

    // Lógica de ajuste inicial do RTC (descomente APENAS NA PRIMEIRA VEZ ou se a hora estiver errada)
    /*
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    if(Rtc.GetIsWriteProtected()){
        Rtc.SetIsWriteProtected(false);
    }
    if(!Rtc.GetIsRunning()){
        Serial.println("RTC não está rodando. Iniciando e ajustando...");
        Rtc.SetIsRunning(true);
        Rtc.SetDateTime(compiled);
    } else {
        RtcDateTime now = Rtc.GetDateTime();
        if (now < compiled) {
            Serial.println("RTC desatualizado. Ajustando...");
            Rtc.SetDateTime(compiled);
        }
    }
    Serial.println("RTC Inicializado. Data/Hora atual:");
    printDateTime(Rtc.GetDateTime());
    Serial.println();
    */

    intro(); // Chama a introdução
}


void loop() {
    key = keypad.getKey();

    // -------------------------------------- MENU PRINCIPAL ---------------------------------------
    if (cont == 0) {
        lcd.setCursor(0, 0);
        lcd.print("A) Ligar Sistema");
        lcd.setCursor(0, 1);
        lcd.print("B) Configurar");

        if (key == 'A') {
            cont = 1; // Entra no estado de sistema ativo
            lcd.clear();
            // Define o momento da primeira dose como "agora" para começar a contagem
            ultima_dose_momento = Rtc.GetDateTime();
        } else if (key == 'B') {
            cont = 2; // Entra no estado de configuração
            lcd.clear();
        }
    }
    // -------------------------------------- SISTEMA ATIVO ---------------------------------------
    else if (cont == 1) {
        lcd.setCursor(0, 0);
        lcd.print("SISTEMA ATIVO!");
        lcd.setCursor(0, 1);
        lcd.print("#) Reiniciar");

        // Lógica de Dosagem Baseada no RTC
        RtcDateTime current_time = Rtc.GetDateTime();

        // Calcular a diferença em minutos
        // Esta biblioteca permite calcular a diferença em segundos diretamente
        long diff_seconds = current_time.getTotalSeconds() - ultima_dose_momento.getTotalSeconds();
        
        bool deve_dosar = false;

        if (intervalo == 6) { // Corresponde a 0.1 minutos (6 segundos)
            if (diff_seconds >= 6) {
                deve_dosar = true;
            }
        } else { // Para intervalos em minutos
            // Calcula a diferença em minutos inteiros
            long diff_minutes = diff_seconds / 60;
            if (diff_minutes >= intervalo) {
                deve_dosar = true;
            }
        }
        
        if (deve_dosar) {
            Serial.print("Dosando "); Serial.print(dose); Serial.println(" vezes...");
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("DOSANDO...");
            
            for (int x = 0; x < dose; x++) {
                motor(); // Aciona o motor
                Serial.print("Dose #"); Serial.println(x + 1);
                delay(T); // Pequeno delay entre as doses, se necessário
            }
            ultima_dose_momento = current_time; // Atualiza o momento da última dose
            lcd.clear(); // Limpa a mensagem "DOSANDO..."
            lcd.setCursor(0, 0); // Redesenha a mensagem do sistema ativo
            lcd.print("SISTEMA ATIVO!");
            lcd.setCursor(0, 1);
            lcd.print("#) Reiniciar");
        }

        if (key == '#') {
            Serial.println("Reiniciando...");
            cont = 0; // Volta para o menu principal
            lcd.clear();
        }

    }
    // -------------------------------------- MENU DOSAGEM & TEMPO ---------------------------------------
    else if (cont == 2) {
        lcd.setCursor(0, 0);
        lcd.print("C)Dosagem");
        lcd.setCursor(0, 1);
        lcd.print("D)Tempo #)Voltar");

        if (key == 'C') {
            cont = 3; // Entra no estado de configuração de doses
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Informe o ");
            lcd.setCursor(0, 1);
            lcd.print("numero de doses:");
            delay(T0);
            lcd.clear();
        } else if (key == 'D') {
            cont = 4; // Entra no estado de configuração de tempo
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Informe o tempo");
            lcd.setCursor(0, 1);
            lcd.print("em minutos:");
            delay(T0);
            lcd.clear();
        } else if (key == '#') {
            cont = 0; // Volta para o menu principal
            lcd.clear();
        }
    }
    // -------------------------------------- MENU OPCOES DOSES ---------------------------------------
    else if (cont == 3) {
        lcd.setCursor(0, 0);
        lcd.print("1)1  2)5  3)10");
        lcd.setCursor(0, 1);
        lcd.print("4)15  5)20  6)25");

        if (key != NO_KEY) {
            cont = 0; // Volta para o menu principal após seleção
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Selecionado:");
            lcd.setCursor(0, 1);

            if (key == '1') { dose = 1; lcd.print("1 dose."); }
            else if (key == '2') { dose = 5; lcd.print("5 doses."); }
            else if (key == '3') { dose = 10; lcd.print("10 doses."); }
            else if (key == '4') { dose = 15; lcd.print("15 doses."); }
            else if (key == '5') { dose = 20; lcd.print("20 doses."); }
            else if (key == '6') { dose = 25; lcd.print("25 doses."); }
            else if (key == '#') { cont = 0; lcd.clear(); } // Se #, volta sem mudar dose
            
            delay(T0);
            lcd.clear();
        }
    }
    // -------------------------------------- MENU OPCOES DE TEMPO ---------------------------------------
    else if (cont == 4) {
        lcd.setCursor(0, 0);
        lcd.print("1)0.1  2)1  3)5");
        lcd.setCursor(0, 1);
        lcd.print("4)10 5)30 6)60");

        if (key != NO_KEY) {
            cont = 0; // Volta para o menu principal após seleção
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Selecionado:");
            lcd.setCursor(0, 1);

            if (key == '1') { intervalo = 6; lcd.print("6 segundos."); } // 0.1 min = 6 segundos
            else if (key == '2') { intervalo = 1; lcd.print("1 minuto."); }
            else if (key == '3') { intervalo = 5; lcd.print("5 minutos."); }
            else if (key == '4') { intervalo = 10; lcd.print("10 minutos."); }
            else if (key == '5') { intervalo = 30; lcd.print("30 minutos."); }
            else if (key == '6') { intervalo = 60; lcd.print("60 minutos."); }
            else if (key == '#') { cont = 0; lcd.clear(); } // Se #, volta sem mudar intervalo

            delay(T0);
            lcd.clear();
        }
    }

    // Atraso para evitar múltiplas leituras de tecla em um ciclo muito rápido
    // Mantenha este atraso pequeno para que o sistema responda rapidamente
    delay(T3);
}

// -------------------------------------- FUNÇÕES AUXILIARES ---------------------------------------

void printDateTime(const RtcDateTime& dt){
    char datestring[20];
    snprintf_P(datestring, 
              sizeof(datestring),
              PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
              dt.Day(), dt.Month(), dt.Year(),
              dt.Hour(), dt.Minute(), dt.Second());
    Serial.print(datestring);
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

void rolagem1() {
    String texto = "Alimentador automatizado para animais domesticos ";
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
    String texto = "Daniel Gomes.";
    int tamanho = texto.length();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Alunos:");

    lcd.setCursor(0, 1);
    lcd.print(texto);
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
        } else {
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