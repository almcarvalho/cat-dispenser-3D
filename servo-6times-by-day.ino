#include <Servo.h>

Servo meuServo;

const int PINO_SERVO = 9;

void acionar() {
  meuServo.write(80);
  Serial.println("Posicao 80");
  delay(10000);

  meuServo.write(89);
  Serial.println("Posicao 89");
}

void setup() {
  Serial.begin(9600);
  meuServo.attach(PINO_SERVO);

  meuServo.write(89); // estado inicial
}

void loop() {
  acionar();

  // aguarda 6 horas
  delay(21600000UL);
}
