// Comunicação LoRa com ESP-32
// Desenvolvido na Disciplina de Sistemas Embarcados e Tempo Real
// ------------------------------------------------------------

// Definicao das bibliotecas a serem utilizadas no projeto

#include <SPI.h> // do lora             
#include <LoRa.h> // do lora
#include <TinyGPS++.h> // do gps

// Fim Definição bibliotecas ------------------------------

// Definição portas ESP32
#define RXD 16 // definição pinagem gps
#define TXD 17 // definição pinagem gps

// Definição variáveis
TinyGPSPlus gps;
float lat, lon, vel;
unsigned long data, hora;
unsigned short sat;
unsigned int counter = 0;

#define ledGPS 33
#define ledLora 25

TaskHandle_t Task1;
TaskHandle_t Task2;

// Definicacao de constantes
const int csPin = 5;         // Chip Select (Slave Select do protocolo SPI) do modulo Lora
const int resetPin = 0;       // Reset do modulo LoRa
const int irqPin = 4;         // Pino DI0

String outgoing;              // outgoing message

byte localAddress = 0xDD;     // Endereco deste dispositivo LoRa
byte msgCount = 0;            // Contador de mensagens enviadas
byte destination = 0xBB;      // Endereco do dispositivo para enviar a mensagem (0xFF envia para todos devices)
long lastSendTime = 0;        // TimeStamp da ultima mensagem enviada
int interval = 5000;          // Intervalo em ms no envio das mensagens (inicial 5s)
void piscaLed(int pinoPorta);

void setup() { // INÍCIO SETUP

  Serial.begin(115200); // Definição velocidade (Bauds) de comunicação do Serial ESP32
  Serial2.begin(9600 , SERIAL_8N1, RXD, TXD); // Definição velocidade (Bauds) de comunicação do Serial GPS
  pinMode(ledGPS, OUTPUT);
  pinMode(ledLora, OUTPUT);
  Serial.println(" Comunicacao LoRa - Sistemas Embarcados e Tempo Real");

  // override the default CS, reset, and IRQ pins (optional)
  LoRa.setPins(csPin, resetPin, irqPin);// set CS, reset, IRQ pin

  // Inicializa o radio LoRa em 915MHz e checa se esta ok!
  if (!LoRa.begin(915E6)) {
    Serial.println(" Erro ao iniciar modulo LoRa. Verifique a coenxao dos seus pinos!! ");
    while (true);
  }


  Serial.println(" Modulo LoRa iniciado com sucesso!!!");


  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
    Task1code,   /* Task function. */
    "Task1",     /* name of task. */
    10000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    1,           /* priority of the task */
    &Task1,      /* Task handle to keep track of created task */
    0);          /* pin task to core 0 */
  delay(500);

  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
    Task2code,   /* Task function. */
    "Task2",     /* name of task. */
    10000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    1,           /* priority of the task */
    &Task2,      /* Task handle to keep track of created task */
    1);          /* pin task to core 1 */
  delay(500);
  Serial.println("Setup Completado!");

} // FIM DO SETUP


void Task1code( void * pvParameters ) {
  
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());
  for (;;) {
    delay(1);
    // verifica se temos o intervalo de tempo para enviar uma mensagem
    if (millis() - lastSendTime > interval) {
      if (gps.location.isValid() && gps.location.isUpdated() && TinyGPSPlus::distanceBetween(gps.location.lat(), gps.location.lng(), 0.00000, 0.00000) > 0.00) {
        piscaLed(ledGPS);
        String mensagem = String(gps.location.lat(), 6) + ";" + String(gps.location.lng(), 6) + ";" + printDateTime(gps.date, gps.time);
        Serial.println("Enviando " + mensagem);
        sendMessage(mensagem);
        Serial.println();
      }
      lastSendTime = millis();            // Timestamp da ultima mensagem
    }
    // parse for a packet, and call onReceive with the result:
    onReceive(LoRa.parsePacket());
  }
}

void Task2code( void * pvParameters ) {
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());
  for (;;) {    
    while (Serial2.available()) {
      gps.encode(Serial2.read());
      //Serial.print("SATELITES= "); Serial.println(gps.satellites.value());
    }
    delay(1);
  }
}
void loop() { 
} 


// Funcao que envia uma mensagem LoRa
void sendMessage(String outgoing) {
  LoRa.beginPacket();                   // Inicia o pacote da mensagem
  LoRa.write(destination);              // Adiciona o endereco de destino
  LoRa.write(localAddress);             // Adiciona o endereco do remetente

  LoRa.write(outgoing.length());        // Tamanho da mensagem em bytes
  LoRa.print(outgoing);                 // Vetor da mensagem
  LoRa.endPacket();                     // Finaliza o pacote e envia
  msgCount++;                           // Contador do numero de mensagnes enviadas
}

// Funcao para receber mensagem
void onReceive(int packetSize) {
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
  if (recipient != localAddress && recipient != 0xCC)
  {
    Serial.println("This message is not for me.");
    return;                             // skip rest of function
  }

  // Caso a mensagem seja para este dispositivo, imprime os detalhes
  Serial.println("Recebido do dispositivo: 0x" + String(sender, HEX));
  Serial.println("Enviado para: 0x" + String(recipient, HEX));

  Serial.println("Tamanho da mensagem: " + String(incomingLength));
  Serial.println("Mensagem: " + incoming);
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println();

  if(incoming.indexOf("OK")>-1){
    piscaLed(ledLora);
  }


}
String printDateTime(TinyGPSDate &d, TinyGPSTime &t)
{
  String dateTime;
  if (!d.isValid() && !t.isValid())
  {
    Serial.print(F("**** "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d;", d.year(), d.month(), d.day());
    dateTime = String(sz);
    char sv[32];
    sprintf(sv, "%02d:%02d:%02d", t.hour(), t.minute(), t.second());
    dateTime += String(sv);
  }
  return dateTime;
}

void piscaLed(int pinoPorta) {
  digitalWrite(pinoPorta, HIGH);
  delay (50);
  digitalWrite(pinoPorta, LOW);
}
