// Comunicação LoRa com Arduino 
// Definicao das bibliotecas a serem utilizadas no projeto
#include <SPI.h>             
#include <LoRa.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <TinyGPS++.h>

#define RXD 16
#define TXD 17
int porta = 15;

TinyGPSPlus gps;   
                  
float lat, lon, vel;
unsigned long data, hora;
unsigned short sat;
const char* ssid = "yourNetworkName";
const char* password =  "yourNetworkPassword";
void piscaLed(int pinoPorta);



// Definicacao de constantes
const int csPin = 5;         // Chip Select (Slave Select do protocolo SPI) do modulo Lora
const int resetPin = 0;       // Reset do modulo LoRa
const int irqPin = 4;         // Pino DI0
 
String outgoing;              // outgoing message
 
byte localAddress = 0xBB;     // Endereco deste dispositivo LoRa
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

  
pinMode(porta, OUTPUT);
digitalWrite(porta, LOW);
}


void piscaLed(int pinoPorta){
  digitalWrite(pinoPorta, HIGH);
  delay (500);
  digitalWrite(pinoPorta, LOW);
}
 
// Loop do microcontrolador - Operacoes de comunicacao LoRa
void loop(){
  // parse for a packet, and call onReceive with the result:
  onReceive(LoRa.parsePacket());
}
 
// Funcao que envia uma mensagem LoRa
void sendMessage(String outgoing){
  LoRa.beginPacket();                   // Inicia o pacote da mensagem
  LoRa.write(destination);              // Adiciona o endereco de destino
  LoRa.write(localAddress);             // Adiciona o endereco do remetente
  LoRa.write(outgoing.length());        // Tamanho da mensagem em bytes
  LoRa.print(outgoing);                 // Vetor da mensagem 
  LoRa.endPacket();                     // Finaliza o pacote e envia
}


// Funcao que envia uma mensagem LoRa
void sendOk(String outgoing, byte destino){
  LoRa.beginPacket();                   // Inicia o pacote da mensagem
  LoRa.write(destino);              // Adiciona o endereco de destino
  LoRa.write(localAddress);             // Adiciona o endereco do remetente
  LoRa.write(outgoing.length());        // Tamanho da mensagem em bytes
  LoRa.print(outgoing);                 // Vetor da mensagem 
  LoRa.endPacket();                     // Finaliza o pacote e envia
}
 
// Funcao para receber mensagem 
void onReceive(int packetSize){
  if (packetSize == 0) return;          // Se nenhuma mesnagem foi recebida, retorna nada
 
  // Leu um pacote, vamos decodificar? 
  int recipient = LoRa.read();          // Endereco de quem ta recebendo
  byte sender = LoRa.read();            // Endereco do remetente
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
  Serial.println("Mensagem: " + incoming);
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
  sendOk("OK, Gatway 0xBB recebeu a mensagem!", sender);
  Serial.println();
  piscaLed(porta);
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
