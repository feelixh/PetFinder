// Comunicação LoRa com Arduino 
// Definicao das bibliotecas a serem utilizadas no projeto
#include <SPI.h>             
#include <LoRa.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <HardwareSerial.h>
#include<TinyGPS.h> 
#include <TinyGPS++.h>

#define RXD 16
#define TXD 17

TinyGPS GPS;  
TinyGPSPlus gps;   
                  
float lat, lon, vel;
unsigned long data, hora;
unsigned short sat;
const char* ssid = "yourNetworkName";
const char* password =  "yourNetworkPassword";

// Definicacao de constantes
const int csPin = 5;         // Chip Select (Slave Select do protocolo SPI) do modulo Lora
const int resetPin = 0;       // Reset do modulo LoRa
const int irqPin = 4;         // Pino DI0
 
String outgoing;              // outgoing message
 
byte localAddress = 0xBB;     // Endereco deste dispositivo LoRa
byte msgCount = 0;            // Contador de mensagens enviadas
byte destination = 0xFF;      // Endereco do dispositivo para enviar a mensagem (0xFF envia para todos devices)
long lastSendTime = 0;        // TimeStamp da ultima mensagem enviada
int interval = 5000;          // Intervalo em ms no envio das mensagens (inicial 5s)
 
// Setup do Microcontrolador
void setup() 
{
  // inicializacao da serial 
  Serial.begin(115200); 
  Serial2.begin(9600 ,SERIAL_8N1, RXD, TXD);                  
  while (!Serial);
  
  /*WiFi.begin(ssid, password); 
   
  while (WiFi.status() != WL_CONNECTED) { //Check for the connection
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
 
  Serial.println("Connected to the WiFi network");*/
 
  Serial.println(" Comunicacao LoRa Duplex - Ping&Pong ");
 
  // override the default CS, reset, and IRQ pins (optional)
  LoRa.setPins(csPin, resetPin, irqPin);// set CS, reset, IRQ pin
 
  // Inicializa o radio LoRa em 915MHz e checa se esta ok!
  if (!LoRa.begin(915E6)){             
    Serial.println(" Erro ao iniciar modulo LoRa. Verifique a coenxao dos seus pinos!! ");
    while (true);                      
  }
 
  Serial.println(" Modulo LoRa iniciado com sucesso!!!");
  Serial.println("Setup Completado!");
}
 
// Loop do microcontrolador - Operacoes de comunicacao LoRa
void loop(){
   while (Serial2.available()) {    
     gps.encode(Serial2.read());     
  }
  
  if (gps.altitude.isUpdated()){
      Serial.print("LATITUDE= "); Serial.println(gps.location.lat(), 6);
      Serial.print("LONGITUDE= "); Serial.println(gps.location.lng(), 6);
      Serial.print("ALTITUDE EM METROS= "); Serial.println(gps.altitude.meters());
      Serial.print("DATA= "); Serial.print(gps.date.day()); Serial.print("/"); Serial.print(gps.date.month()); Serial.print("/"); Serial.println(gps.date.year());
      Serial.print("HORA= "); Serial.print(gps.time.hour()); Serial.print(":"); Serial.print(gps.time.minute()); Serial.print(":"); Serial.println(gps.time.second());
      Serial.print("VELOCIDADE KM/H= "); Serial.println(gps.speed.kmph());
      Serial.print("SATELITES= "); Serial.println(gps.satellites.value());
      Serial.println();

      Serial.println(String(gps.date.day())+String(gps.date.month())+String(gps.date.year()));


     }
    


    
    /*
     * tinygps
     * if (GPS.encode(Serial2.read())) {
 
      //Hora e data
      GPS.get_datetime(&data, &hora);
      
      Serial.print("--");
      Serial.print(hora / 1000000);
      Serial.print(":");
      Serial.print((hora % 1000000) / 10000);
      Serial.print(":");
      Serial.print((hora % 10000) / 100);
      Serial.print("--");
 
      Serial.print(data / 10000);
      Serial.print("/");
      Serial.print((data % 10000) / 100);
      Serial.print("/");
      Serial.print(data % 100);
      Serial.println("--");
      
      //latitude e longitude
      GPS.f_get_position(&lat, &lon);
 
      Serial.print("Latitude: ");
      Serial.println(lat, 6);
      Serial.print("Longitude: ");
      Serial.println(lon, 6);
 
      //velocidade
      vel = GPS.f_speed_kmph();
 
      Serial.print("Velocidade: ");
      Serial.println(vel);
 
      //Satelites
      sat = GPS.satellites();
 
      if (sat != TinyGPS::GPS_INVALID_SATELLITES) {
        Serial.print("Satelites: ");
        Serial.println(sat);
      }
      
      Serial.println("");
    }*/

    
  // verifica se temos o intervalo de tempo para enviar uma mensagem
  if (millis() - lastSendTime > interval){
    String mensagem = " Ola mundo! :O ";    // Definicao da mensagem 
    sendMessage(mensagem);
    Serial.println("Enviando " + mensagem);
    lastSendTime = millis();            // Timestamp da ultima mensagem
  }
 
  // parse for a packet, and call onReceive with the result:
  onReceive(LoRa.parsePacket());
}
 
