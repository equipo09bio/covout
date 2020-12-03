#include <Adafruit_GFX.h>        //Librerias del OLED
#include <Adafruit_SSD1306.h>

float tempLM35;
int lectura;
int pin_sensor = A0;            //Pin análogo en donde va conectado el sensor LM35

#define SCREEN_WIDTH 128        // Ancho de pantalla OLED en pixeles
#define SCREEN_HEIGHT 32        // Altura de pantalla OLED en pixeles
#define OLED_RESET    -1
#define MAX_BRIGHTNESS 255

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); //Declarando variables de Visualización

void setup(){
  Serial.begin(115200);         //Iniciar la comunicación en serie a 115200 bits por segundo
  pinMode(pin_sensor, INPUT);   //Declarando que el pin para el LM35
}

void loop(){
  lectura = analogRead(A0);                           //Lee los valores del sensor LM35
  tempLM35 = (500.00 * A0) / 1023;               //Calculo de la temperatura
      
  //Temperatura
  display.println(F("Temp =  "));
  display.println(tempLM35);
  display.println(F(" °C"));
     
  if(tempLM35 < 36.5 || tempLM35 > 40){
    display.println(F("Su temperura es riesgoso. "));
  }
}
