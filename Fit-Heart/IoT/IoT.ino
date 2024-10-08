#define USE_ARDUINO_INTERRUPTS true
#include <WiFi.h>
#include <WiFiUdp.h>

#define SERIAL_BAUD 115200

const char * udpAddress = "192.168.0.189";//mac
const int udpPort = 4444;

const char* ssid     = "";
const char* password = "";

/*
const char * udpAddress = "192.168.0.35";//mac
const int udpPort = 4444;

const char* ssid     = "Fam SQ";
const char* password = "1970alsoli";
*/
// Variables para la temperatura
double temperaturas = 0;

// Variables para la intensidad
int intensidad = 0;

// Variable para HRV
float cuadrado;
float suma_cuadrados;
float HRV = 0;

// Variables para los BPM
const int umbral = 1700;
float BPM = 0;

// Variables para la intensidad
float pesoBPM = 0;
float pesoHRV = 0;
float pesoTemp = 0;
float pesos = 0;

// Variables de contadores y banderas
int contadorPulsosHRV = 0;
int contadorPulsosBPM = 0;
int contadorMediciones = 0;
int flagTimer = 0;
int flagPulso = 0;

// Variables de tiempo
float tiempo_inmediato = 0;
float tiempo_inicio_total = 0;
float tiempo_inicio_pulso_anterior = 0;
float tiempo_RR_anterior = 0;
float tiempo_RR_actual = 0;
float tiempo_actualizar_interfaz = 0;
float tiempo_ultima_actualizacion = 0;
float tiempo_total = 0;

WiFiUDP udp;

void setup() {
  Serial.begin(SERIAL_BAUD);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
    Serial.println("");
    Serial.println("WiFi connected with IP address: ");
    Serial.println(WiFi.localIP());

  Serial.println("Setup done!");

  pinMode(34, INPUT);
  pinMode(36, INPUT);
  pinMode(15, OUTPUT);
  
}

