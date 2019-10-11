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

TaskHandle_t Task1;
TaskHandle_t Task2;

String fifo[256];
int fifo_tail;
int fifo_head;
int fifo_n_data;

#define FIFO_MAX 256;

int fifo_data_isavailable();
int fifo_data_isfull();
int fifo_push(String data);
String fifo_pull(void);

float lat, lon, vel;
unsigned long data, hora;
unsigned short sat;
const char* ssid = "esp";
const char* password =  "petfinder2019";
void piscaLed(int pinoPorta);
long lastSendTime = 0;
int interval = 5000;


// Definicacao de constantes
const int csPin = 5;         // Chip Select (Slave Select do protocolo SPI) do modulo Lora
const int resetPin = 0;       // Reset do modulo LoRa
const int irqPin = 4;         // Pino DI0

String outgoing;              // outgoing message

byte localAddress = 0xBB;     // Endereco deste dispositivo LoRa
byte destination = 0xFF;      // Endereco do dispositivo para enviar a mensagem (0xFF envia para todos devices)

// Setup do Microcontrolador
void setup()
{
  // inicializacao da serial
  Serial.begin(115200);
  Serial2.begin(9600 , SERIAL_8N1, RXD, TXD);
  while (!Serial);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) { //Check for the connection
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println("Connected to the WiFi network");

  Serial.println(" Comunicacao LoRa Duplex - Ping&Pong ");

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


  pinMode(porta, OUTPUT);
  digitalWrite(porta, LOW);
}


void piscaLed(int pinoPorta) {
  digitalWrite(pinoPorta, HIGH);
  delay (500);
  digitalWrite(pinoPorta, LOW);
}



//Task1code: blinks an LED every 1000 ms
void Task1code( void * pvParameters ) {
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  for (;;) {

    delay(1000);
   
    if (fifo_data_isavailable()) {
       Serial.print("data retirada? ");
      Serial.println(String(fifo_pull()));
    }
  }
}

//Task2code: blinks an LED every 700 ms
void Task2code( void * pvParameters ) {
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());
  int op = 500;
  for (;;) {
    op++;
    delay(600);
    //Serial.println("inserindo:  " + String(op));
    //fifo_push(String(op));
  }
}



// Loop do microcontrolador - Operacoes de comunicacao LoRa
void loop() {
  if (millis() - lastSendTime > interval) {
    //enviaServidor("parameter=value&also=another");
    lastSendTime = millis();            // Timestamp da ultima mensagem
  }

  // parse for a packet, and call onReceive with the result:
  onReceive(LoRa.parsePacket());
}

// Funcao que envia uma mensagem LoRa
void sendMessage(String outgoing) {
  LoRa.beginPacket();                   // Inicia o pacote da mensagem
  LoRa.write(destination);              // Adiciona o endereco de destino
  LoRa.write(localAddress);             // Adiciona o endereco do remetente
  LoRa.write(outgoing.length());        // Tamanho da mensagem em bytes
  LoRa.print(outgoing);                 // Vetor da mensagem
  LoRa.endPacket();                     // Finaliza o pacote e envia
}


// Funcao que envia uma mensagem LoRa
void sendOk(String outgoing, byte destino) {
  LoRa.beginPacket();                   // Inicia o pacote da mensagem
  LoRa.write(destino);              // Adiciona o endereco de destino
  LoRa.write(localAddress);             // Adiciona o endereco do remetente
  LoRa.write(outgoing.length());        // Tamanho da mensagem em bytes
  LoRa.print(outgoing);                 // Vetor da mensagem
  LoRa.endPacket();                     // Finaliza o pacote e envia
}

// Funcao para receber mensagem
void onReceive(int packetSize) {
  if (packetSize == 0) return;          // Se nenhuma mesnagem foi recebida, retorna nada

  // Leu um pacote, vamos decodificar?
  int recipient = LoRa.read();          // Endereco de quem ta recebendo
  byte sender = LoRa.read();            // Endereco do remetente
  byte incomingLength = LoRa.read();    // Tamanho da mensagem

  String incoming = "";

  while (LoRa.available())  {
    incoming += (char)LoRa.read();
  }

  if (incomingLength != incoming.length())  {
    // check length for error
    Serial.println("erro!: o tamanho da mensagem nao condiz com o conteudo!");
    return;
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xFF)  {
    Serial.println("This message is not for me.");
    return;                             // skip rest of function
  }

  // Caso a mensagem seja para este dispositivo, imprime os detalhes
  Serial.println("Recebido do dispositivo: 0x" + String(sender, HEX));
  Serial.println("Enviado para: 0x" + String(recipient, HEX));
  Serial.println("Mensagem: " + incoming);
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
  fifo_push("id:0x"+String(sender, HEX)+";"+incoming);
  sendOk("OK, Gatway 0xBB recebeu a mensagem!", sender);
  Serial.println();
  piscaLed(porta);
}

void enviaServidor(String msg) {
  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
    HTTPClient http;

    http.begin("http://192.168.137.1/saveinfo.php");  //Specify destination for HTTP request
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    int httpResponseCode = http.POST(msg);   //Send the actual POST request

    if (httpResponseCode > 0) {
      String response = http.getString();                       //Get the response to the request

      Serial.println("repondecode: " + String(httpResponseCode)); //Print return code
      Serial.println("responde: " + String(response));         //Print request answer

    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }
    http.end();  //Free resources

  } else {
    Serial.println("Error in WiFi connection");
  }
  //delay(10000);  //Send a request every 10 seconds
}

int fifo_data_isavailable() {
  if (fifo_n_data > 0)  {
    return 1;
  }  else  {
    return 0;
  }

}

int fifo_data_isfull() {
  if (fifo_n_data < 256) {
    return 0;
  } else {
    return 1;
  }
}

int fifo_push(String data) {
    if (!fifo_data_isfull())  {
      fifo[fifo_head] = data;
      if (fifo_head < 255)    {
        fifo_head ++;
      }    else    {
        fifo_head = 0;
      }

      fifo_n_data ++;
      return 1;
    }  else  {
      return 0;
    }

  }

String fifo_pull(void) {
    String data;
    if (fifo_data_isavailable())  {
      data = fifo[fifo_tail];
      if (fifo_tail < 255)    {
        fifo_tail ++;
      }    else    {
        fifo_tail = 0;
      }
      fifo_n_data --;
      return data;
    }
    return "-1";
  }
