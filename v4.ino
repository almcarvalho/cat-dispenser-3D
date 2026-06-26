//============================================================
// DISPENSADOR DE RAÇÃO
// Autor: Lucas Carvalho
// Data: 25/06/2026
//
// Funcionalidades:
// - Relé para acionamento do dispensador
// - Servo para abrir/fechar a portinha
// - Sensor TCRT5000 para contar a quantidade de ração dispensada
// - Buzzer de aviso
//============================================================

#include <Servo.h>

Servo servoPortinha;

// ---------------- PINOS ----------------
const int PINO_RELE = 9;
const int PINO_SERVO_PORTINHA = 10;
const int PINO_TCRT5000 = 2;
const int PINO_BUZZER = 8;

// ---------------- CONFIGURAÇÕES ----------------

// Alguns módulos de relé utilizam LOW para ligar.
// Caso o seu funcione invertido basta trocar HIGH por LOW.
const int RELE_LIGADO = HIGH;
const int RELE_DESLIGADO = LOW;

const int PORTINHA_FECHADA = 90;
const int PORTINHA_ABERTA = 180;

// Quantidade de refeições por dia
const int DISPENSAS_POR_DIA = 3;

// Tempo que o relé ficará ligado
const unsigned long TEMPO_RELE_MS = 60000UL;   // 1 minuto

// Quantidade mínima aceitável de detecções
const int MINIMO_ACEITAVEL = 1;

// Se detectar invertido troque LOW por HIGH
const int SENSOR_DETECTADO = LOW;

// Intervalo automático entre dispensas
const unsigned long INTERVALO_ENTRE_DISPENSAS =
    (24UL * 60UL * 60UL * 1000UL) / DISPENSAS_POR_DIA;

//-----------------------------------------------------------
// INTERVALO ENTRE DISPENSAS
//
// 1 dispensa/dia  = 24h00min
// 2 dispensas/dia = 12h00min
// 3 dispensas/dia = 08h00min  <-- configuração atual
// 4 dispensas/dia = 06h00min
// 5 dispensas/dia = 04h48min
//-----------------------------------------------------------

// ---------------- VARIÁVEIS ----------------

unsigned long ultimoAcionamento = 0;
unsigned long ultimoBipRacaoBaixa = 0;

bool racaoBaixa = false;

//============================================================
// BUZZER
//============================================================

void tocar(int frequencia, int duracao) {
  tone(PINO_BUZZER, frequencia);
  delay(duracao);
  noTone(PINO_BUZZER);
}

void bipLigado() {
  tocar(1000, 100);
  delay(100);
  tocar(2000, 300);
}

void bipAntesAcionar() {

  for (int i = 0; i < 3; i++) {
    tocar(2500, 700);
    delay(300);
  }

}

void bipFinal() {
  tocar(3000, 150);
}

void alertaRacaoBaixa() {

  tocar(800, 200);
  delay(100);
  tocar(600, 200);

}

//============================================================
// SENSOR TCRT5000
//============================================================

int medirRacaoDuranteDispensa() {

  int totalDeteccoes = 0;

  int estadoAnterior = digitalRead(PINO_TCRT5000);

  unsigned long inicio = millis();

  while (millis() - inicio < TEMPO_RELE_MS) {

    int estadoAtual = digitalRead(PINO_TCRT5000);

    if (estadoAtual == SENSOR_DETECTADO &&
        estadoAnterior != SENSOR_DETECTADO) {

      totalDeteccoes++;

      Serial.print("Deteccao #");
      Serial.println(totalDeteccoes);

    }

    estadoAnterior = estadoAtual;

    delay(5);
  }

  return totalDeteccoes;
}

//============================================================
// DISPENSA
//============================================================

void acionar() {

  Serial.println();
  Serial.println("================================");
  Serial.println("Preparando dispensa");

  bipAntesAcionar();

  servoPortinha.write(PORTINHA_ABERTA);

  Serial.println("Portinha aberta");

  delay(2000);

  Serial.println("Rele ligado");

  digitalWrite(PINO_RELE, RELE_LIGADO);

  int quantidadeDetectada = medirRacaoDuranteDispensa();

  digitalWrite(PINO_RELE, RELE_DESLIGADO);

  Serial.println("Rele desligado");

  delay(2000);

  servoPortinha.write(PORTINHA_FECHADA);

  Serial.println("Portinha fechada");

  Serial.print("Quantidade detectada: ");
  Serial.println(quantidadeDetectada);

  if (quantidadeDetectada < MINIMO_ACEITAVEL) {

    racaoBaixa = true;

    Serial.println();
    Serial.println("******** ALERTA ********");
    Serial.println("RACAO BAIXA OU ACABOU");
    Serial.println("***********************");

  }
  else {

    racaoBaixa = false;

    Serial.println("Nivel de racao OK");

  }

  bipFinal();

  Serial.println("================================");
}

//============================================================
// SETUP
//============================================================

void setup() {

  Serial.begin(9600);

  pinMode(PINO_RELE, OUTPUT);
  pinMode(PINO_TCRT5000, INPUT_PULLUP);
  pinMode(PINO_BUZZER, OUTPUT);

  digitalWrite(PINO_RELE, RELE_DESLIGADO);

  servoPortinha.attach(PINO_SERVO_PORTINHA);

  servoPortinha.write(PORTINHA_FECHADA);

  Serial.println();
  Serial.println("================================");
  Serial.println(" DISPENSADOR DE RACAO");
  Serial.println(" Lucas Carvalho");
  Serial.println("================================");

  bipLigado();

  // Primeira dispensa acontece logo após ligar
  ultimoAcionamento = millis() - INTERVALO_ENTRE_DISPENSAS;
}

//============================================================
// LOOP
//============================================================

void loop() {

  // Verifica se chegou o horário da próxima dispensa
  if (millis() - ultimoAcionamento >= INTERVALO_ENTRE_DISPENSAS) {

    ultimoAcionamento = millis();

    acionar();

    Serial.println();
    Serial.println("Aguardando proxima dispensa...");
  }

  // Se a ração estiver baixa, avisa a cada 10 segundos
  if (racaoBaixa &&
      millis() - ultimoBipRacaoBaixa >= 10000UL) {

    ultimoBipRacaoBaixa = millis();

    alertaRacaoBaixa();

    Serial.println("ALERTA: RACAO BAIXA");

  }

}