void loop(){
  //--------------------------------------------------------LECTURA DE PULSO---------------------------------------------------------
  
  // Se realiza la lectura del sensor de pulso, el cálculo de bpm y de hrv
  int pulso = analogRead(34);            // Se lee la señal del sensor
  int ET = analogRead(36);               // Recibe el ADC del termistor
  
  //--------------------------------------------------------LECTURA DE TEMPERATURA---------------------------------------------------

  // Se calcula la temperatura
  double VT = ET*3.33/4095;                           // Lo convierte a voltaje
  double RT = (3.33*4700)/VT - 4700;                   // Hace el divisor de voltaje para obtener resistencia del termistor
  temperaturas = temperaturas + (1/(0.00130752 + 0.000231689 * log(RT) - 0.000000118475 * pow(log(RT), 3))) - 273.15 - 6.7;
  contadorMediciones = contadorMediciones + 1;

  //-------------------------------------------------SE PROCESA EL PULSO Y TIEMPOS---------------------------------------------------
  
  tiempo_inmediato = millis();           // Se empieza a contar el tiempo
  // Tiempo para actualizar datos en interfaz
  tiempo_actualizar_interfaz = tiempo_inmediato - tiempo_ultima_actualizacion;
  
  if (pulso > umbral){
    // Se verifica si se está en el primer pulso
    if (contadorPulsosHRV == 0){
      // Se guarda el tiempo de inicio
      tiempo_inicio_total = tiempo_inmediato;
      tiempo_inicio_pulso_anterior = tiempo_inmediato;
      // Se levanta la bandera para que se de el conteo de tiempo total
      flagTimer = 1;
    }
    // Se verifica si se está dentro de un pulso
    if (flagPulso == 0){
      
      // Se verifica que no sea el primer pulso
      if (contadorPulsosHRV != 0){
        // Se verifica si ya se pasó el primer pulso y segundo pulso
        if (contadorPulsosHRV >= 2){
          // Se registra el tiempo anterior con el tiempo actual antes de ser actualizado
          tiempo_RR_anterior = tiempo_RR_actual;
        }
        
        // Se calcula el tiempo RR con el tiempo inmediato y el del incio anterior
        tiempo_RR_actual = tiempo_inmediato - tiempo_inicio_pulso_anterior;
        // Se actualiza el tiempo inmediato como el inicio del pulso anterior para futuros pulsos
        tiempo_inicio_pulso_anterior = tiempo_inmediato;

        // Se calculan los BPM promedio en el tiempo recorrido desde la última actualización
        BPM = BPM + ((1000 / (tiempo_RR_actual))) * 60;
        
        // Se verifica si ya se pasó el primer pulso una vez actualizados los tiempos
        if (contadorPulsosHRV >= 2){
          // Se calcula el cuadrado de la diferencia entre los últimos dos tiempos RR
          cuadrado = pow((tiempo_RR_anterior - tiempo_RR_actual), 2);
          // Se actualiza el sumador de cuadrados de diferencias
          suma_cuadrados = suma_cuadrados + cuadrado;
          // Cálculo de HRV por RMSSD
          HRV = sqrt(suma_cuadrados / (contadorPulsosHRV - 3));
        }
      }   

      // Se suma un pulso al contador
      contadorPulsosHRV = contadorPulsosHRV + 1;
      contadorPulsosBPM = contadorPulsosBPM + 1;
      
      // Se desactiva la posibilidad de contar pulsos hasta que pulso sea menor que el umbral
      flagPulso = 1;
    }
  }
  else{
    // Se restablece la posibilidad de contabilizar pulsos
    flagPulso = 0;
  }

  // Si la bandera del contador está habilitada, se permite contabilizar tiempo
  if (flagTimer == 1){
    tiempo_total = tiempo_inmediato - tiempo_inicio_total;
  }

  //--------------------------------------------------------INICIO UDP---------------------------------------------------------

  // Cada cierto tiempo se reinicia del todo el sistema
  if (tiempo_total > 5000){
    // RESETEO DE VARIABLES
    tiempo_ultima_actualizacion = tiempo_inmediato;
    tiempo_total = 0;
    flagTimer = 0;

    //CÁLCULO DE VARIABLES
    temperaturas = temperaturas / contadorMediciones;
    BPM = BPM / (contadorPulsosBPM - 1);
    
    //CÁLCULO DE INTENSIDAD
    // Peso de los BPM
    if (BPM <= 220 and BPM > 0){
      pesoBPM = 0.00005 * pow(BPM, 2) - 0.0002 * BPM - 0.0043;
    } else {
      pesoBPM = 0;
    }

    // Peso del HRV
    if (HRV >= 120){
      pesoHRV = 0;
    } else if (HRV >= 60 and pesoBPM >= 0.6){
      pesoHRV = 0.15;
    } else if (HRV >= 25 and pesoBPM >= 0.6){
      pesoHRV = 0.6;
    } else if (HRV >= 10 and pesoBPM >= 0.6){
      pesoHRV = 0.9;
    } else {
      pesoHRV = 0;
    }

    // Peso de la temperatura
    if (temperaturas <= 40 and temperaturas >= 37.5){
      pesoTemp = 0.3;
    } else {
      pesoTemp = 0;
    }

    Serial.println("NUEVOS PESOS");
    Serial.println(BPM);
    Serial.println(pesoBPM);
    Serial.println(HRV);
    Serial.println(pesoHRV);
    Serial.println(temperaturas);
    Serial.println(pesoTemp);
    Serial.println("PESOS");
    Serial.println(pesos);

    // Cálculo del peso total
    pesos = pesoBPM + pesoHRV + pesoTemp;

    if (pesos >= 3){
      intensidad = 6;
    }
    else if (pesos >= 2.5){
      intensidad = 5;
    }
    else if (pesos >= 2){
      intensidad = 4;
    }
    else if (pesos >= 1.5){
      intensidad = 3;
    }
    else if (pesos >= 1){
      intensidad = 2;
    }
    else if (pesos >= 0.5){
      intensidad = 1;
    }
    else{
      intensidad = 0;
    }
    // Se activca o desactiva la alarma
    if (intensidad == 6){
       digitalWrite(15, 1);
    } else {
       digitalWrite(15, 0);
    }
   
    // Se define un mensaje que va a ser enviado por udp
    char msg[200];
    udp.beginPacket(udpAddress, udpPort);
    sprintf(msg, "%d,%f,%f,%f,%d", pulso, BPM, HRV, temperaturas, intensidad);
    Serial.println(msg);
    udp.print(msg);
    udp.endPacket();
    
    HRV = 0;
    contadorPulsosHRV = 0;
    suma_cuadrados = 0;
    
    BPM = 0;
    contadorPulsosBPM = 0;
    
    temperaturas = 0;
    contadorMediciones = 0;
  }

  else{
    // Se define un mensaje que va a ser enviado por udp
    char msg[200];
    udp.beginPacket(udpAddress, udpPort);
    sprintf(msg, "%d,%d,%d,%d,%d",pulso,0,0,0,0);
    udp.print(msg);
    udp.endPacket();
  }
  
  delay(50);
}
