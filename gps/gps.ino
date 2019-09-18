#include <HardwareSerial.h>
#include<TinyGPS.h> 
#include <TinyGPS++.h>
#include <AES.h>//Biblioteca do AES.

#define RXD 16
#define TXD 17

TinyGPS GPS;  
TinyGPSPlus gps;   
                  
 
float lat, lon, vel;
unsigned long data, hora;
unsigned short sat;

AES aes;//Cria a classe aes.
byte key[16], out[16], inp[32];//Cria arrays (vetores) para a chave, input e output de dados.
const char pass[] = "abc";//Define a chave usada, neste exemplo usamos AES128, então precisa ser <= 16 Bytes.
 
void enc128(const char txt[], bool db);

void setup() {

//In the set-up function
Serial.begin(115200);
Serial2.begin(9600 ,SERIAL_8N1, RXD, TXD);
Serial.println("setup complete");
enc128("vida de silicio", 1);//Faz a função de encriptação e retorna o HEX encriptado.

}

void loop() {
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
}

void enc128(const char txt[], bool db){
   if (strlen(pass) > 16){
      if (db == true)      {
         Serial.println("Chave para AES128 <= 16 Bytes");
      }
      return;//Se a chave for maior, irá sair da função.
   }
 
   if (strlen(txt) > 16)//Verifica se o texto tem o tamanho limite de 16 caracteres.
   {
      if (db == true)      {
         Serial.println("Frase/numero para AES <= 16 Bytes / bloco");
      }
      return;//Se o texto for maior, irá sair da função.
   }
 
   for (byte i = 0; i < strlen(pass); i++)//Adiciona a chave(pass) na array key.
   {
      key[i] = pass[i];
   }
 
   for (byte i = 0; i < strlen(txt); i++)//Adiciona o texto na array input.
   {
      inp[i] = txt[i];
   }
 
   //Adiciona a chave ao algoritimo.
   if (aes.set_key(key, 16) != 0)//Verifica se a chave esta correta, caso nao, sairá da função.
   {
      if (db == true)
      {
         Serial.println("Erro ao configurar chave");
      }
      return;//Sai da função
   }
 
   //Faz a encriptação da array INPUT e retorna o HEXA na array OUTPUT.
   if (aes.encrypt(inp, out) != 0)//Verifica se a encriptação esta correta, se não, sairá da função.
   {
      if (db == true)
      {
         Serial.println("Erro ao encriptar");
      }
      return;//Sai da função
   }
 
   if (db == true)//Se o debug estiver on (1), irá mostrar o HEXA no serial monitor.
   {
      for (byte i = 0; i < 16; i++)
      {
         Serial.print(out[i], HEX);
         Serial.print(" ");
      }
      Serial.println();
   }
 
   aes.clean();//Limpa a chave e residuos sensiveis da encriptação.
}
