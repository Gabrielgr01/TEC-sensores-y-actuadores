//////////////////////////////
// termistor.ino
// 19/08/2023
// 
// Creado por: 
// Gabriel González Rodríguez
// Alexander Solís Quesada
//////////////////////////////

// ---------------------------------- Para el funcionamiento de la pantalla LCD ----------------------------------
#include <LiquidCrystal.h>

const int rs = 11, en = 12, d4 = 4, d5 = 5, d6 = 6, d7 =7;
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
// Declaración de variables globales
// ---------------------------------- Para el cálculo de RT y su incertidumbre ----------------------------------
float RT;                                   // Resistencia del termistor.
float incertidumbreRT;                      // Incertidumbre de la resistencia del termistor.

float R = 101900;                           // Resistencia en serie (multimetro).
float incertidumbreR = 2000;                // Resolución de la resistencia en serie.

float VCC = 5.04;                           // Voltage de alimentación (multimetro).
float incertidumbreVCC = 0.01;              // Incertidumbre del voltaje de alimentacion.

float VT;                                   // Voltage en el termistor (arduino).
float incertidumbreVT = 0.005;              // Incertidumbre del arduino.
int VTLectura;
const int analogPin1 = A0;                  // Se define el pin de entrada analógica A0.

float derivadaVCC;                          // Derivada parcial de la función para calcular la resistencia del termistor respecto a VCC.
float derivadaVT;                           // Derivada parcial de la función para calcular la resistencia del termistor respecto a VT.
float derivadaR;                            // Derivada parcial de la función para calcular la resistencia del termistor respecto a R.

// ---------------------------------- Para el cálculo de T y su incertidumbre ----------------------------------
float T;                                    // Temperatura del termistor.
float incertidumbreT;                       // Incertidumbre de la temperatura del termistor.

int B = 4529;                               // Constante B (hoja de datos).

float R0 = 100000;                          // Resistencia a 25 celsius (hoja de datos).
float incertidumbreR0 = 5000;               // Incertidumbre de la resistencia a 25 celcius (hoja de datos).

float T0 = 298.15;                          // Temperatura base 25 celsius (hoja de datos).
float incertidumbreT0 = 0.05;               // Incertidumbre de la temperatura base 25 celsius (hoja de datos).

float derivadaT0;                           // Derivada parcial de la función para calcular la temperatura respecto a T0.
float derivadaR0;                           // Derivada parcial de la función para calcular la temperatura respecto a R0.
float derivadaRT;                           // Derivada parcial de la función para calcular la temperatura respecto a RT.


void setup() {
  Serial.begin(9600);
  Serial.println("\nEl programa se está ejecutando.");
  lcd.begin(16, 2);
  lcd.createChar(0, plusMinus);
  delay(5000);
}

void loop() {
  // Lectura de tensión en el termistor
  VTLectura = analogRead(analogPin1);                       // realizar la lectura analógica raw
  VT = map(VTLectura, 0, 1023, 0, 5000);                    // convertir a mV
  VT = VT/1000;                                             // convertir a V

  // Cálculo de la resistencia del termistor y su incertidumbre
  RT = R/((VCC/VT)-1);
  derivadaVCC = -(R*VT)/pow(VCC-VT,2);
  derivadaVT = (VCC*R)/pow(VCC-VT,2);
  derivadaR = VT/(VCC-VT);
  incertidumbreRT = sqrt(pow((derivadaVCC*incertidumbreVCC),2)+pow((derivadaVT*incertidumbreVT),2)+pow((derivadaR*incertidumbreR),2));

  // Impresión del resumen de los datos para RT en el monitor serial.
  //Serial.print("\n\n\n");
  //Serial.print("\nLa resistencia del termistor es de: (" + String(RT,2) + " ± ");
  //Serial.print(String(incertidumbreRT,2));
  //Serial.print(") Ω");
 
   // Cálculo de la resistencia del termistor y su incertidumbre
  T = (B*T0)/(T0*log(RT/R0)+B)-273.15;
  derivadaT0 = pow(B,2)/pow(B+T0*log(RT/R0),2);
  derivadaR0 = (B*pow(T0,2))/(R0*pow(B+T0*log(RT/R0),2));
  derivadaRT = (B*pow(T0,2))/(RT*pow(B+T0*log(RT/R0),2));
  incertidumbreT = sqrt(pow((derivadaRT*incertidumbreRT),2)+pow((derivadaT0*incertidumbreT0),2)+pow((derivadaR0*incertidumbreR0),2));

  // Impresión del resumen de los datos para T en el monitor serial.
  
  Serial.print("\nLa temperatura es de: (" + String(T,2) + " ± ");
  Serial.print(String(incertidumbreT,2));
  Serial.print(") C");

  // Impresión en la LCD:
  lcd.setCursor(0,0);
  lcd.print("TEMPERATURA:");
  lcd.setCursor(0,1);
  lcd.print("(" + String(T,2));
  lcd.write(byte(0));
  lcd.print(String(incertidumbreT,2) + ")");
  lcd.print((char)223);
  lcd.print("C");

  delay(1000);
}
