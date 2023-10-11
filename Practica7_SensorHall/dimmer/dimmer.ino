#include <TimerOne.h> // Incluir Librería TimerOne

volatile int i = 0;   // Variable usada por el contador.
int Triac = 3;        // Salida conectada al optoacoplador MOC 3021.
//int dim;              // Controla la intensidad de iluminación, 0 = ON ; 83 = OFF
int T_int = 300;      // Tiempo en el cual se producen las interrupciones en us.
int POT;              
int hall_pin = 2;     // Pin al que está conectada la salida del sensor hall    
int estado_hall = 0;  // Señal de salida del sensor hall
int contador = 0;

void setup() {
  Serial.begin(9600);
  pinMode(Triac, OUTPUT);   // Configurar como salida.
  pinMode(hall_pin, INPUT); // Pin de entrada de la señal del sensor hall
  Timer1.initialize(T_int); // Inicializa la librería con el tiempo deseado.
  Timer1.attachInterrupt(Dimer, T_int); // En cada interrupción ejecuta el código Dimer.
}

void Dimer() {
  // Calcula la duración del pulso en función del valor del potenciómetro
  int pulseWidth = map(POT, 0, 1023, 100, T_int);

  // Enciende el Triac durante la duración calculada
  digitalWrite(Triac, HIGH);
  delayMicroseconds(pulseWidth);
  digitalWrite(Triac, LOW);
}

void loop() {
  POT = analogRead(A0);
  estado_hall = digitalRead(hall_pin);

  if (estado_hall == 0) {
    Serial.print("\n\nIman detectado - " + String(contador));
    contador++;
  }

  delay(250);
}
