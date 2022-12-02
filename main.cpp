/*********
**
** Instituto Tecnológico de Aeronáutica – ITA
** Divisão de Engenharia da Computação – IEC
** Disciplina: CE-289 – Internet das Coisas
** Professor: Cecilia de Azevedo Castro Cesar
** Alunos:
** 1- André Luiz Elias Melo
** 2- Guilherme Trindade Tolentino Bernardo
** 3- Leandro de Oliveira Peixoto
**
** Está implementação tem como objetivo coletar dados de sensoriamento embarcado em Um drone
*********/
 
#ifdef __cplusplus
extern "C" {
#endif
float temprature_sens_read();
#ifdef __cplusplus
}
#endif
float temprature_sens_read();
 
#include <Arduino.h>
#include <WiFi.h>         // Biblioteca com implentação da Comunicação WIFI
#include <ThingSpeak.h>   // Biblioteca com implentação das chamadas ao sistemas ThingSpeaks
#include "DHT.h"          // Biblioteca para leitura de dados do sensor de Temperatura e Humidade DHT11
#include "Ultrasonic.h"   // Biblioteca para calculo de distancia com senosor de ultrason
 
#define DHTTYPE DHT11     // Definição do tipo de sendor de temperatura e humidade  são outras opções DHT 22  (AM2302), AM2321
#define DHTPIN 15         // Pino selecionado para leitura dos dados de Temperatura e Humidade.
DHT dht(DHTPIN, DHTTYPE); //Chamada da biblioteca definindo as confifurações do sensor e da montagem no pino 15
 
int intervalSensor = 2000;
long prevMillisThingSpeak = 0;
int intervalThingSpeak = 15000; // Intervalo minímo para escrever no ThingSpeak write é de 15 segundos
 
const char ssid[] = "Apt 23"; //Nome da Rede Wifi
const char password[] = "caloihtx"; // Senha da Rede wifi
WiFiClient client;
 
/***
* Canal criado na Conta andreeliasmelo@gmail.com
* Senha da Conta Ce-2892022
* Channel ID: 1935196
* Write Key 7ZFMAM15AWGPZ9Z1
* Read key MHTXFD1QBGTZ3ZIG
*/
const long CHANNEL = 1935196;
const char *WRITE_API = "7ZFMAM15AWGPZ9Z1";
 
const int PIN_SENSOR_ECHO = 13; //Pino escolhido para leitura dos dados de Ultrason (distancia)
Ultrasonic ultrasonic(PIN_SENSOR_ECHO); //Chamada da Lib com os paramentros de montagem
 
/******************************************************************************************************
* SETUP Montado.*
* ***************
*                                    ESP32S
*                               :---------------:
* Sensor de Ultrason            :               :        Sensor de Temp E Humidade
*   |-------                    :               :                 ---------|
*   |   () |===SIG=============>|D13         D15|<==========SIG===| #####  |
*   |      |===VCC======\\//===>|GND         GND|<===\\//===VCC===| #DHT#  |
*   |   () |===GND======//\\===>|VCC         VCC|<===//\\===GND===| #####  |  
*   |-------                    |__*__|USB|__*__|                 ---------|
*                                                  
*********************************************************************************************************/
void setup() {
 Serial.begin(115200);
 WiFi.mode(WIFI_STA); //Modo Station
 ThingSpeak.begin(client);  // Inicializa o ThingSpeak
 dht.begin(); //Inicializa o senhor DHT11
}
 
 
/********************************************************************************
* Rotina principal
* Faz a varredura dos sensores:
* Passo 1 Leitura dos sensores Intervos do Dispositivo Temp e Hall
* Passo 2 Leitura da Distancia do Senhor de ultrasson(Inicializado de modo Global)
* Passo 3 Leitura dos dados do Sensor DHT Humidade e Temp
* Passo 4 Veirifica a WIFI
* Passo 5 Faz o mapeamento das leituras nos campos do ThingSpeaks
* Passo 6 Escreve no Canal do ThingSpeaks
* Passo 7 Aguarda a proxima leitura.
**********************************************************************************/
void loop()
{
 int Dev_H = 0;
 
 Dev_H = hallRead();
 
 long distance = ultrasonic.MeasureInCentimeters();
 
 float SensorTH_H = dht.readHumidity();
 float SensorTH_T = dht.readTemperature();
 
 //Serial.println(SensorTH);
    // Conecta ou reconecta o WiFi
   if (WiFi.status() != WL_CONNECTED) {
     Serial.printf("Atenção para conectar o SSID: ");
     Serial.println(ssid);
     while (WiFi.status() != WL_CONNECTED) {
       WiFi.begin(ssid, password);
       Serial.print(".");
       delay(5000);
     }
     Serial.println("\nConectado");
   }   
     delay(1000);    
 
   if (millis() - prevMillisThingSpeak > intervalThingSpeak) {
 
     // Configura os campos com os valores
     /**
      *  MAPA de preenchimento do CANAL
      *
      *  |--------------------------------|
      *  | CAMPO  |     Grandeza Lida     |
      *  |--------------------------------|
      *  |Campo 2 |          HALL         |
      *  |--------------------------------|
      *  |Campo 3 | Temperatura Externa   |
      *  |--------------------------------|
      *  |Campo 4 |        Humidade       |
      *  |--------------------------------|
      *  |Campo 5 |        Distancia      |
      *  |--------------------------------|
 
    */
     ThingSpeak.setField(2,Dev_H);
     ThingSpeak.setField(3,SensorTH_T);
     ThingSpeak.setField(4,SensorTH_H);   
     ThingSpeak.setField(5,distance);   
 
     // Escreve no canal do ThingSpeak
     int x = ThingSpeak.writeFields(CHANNEL, WRITE_API);
     if (x == 200) {
       Serial.println("Update realizado com sucesso");
     }
     else {
       Serial.println("Problema no canal - erro HTTP " + String(x));
     }
 
     prevMillisThingSpeak = millis();
   }
}
