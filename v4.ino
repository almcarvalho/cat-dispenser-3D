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
const int RELE_LIGADO = HIGH;
const int RELE_DESLIGADO = LOW;

const int PORTINHA_FECHADA = 90;
const int PORTINHA_ABERTA = 180;

const int DISPENSAS_POR_DIA = 3;
const unsigned long TEMPO_RELE_MS = 60000UL;
const int MINIMO_ACEITAVEL = 1;
const int SENSOR_DETECTADO = LOW;

const unsigned long INTERVALO_ENTRE_DISPENSAS =
    (24UL * 60UL * 60UL * 1000UL) / DISPENSAS_POR_DIA;

unsigned long ultimoAcionamento = 0;
unsigned long ultimoBipRacaoBaixa = 0;
bool racaoBaixa = false;

//============================================================
// BUZZER
//============================================================
void tocar(int frequencia, int duracao){
  tone(PINO_BUZZER, frequencia);
  delay(duracao);
  noTone(PINO_BUZZER);
}

void bipLigado(){
  tocar(1000,100);
  delay(100);
  tocar(2000,300);
}

void bipAntesAcionar(){
  for(int i=0;i<3;i++){
    tocar(2500,700);
    delay(300);
  }
}

void bipFinal(){
  tocar(3000,150);
}

void alertaRacaoBaixa(){
  tocar(800,200);
  delay(100);
  tocar(600,200);
}

void bipDeteccao(){
  tone(PINO_BUZZER,2500);
  delay(20);
  noTone(PINO_BUZZER);
}

//============================================================
// SENSOR TCRT5000
//============================================================
int medirRacaoDuranteDispensa(){

  int totalDeteccoes = 0;
  int estadoAnterior = digitalRead(PINO_TCRT5000);
  unsigned long inicio = millis();

  while(millis()-inicio < TEMPO_RELE_MS){

    int estadoAtual = digitalRead(PINO_TCRT5000);

    if(estadoAtual == SENSOR_DETECTADO &&
       estadoAnterior != SENSOR_DETECTADO){

      totalDeteccoes++;

      bipDeteccao();   // Bip rápido a cada detecção

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
void acionar(){

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

  if(quantidadeDetectada < MINIMO_ACEITAVEL){
    racaoBaixa = true;
    Serial.println("******** ALERTA ********");
    Serial.println("RACAO BAIXA OU ACABOU");
  }else{
    racaoBaixa = false;
    Serial.println("Nivel de racao OK");
  }

  bipFinal();
  Serial.println("================================");
}

//============================================================
// SETUP
//============================================================
void setup(){

  Serial.begin(9600);

  pinMode(PINO_RELE,OUTPUT);
  pinMode(PINO_TCRT5000,INPUT_PULLUP);
  pinMode(PINO_BUZZER,OUTPUT);

  digitalWrite(PINO_RELE,RELE_DESLIGADO);

  servoPortinha.attach(PINO_SERVO_PORTINHA);
  servoPortinha.write(PORTINHA_FECHADA);

  Serial.println("DISPENSADOR DE RACAO");

  bipLigado();

  ultimoAcionamento = millis() - INTERVALO_ENTRE_DISPENSAS;
}

//============================================================
// LOOP
//============================================================
void loop(){

  if(millis() - ultimoAcionamento >= INTERVALO_ENTRE_DISPENSAS){
    ultimoAcionamento = millis();
    acionar();
    Serial.println("Aguardando proxima dispensa...");
  }

  if(racaoBaixa &&
     millis() - ultimoBipRacaoBaixa >= 10000UL){

    ultimoBipRacaoBaixa = millis();
    alertaRacaoBaixa();
    Serial.println("ALERTA: RACAO BAIXA");
  }
}
