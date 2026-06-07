#include <Servo.h>

Servo meuServo;

const int PINO_SERVO = 9;

void setup() {
  Serial.begin(9600);
  meuServo.attach(PINO_SERVO);

  Serial.println("Digite um angulo de 0 a 180 e pressione Enter:");
}

void loop() {
  if (Serial.available() > 0) {
    int angulo = Serial.parseInt();

    if (angulo >= 0 && angulo <= 180) {
      meuServo.write(angulo);

      Serial.print("Servo movido para: ");
      Serial.print(angulo);
      Serial.println(" graus");
    } else {
      Serial.println("Angulo invalido. Digite de 0 a 180.");
    }

    while (Serial.available() > 0) {
      Serial.read();
    }
  }
}
