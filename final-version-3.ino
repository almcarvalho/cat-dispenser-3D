//BUZZER AND TCRT SENSOR BY LUCAS CARVALHO 24/06/2026

#include <Servo.h>

Servo servoRacao;
Servo servoPortinha;

// ---------------- PINOS ----------------
const int PINO_SERVO_RACAO = 9;
const int PINO_SERVO_PORTINHA = 10;
const int PINO_TCRT5000 = 2;
const int PINO_BUZZER = 8;

// ---------------- CONFIGURAÇÕES ----------------
const int POSICAO_RACAO_ATIVA = 80;
const int POSICAO_RACAO_REPOUSO = 89;

const int PORTINHA_FECHADA = 90;
const int PORTINHA_ABERTA = 180;

const int DISPENSAS_POR_DIA = 3;
const int TEMPO_DISPENSA_SEGUNDOS = 8;

// Quantidade mínima aceitável de detecções
const int MINIMO_ACEITAVEL = 1;

// Se o sensor detectar invertido, troque LOW por HIGH
const int SENSOR_DETECTADO = LOW;

const unsigned long INTERVALO_ENTRE_DISPENSAS =
    (24UL * 60UL * 60UL * 1000UL) / DISPENSAS_POR_DIA;

// ---------------- VARIÁVEIS ----------------
unsigned long ultimoAcionamento = 0;
unsigned long ultimoBipRacaoBaixa = 0;

bool racaoBaixa = false;

// ---------------- BUZZER ----------------

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

// ---------------- SENSOR ----------------

int medirRacaoDuranteDispensa() {
  int totalDeteccoes = 0;

  int estadoAnterior = digitalRead(PINO_TCRT5000);

  unsigned long inicio = millis();
  unsigned long tempoDispensa = TEMPO_DISPENSA_SEGUNDOS * 1000UL;

  while (millis() - inicio < tempoDispensa) {

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

// ---------------- DISPENSA ----------------

void acionar() {

  Serial.println("--------------------------------");
  Serial.println("Preparando dispensa");

  bipAntesAcionar();

  servoPortinha.write(PORTINHA_ABERTA);
  Serial.println("Portinha aberta");

  delay(2000);

  servoRacao.write(POSICAO_RACAO_ATIVA);
  Serial.println("Liberando racao...");

  int quantidadeDetectada = medirRacaoDuranteDispensa();

  servoRacao.write(POSICAO_RACAO_REPOUSO);

  Serial.println("Dispensa finalizada");

  delay(2000);

  servoPortinha.write(PORTINHA_FECHADA);

  Serial.println("Portinha fechada");

  Serial.print("Quantidade detectada: ");
  Serial.println(quantidadeDetectada);

  if (quantidadeDetectada < MINIMO_ACEITAVEL) {

    racaoBaixa = true;

    Serial.println("***** ALERTA *****");
    Serial.println("RACAO BAIXA OU ACABOU");
    Serial.println("******************");

  } else {

    racaoBaixa = false;

    Serial.println("Nivel de racao OK");
  }

  bipFinal();

  Serial.println("--------------------------------");
}

// ---------------- SETUP ----------------

void setup() {

  Serial.begin(9600);

  pinMode(PINO_TCRT5000, INPUT_PULLUP);
  pinMode(PINO_BUZZER, OUTPUT);

  servoRacao.attach(PINO_SERVO_RACAO);
  servoPortinha.attach(PINO_SERVO_PORTINHA);

  servoRacao.write(POSICAO_RACAO_REPOUSO);
  servoPortinha.write(PORTINHA_FECHADA);

  Serial.println("Sistema iniciado");

  bipLigado();

  // Faz a primeira dispensa imediatamente após ligar
  ultimoAcionamento = millis() - INTERVALO_ENTRE_DISPENSAS;
}

// ---------------- LOOP ----------------

void loop() {

  // Dispensa automática
  if (millis() - ultimoAcionamento >= INTERVALO_ENTRE_DISPENSAS) {

    ultimoAcionamento = millis();

    acionar();

    Serial.println("Aguardando proxima dispensa...");
  }

  // Alerta de ração baixa a cada 10 segundos
  if (racaoBaixa &&
      millis() - ultimoBipRacaoBaixa >= 10000UL) {

    ultimoBipRacaoBaixa = millis();

    alertaRacaoBaixa();

    Serial.println("ALERTA: RACAO BAIXA");
  }
}
