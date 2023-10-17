#include <TimerOne.h> // Incluir Librería TimerOne
#include <LiquidCrystal.h>

const int rs = 12, en = 11, d4 = 6, d5 = 7, d6 = 8, d7 =9;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

byte plusMinus[8] = {
  B00100,
  B00100,
  B11111,
  B00100,
  B00100,
  B00000,
  B11111,
  B00000,
};

volatile int i = 0;   // Variable usada por el contador.
int Triac = 5;
int T_int = 300;      // Tiempo en el cual se producen las interrupciones en us.
int POT;              
int hall_pin = 2;     // Pin al que está conectada la salida del sensor hall
int decoder_pin = 3;     // Pin al que está conectada la salida del decoder  
int estado_hall = 0;  // Señal de salida del sensor hall
int contadorDec = 0;
volatile unsigned long lastTimeHall = 0;
volatile unsigned long lastTimeDec = 0;
float frecHall;
float frecDec;
float potCal;

void setup() {
  Serial.begin(9600);

  lcd.begin(16, 2);
  lcd.createChar(0, plusMinus);

  pinMode(Triac, OUTPUT);   // Configurar como salida.
  pinMode(hall_pin, INPUT); // Pin de entrada de la señal del sensor hall
  pinMode(decoder_pin, INPUT); // Pin de entrada de la señal del decoder

  // Configura la interrupción del sensor Hall en un cambio de flanco
  attachInterrupt(digitalPinToInterrupt(hall_pin), hallInterrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(decoder_pin), decoderInterrupt, RISING);

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

void hallInterrupt() {
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - lastTimeHall;
  lastTimeHall = currentTime;

  if (elapsedTime != 0) {
    //Serial.print("\nSensor Hall detectado");
    frecHall = 1000.000 / elapsedTime;
    //Serial.println("\nFrecuencia: " + String(frecHall,3));
  }
}

void decoderInterrupt() {
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - lastTimeDec;
  lastTimeDec = currentTime;
  contadorDec++;

  if (elapsedTime != 0 and contadorDec>=210) {
    //Serial.print("\nDecoder detectado");
    frecDec = 1000.000 / (elapsedTime*7*30);
    //Serial.println("\nFrecuencia: " + String(frecDec,3));
    contadorDec=0;
  }
}

void loop() {
  POT = analogRead(A0);
  potCal = map(POT, 0, 1023, 0, 5000);
  potCal = potCal/1000.00;
  potCal = 1000*potCal/5.00;

  Serial.print(String(frecHall,3));
  Serial.print(",");
  Serial.print(String(frecDec,3));
  Serial.print(",");
  Serial.println(String(potCal));

  //Serial.println("\n\nFRECUENCIAS:");
  //Serial.println("Frecuencia Hall: " + String(frecHall,3));
  //Serial.println("Frecuencia Decoder: " + String(frecDec,3));
  //Serial.println("\n\POTENCIOMETRO:");
  //Serial.println("Resistencia Pot: " + String(potCal));

  // Impresión en la LCD:
  lcd.setCursor(0,0);
  //lcd.print("Prueba");
  lcd.print("Resist|FDec|FHal");
  lcd.setCursor(0,1);
  lcd.print(String(potCal,2));
  lcd.print("|");
  lcd.print(String(frecDec,2));
  lcd.print("|");
  lcd.print(String(frecHall,2));
  
  delay(1000);
}