// Funcao que envia uma mensagem LoRa
void sendMessage(String outgoing){
  LoRa.beginPacket();                   // Inicia o pacote da mensagem
  LoRa.write(destination);              // Adiciona o endereco de destino
  LoRa.write(localAddress);             // Adiciona o endereco do remetente
  LoRa.write(msgCount);                 // Contador da mensagem
  LoRa.write(outgoing.length());        // Tamanho da mensagem em bytes
  LoRa.print(outgoing);                 // Vetor da mensagem 
  LoRa.endPacket();                     // Finaliza o pacote e envia
  msgCount++;                           // Contador do numero de mensagnes enviadas
}
 
// Funcao para receber mensagem 
void onReceive(int packetSize){
  if (packetSize == 0) return;          // Se nenhuma mesnagem foi recebida, retorna nada
 
  // Leu um pacote, vamos decodificar? 
  int recipient = LoRa.read();          // Endereco de quem ta recebendo
  byte sender = LoRa.read();            // Endereco do remetente
  byte incomingMsgId = LoRa.read();     // Mensagem
  byte incomingLength = LoRa.read();    // Tamanho da mensagem
 
  String incoming = "";
 
  while (LoRa.available())
  {
    incoming += (char)LoRa.read();
  }
 
  if (incomingLength != incoming.length()) 
  {   
    // check length for error
    Serial.println("erro!: o tamanho da mensagem nao condiz com o conteudo!");
    return;                        
  }
 
  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xFF)
  {
    Serial.println("This message is not for me.");
    return;                             // skip rest of function
  }
 
  // Caso a mensagem seja para este dispositivo, imprime os detalhes
  Serial.println("Recebido do dispositivo: 0x" + String(sender, HEX));
  Serial.println("Enviado para: 0x" + String(recipient, HEX));
  Serial.println("ID da mensagem: " + String(incomingMsgId));
  Serial.println("Tamanho da mensagem: " + String(incomingLength));
  Serial.println("Mensagem: " + incoming);
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println();
}

void enviaServidor(String msg){
   if(WiFi.status()== WL_CONNECTED){   //Check WiFi connection status
   HTTPClient http;   
 
   http.begin("http://jsonplaceholder.typicode.com/posts");  //Specify destination for HTTP request
   http.addHeader("Content-Type", "text/plain");             //Specify content-type header
 
   int httpResponseCode = http.POST("POSTING from ESP32");   //Send the actual POST request
 
   if(httpResponseCode>0){
    String response = http.getString();                       //Get the response to the request
 
    Serial.println(httpResponseCode);   //Print return code
    Serial.println(response);           //Print request answer
 
   }else{
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
   }
   http.end();  //Free resources
 
 }else{
    Serial.println("Error in WiFi connection");
 }
  //delay(10000);  //Send a request every 10 seconds
}
