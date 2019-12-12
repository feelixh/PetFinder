// Comunicação LoRa com Arduino
// Definicao das bibliotecas a serem utilizadas no projeto
#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <TinyGPS++.h>

#define RXD 16
#define TXD 17

#define maxToSent 5
int porta = 15;

TinyGPSPlus gps;

TaskHandle_t Task1;
TaskHandle_t Task2;

#define FIFO_MAX 512
String fifo[FIFO_MAX];
volatile int fifo_tail;
volatile int fifo_head;
volatile int fifo_n_data;
String bpFifo[FIFO_MAX];


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
void setup() {
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
  delay (50);
  digitalWrite(pinoPorta, LOW);
}

void Task1code( void * pvParameters ) {
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());
  for (;;) {
    delay(1000);
    if (fifo_n_data >= maxToSent) {
      String post = "";
      int msg = 0;
      while (fifo_n_data != 0) {
        String ms = fifo_pull();
        post += String(msg) + "=" + ms + "&";
        bpFifo[msg] = ms;
        msg++;
      }
      Serial.print("data post: ");
      Serial.println(post.substring(0, post.lastIndexOf("&")));
      if (enviaServidor(post.substring(0, post.lastIndexOf("&")))) {
        Serial.println("Enviado com sucesso para o banco de dados!");
      } else {
        //tratar exceção aqui
        for (int i = 0; i < FIFO_MAX; i++) {
          if (bpFifo[i].length() != 0) {
            fifo_push(bpFifo[i]);
            bpFifo[i] = "";
          } else {
            break;
          }
        }
        Serial.println("Erro ao enviar para o banco!");
      }
    }
  }
}

void Task2code( void * pvParameters ) {
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());
  for (;;) {
    // parse for a packet, and call onReceive with the result:
    onReceive(LoRa.parsePacket());
  }
}

void loop() {
  /*if (millis() - lastSendTime > interval) {
    lastSendTime = millis();            // Timestamp da ultima mensagem
    }
  */
}

// Funcao que envia uma mensagem LoRa
void sendOk(String outgoing, byte destino) {
  LoRa.beginPacket();                   // Inicia o pacote da mensagem
  LoRa.write(destino);                  // Adiciona o endereco de destino
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
  if (fifo_data_isfull() == 1) {
    sendOk("Congestionamento!", sender);
  } else {
    fifo_push("0x" + String(sender, HEX) + ";0x" + String(localAddress, HEX) + ";" + incoming);
    sendOk("OK, Gateway 0xBB recebeu a mensagem!", sender);
  }
  Serial.println();
  piscaLed(porta);
}

boolean enviaServidor(String msg) {
  boolean res = false;
  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
    HTTPClient http;
    http.begin("http://192.168.137.1/saveinfo.php");  //Specify destination for HTTP request
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpResponseCode = http.POST(msg);   //Send the actual POST request
    if (httpResponseCode > 0) {
      String response = http.getString();                       //Get the response to the request
      Serial.println("reponsecode: " + String(httpResponseCode)); //Print return code
      Serial.println("response: " + String(response));         //Print request answer
      if (String(response) == "ok") {
        res = true;
      } else if (String(response) == "err") {
        res = false;
      }
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
      res = false;
    }
    http.end();  //Free resources
  } else {
    WiFi.begin(ssid, password);
    delay(200);
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Connected to the WiFi network");
    } else {
      Serial.println("Error in WiFi connection");
    }
    res = false;
  }
  return res;
}

int fifo_data_isavailable() {
  if (fifo_n_data > 0)  {
    return 1;
  }  else  {
    return 0;
  }
}

int fifo_data_isfull() {
  if (fifo_n_data < FIFO_MAX) {
    return 0;
  } else {
    return 1;
  }
}

int fifo_push(String data) {
  if (!fifo_data_isfull())  {
    fifo[fifo_head] = data;
    if (fifo_head < FIFO_MAX - 1)    {
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
    if (fifo_tail < FIFO_MAX - 1)    {
      fifo_tail ++;
    }    else    {
      fifo_tail = 0;
    }
    fifo_n_data --;
    return data;
  }
  return "-1";
}
