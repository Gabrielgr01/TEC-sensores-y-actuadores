//////////////////////////////
// termistor.ino
// 25/09/2023
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
// ---------------------------------- Para el cálculo de VP, T y sus incertidumbres ----------------------------------

float VP;                                   // Voltage en el paralelo (arduino).
float incertidumbreVT = 0.005;              // Incertidumbre del arduino.
int VTLectura;
const int analogPin1 = A0;                  // Se define el pin de entrada analógica A0.

float T;                                    // Temperatura del termistor.
float incertidumbreT;                       // Incertidumbre de la temperatura del termistor.



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
  VP = map(VTLectura, 0, 1023, 0, 5000);                    // lectura en mV
  VP = VP/1000;                                             // convertir a V
 
  // Cálculo de la resistencia del termistor y su incertidumbre
  T = (2.2-VP)/0.026;
  incertidumbreT = sqrt((250000/28561)*pow((VP-2.2),2)+(1003/676));

  // Impresión del resumen de los datos para T en el monitor serial.
  
  Serial.print("\nLa temperatura es de: (" + String(T,2) + " ± ");
  Serial.print(String(incertidumbreT,2));
  Serial.print(") C");
  Serial.print("\nLa tensión leída es de: " + String(VP,2));

  // Impresión en la LCD:
  lcd.setCursor(0,0);
  lcd.print("TEMPERATURA:");
  lcd.setCursor(0,1);
  lcd.print("(" + String(T,1));
  lcd.write(byte(0));
  lcd.print(String(incertidumbreT,1) + ")");
  lcd.print((char)223);
  lcd.print("C");

  delay(1000);
}
