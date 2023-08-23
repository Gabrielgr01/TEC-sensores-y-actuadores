//////////////////////////////
// calculo_resistencia_pot.ino
// 14/08/2023
// 
// Creado por: 
// Gabriel González Rodríguez
// Alexander Solís Quesada
//////////////////////////////

// Llamada a librerías externas
#include <LiquidCrystal.h>

// Declaración de variables globales
float resistenciaIngresadaPot;      // Resistencia del potenciómetro medida con multímetro (utilizado: 9380 ohm).
float incertidumbreIngresadaPot;    // Resolución del instrumento/multímetro (utilizado: 10 ohm).

float tensionRef;                   // Tensión de referencia medida.
float incertidumbreVRef;            // Incertidumbre de la tensionRef.

const int analogPin1 = A0;          // Se define el pin de entrada analógica A0.
int tensionLeidaPot;                // Variable que almacena la lectura analógica raw de la tensión en el potenciometro.
float tensionPot;                   // Tensión del potenciómetro obtenida a partir de tensionLeidaPot.
float incertidumbreArduino = 0.005; // Incertidumbre de la tensionPot.

float resistenciaCalculadaPot;      // Resistencia del potenciómetro calculada.
float derivadaVPot;                 // Derivada parcial de la función para calcular la resistencia del pot con respecto a tensionPot.
float derivadaVRef;                 // Derivada parcial de la función para calcular la resistencia del pot con respecto a tensionRef.
float derivadaRPotIngresada;        // Derivada parcial de la función para calcular la resistencia del pot con respecto a resisteciaIngresadaPot.
float incertidumbreResistencia;     // Incertidumbre obtenida para el valor de la resistencia calculada para el potenciómetro.

// Intanciación de pines del LCD
const int rs = 12, en = 11, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Carácteres custom para usar en el LCD
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

byte ohm[8] = {
  B00000,
  B11111,
  B10001,
  B10001,
  B11011,
  B01010,
  B11011,
  B00000,
};

void setup() {
  // Inicio y encabezado del programa
  lcd.begin(16,2);
  lcd.createChar(0, plusMinus);
  lcd.createChar(1, ohm);

  Serial.begin(9600);
  Serial.println("\n\n");
  Serial.println("------------ LECTURA DE LA RESISTENCIA DE UN POTENCIOMETRO ------------");
  
  // Ingreso del dato de resistencia del potenciómetro medida
  Serial.println("Ingrese la resistencia del potenciómetro medida (en kΩ):");
  while (Serial.available() == 0) {
    resistenciaIngresadaPot = Serial.parseFloat();
  }
  Serial.end();
  // Ingreso de la incertidumbre de la resistencia del potenciómetro
  Serial.begin(9600);
  Serial.println("Ingrese la incertidumbre de la resistencia del potenciómetro:");
  while (Serial.available() == 0) {
    incertidumbreIngresadaPot = Serial.parseFloat();
  } 
  Serial.end();

  // Ingreso del dato de tensión de referencia medida
  Serial.begin(9600);
  Serial.println("Ingrese la tensión de referencia medida (en V):");
  while (Serial.available() == 0) {
    tensionRef = Serial.parseFloat();
  } 
  Serial.end();
  // Ingreso de la incertidumbre de la tensión de referencia
  Serial.begin(9600);
  Serial.println("Ingrese la incertidumbre de la tensión de referencia:");
  while (Serial.available() == 0) {
    incertidumbreVRef = Serial.parseFloat();
  } 

  // Se imprime resumen de los datos ingresados
  Serial.println("\nResumen Datos Ingresados: ");
  Serial.println("Considerando un potenciómetro de ("+ String(resistenciaIngresadaPot) + " ± " + String(incertidumbreIngresadaPot) + ") kΩ.");
  Serial.println("Considerando una tensión de referencia de (" + String(tensionRef) + " ± " + String(incertidumbreVRef) + ") V.");
  Serial.println("\nEl programa se ejecutará en breve.");
  delay(5000);
}

void loop() {
  // Lectura de tensiones en el pin1 (potenciómetro) y pin2 (referencia)
  tensionLeidaPot = analogRead(analogPin1);                 // realizar la lectura analógica raw
  tensionPot      = map(tensionLeidaPot, 0, 1023, 0, 5000); // convertir a mV
  tensionPot      = tensionPot/1000;                        // convertir a V
  Serial.println(incertidumbreArduino);

  // Cálculo de la resistencia del potenciómetro
  resistenciaCalculadaPot = (tensionPot/tensionRef)*resistenciaIngresadaPot;

  // Cálculo de la incertimbre de la resistencia en el potenciómetro
  derivadaVPot = resistenciaIngresadaPot / tensionRef;
  derivadaVRef = - (tensionPot * resistenciaIngresadaPot) / pow(tensionRef,2);
  derivadaRPotIngresada = tensionPot / tensionRef;
  incertidumbreResistencia = sqrt(pow(derivadaVPot * incertidumbreArduino,2)+pow(derivadaVRef * incertidumbreVRef,2)+pow(derivadaRPotIngresada * incertidumbreIngresadaPot,2));

  // Impresión del resumen de los datos en el monitor serial
  Serial.print("\n\n\n");
  
  Serial.print("\nTensión de referencia:            (" + String(tensionRef,2) + " ± ");
  Serial.print(incertidumbreVRef,2);
  Serial.print(") V");
  
  Serial.print("\nTensión del potenciometro:        (" + String(tensionPot,3) + " ± ");
  Serial.print(incertidumbreArduino, 3);
  Serial.print(") V");
  
  Serial.print("\nResistencia de referencia (máx):  (" + String(resistenciaIngresadaPot,2) + " ± " + String(incertidumbreIngresadaPot,2) + ") kΩ");
  Serial.print("\nResistencia del potenciometro:    (" + String(resistenciaCalculadaPot,2) + " ± " + String(incertidumbreResistencia,2) + ") kΩ");
  
  // Impresión de datos en LCD
  lcd.setCursor(0,0);
  lcd.print("Potenciometro:");
  lcd.setCursor(0,1);
  lcd.print(String(resistenciaCalculadaPot,2) + " ");
  lcd.write(byte(0)); // prints plusMinus
  lcd.print(" " + String(incertidumbreResistencia,2) + " k");
  lcd.write(byte(1)); // prints ohm symbol

  delay(1000);
  lcd.clear();
}
