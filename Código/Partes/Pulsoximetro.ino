#include <Adafruit_GFX.h>        //Librerias del OLED
#include <Adafruit_SSD1306.h>

#include <Wire.h>
#include "MAX30105.h"           //Librerias del oximetro
#include "spo2_algorithm.h"     //Algoritmo para calcular pulso y saturación

MAX30105 particleSensor;        //Función de cálculo

uint16_t irBuffer[100];         //Datos del sensor de infrarrojos LED
uint16_t redBuffer[100];        //Datos del sensor LED rojo

int32_t bufferLength;           //Declarando el número de datos
int32_t spo2;                   //Valor del SaO2
int8_t validSPO2;               //Indicador para mostar si el SaO2 es válido
int32_t heartRate;              //Valor de PPM
int8_t validHeartRate;          //Indicaro para mostrar si el PPM es válido

byte pulseLED = 11;             //Debe estar en el pin PWM (para señales)
byte readLED = 13;              //LED que va a parpadear con cada lectura de datos

#define SCREEN_WIDTH 128        // Ancho de pantalla OLED en pixeles
#define SCREEN_HEIGHT 32        // Altura de pantalla OLED en pixeles
#define OLED_RESET    -1
#define MAX_BRIGHTNESS 255

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); //Declarando variables de Visualización

void setup(){
  Serial.begin(115200);         //Iniciar la comunicación en serie a 115200 bits por segundo

  pinMode(pulseLED, OUTPUT);    //Declarando que el pin para el MAX10302
  pinMode(readLED, OUTPUT);     //Declarando que el pin para el MAX10302
  
  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)){ //Utilizando el puerto I2C predeterminado, velocidad de 400 kHz
  
    display.println(F("MAX30105 no se encuentra conectado."));     //Estilo de letra
    while (1);
  }
  display.println(F("Presione el botón para empezar a tomar las muestras"));       //Estilo de letra OLED
  while (Serial.available() == 0) ;   //Para que espere hasta que el usuario presione una tecla
  Serial.read();

  byte ledBrightness = 60;            //Opciones: 0 = Apagado a 255=50mA
  byte sampleAverage = 4;             //Opciones: 1, 2, 4, 8, 16, 32
  byte ledMode = 2;                   //Opciones: 1 = Solo Rojo, 2 = Rojo + IR, 3 = Rojo + IR + Verde
  byte sampleRate = 100;              //Octiones: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 411;               //Opciones: 69, 118, 215, 411
  int adcRange = 4096;                //Opciones: 2048, 4096, 8192, 16384

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);     //Configurando el sensor con dichas opciones
}

void loop(){
  bufferLength = 100;                 //La longitud del búfer es de 100 (4 segundos)

  //Leer las primeras 100 muestras y determinar el rango de la señal
  for (byte i = 0 ; i < bufferLength ; i++){
    
    while (particleSensor.available() == false)       //Ver si tenemos nuevos datos
      particleSensor.check();                         //Comprobando el sensor para nuevos datos

    redBuffer[i] = particleSensor.getRed();           //Guardando el valor de "Rojo"
    irBuffer[i] = particleSensor.getIR();             //Guardando el valor del "IR"
    particleSensor.nextSample();                      //Moviendonos a la siguiente toma de datos
   }

  //Calcular la frecuencia cardíaca y la SpO2 después de las primeras 100 muestras (primeros 4 segundos de las muestras)
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
  
     

  //Tomando muestras continuamente de MAX30102. La frecuencia cardíaca y la SpO2 se calculan cada segundo
  while (1){
    
    //Colocar los primeros 25 conjuntos de muestras en la memoria
    for (byte i = 25; i < 100; i++){
      redBuffer[i - 25] = redBuffer[i];
      irBuffer[i - 25] = irBuffer[i];
    }

    //Tomando los 25 conjuntos de muestras antes de calcular la frecuencia cardíaca.
    for (byte i = 75; i < 100; i++){
      while (particleSensor.available() == false)         //Ver si tenemos nuevos datos
        particleSensor.check();                           //Comprobando el sensor para nuevos datos

      digitalWrite(readLED, !digitalRead(readLED));       //Parpadea el LED integrado en cada lectura de datos

      redBuffer[i] = particleSensor.getRed();
      irBuffer[i] = particleSensor.getIR();
      particleSensor.nextSample();                        //Moviendonos a la siguiente toma de datos

      //Mostrando los cálculos obtenidos

      //PPM
      display.clearDisplay();                                
      display.println(F("PPM = "));
      display.println(heartRate, DEC);

      if(heartRate < 50 || heartRate > 100){
        display.println(F("Su PPM es riesgoso. "));
      }

      //SaO2
      display.println(F("SaO2 = "));
      display.println(spo2, DEC);

      if(spo2 < 60 || spo2 > 100){
        display.println(F("Su SaO2 es riesgoso. "));
      }
    }
    //Después de recopilar 25 muestras nuevas, vuelva a calcular HR y SP02
    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
  }
}
