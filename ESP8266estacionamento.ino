/*------------------------ BIBLIOTECAS ------------------------*/
#include <SPI.h> // Biblioteca SPI
#include <MFRC522.h> // Biblioteca Leitor RF RC522
#include <ESP8266WiFi.h> // Biblioteca do Módulo Wifi
#include <ESP8266SMTP.h> // Biblioteca do Modo SMTP para enviar email
#include <Servo.h>  

/*------------------------ DEFINES ------------------------*/

#define RST_PIN    D3    // Define o pino reset do leitor RF
#define SS_PIN     D8 

/*------------------------ OBJETOS ------------------------*/
MFRC522 mfrc522(SS_PIN, RST_PIN); //Cria um objeto leitor de Cartao RF
Servo servo; // Declara o objeto do tipo servo

/*------------------------ VARIAVEIS GLOBAIS ------------------------*/
const char* ssid = "802.11";                // Nome do Wifi
const char* password = "senha";           // Senha Wifi
uint8_t connection_state = 0;           // Variavel auxiliar para controlar estado da conecxão
uint16_t reconnect_interval = 10000;    // tempo de 10 segundos para uma reconexão

String email = "";

String nome="";


/*------------------------ ESCOPO DAS FUNÇÕES ------------------------*/
void enviaEmail(String email,String nome);
void cancelaSobe(); // Declaração da função que controla a cancela
void cancelaDesce(); // Declaração da função que controla a cancela

/*----------------------------- CONECTANDO AO WIFI -----------------------------*/
uint8_t WiFiConnect(const char* ssID, const char* nPassword)
{
  static uint16_t attempt = 0;
  Serial.print("Conectando em:   ");
  Serial.println(ssID);
  WiFi.begin(ssID, nPassword);  // COnecta ao wifi usando a biblioteca do WIFI

  uint8_t i = 0;
  while(WiFi.status() != WL_CONNECTED && i++ < 50) {  // Faz o controle da concexão, tempo e etc
    delay(200);
    
  }
  ++attempt;
  Serial.println("");
  if(i == 51) {
    Serial.print(F("Tempo de Conexão estourou na tentativa numero: "));
    Serial.println(attempt);
    if(attempt % 2 == 0)
      Serial.println(F("Verifique se o ponto de acesso esta disponível ou SSID e senha estão corretos\r\n"));
    return false;
  }
  Serial.println(F("Conexão estabilizada"));
  Serial.print(F("Seu endereço IP: "));
  Serial.println(WiFi.localIP());
  return true;
}

void Awaits(uint16_t interval)
{
  uint32_t ts = millis();
  while(!connection_state){
    delay(50);
    if(!connection_state && millis() > (ts + interval)){
      connection_state = WiFiConnect(ssid, password);
      ts = millis();
    }
  }
}

/*----------------------------- SETUP -----------------------------*/

void setup()
{
  Serial.begin(9600);
 servo.attach(D1); // PWM saindo do pino D1

  servo.write(70); // Inicia o servo em 70 // Abaixada
  SPI.begin();      // Inicia  SPI bus
  mfrc522.PCD_Init();   // Inicia o lietor de cartao MFRC522
  delay(2000);
 
  
  connection_state = WiFiConnect(ssid, password);

  if(!connection_state) {         // Se nao esta conectado ao wifi
    Awaits(reconnect_interval);         // constantly trying to connect
  }

Serial.println("Aproxime o seu cartao do leitor...");  // Se wifi conectado e estavel aproxime emite a mensagem para aproximar o cartao RFID
Serial.println();

  
}

void loop()
{
  // Procura por cartao RFID
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Seleciona o cartao RFID
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }

  String conteudo= "";
  byte letra;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
    // Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");  // Printa UID do cartao rfid
    // Serial.print(mfrc522.uid.uidByte[i], HEX);                  // Printa UID
     conteudo.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ")); // Adiciona o valor lido numa string conteudo lido para uma string como  0 ou em branco
     conteudo.concat(String(mfrc522.uid.uidByte[i], HEX));               //// // Concateta conteudo lido para uma string em Hexadecimal
  }
  Serial.println();
  Serial.print("Mensagem : ");
  conteudo.toUpperCase();
  


   if (conteudo.substring(1) == "ED 69 7B 89") //UID 1 - Cartao Lucas Steffens
  {
    Serial.println("Liberado NOme Primeira pessoa !");
    Serial.println();
      cancelaSobe();
    email="e-mail@gmail.com";
    nome= "Lucas Steffens";
    enviaEmail(email,nome);
    
   cancelaDesce();
   

  }

  else if(conteudo.substring(1)=="33 16 53 73"){ //UID 2 - Cartao chaveiro campus

    Serial.println("Liberado Nome da Segunda Pessoa cadastrada  !");
    Serial.println();
     cancelaSobe();
    email="e-mail@gmail.com";
    nome= "Nome da Segunda Pessoa cadastrada";
    enviaEmail(email,nome);
     cancelaDesce();
    
    
    }

     else if(conteudo.substring(1)=="90 C1 64 A3"){ //UID 3 - Cartao chaveiro pendrive

    Serial.println("Liberado Nome 3 pessoa !");
    Serial.println();
     cancelaSobe();
    email="e-mail@gmail.com";
    nome= " NOme";
    enviaEmail(email,nome);
     cancelaDesce();
    
    
    }
  else{
    Serial.println("Cliente Desconhecido !");
    Serial.println();
     delay(5000);
    
    }  
  
  
  
  }



void enviaEmail(String email, String nome){
  
    
  SMTP.setEmail("e-mail@gmail.com") // Email que ira mandar o e-mail  // Com as configurações SMTP realizadas no e-mail
    .setPassword("senha")  // Senha do e-mail que ira mandar o e-mail
    .Subject("Entrada no Estacionamento") // Assunto
    .setFrom("Estacionamento LabArq") // Nome do remetente
    .setForGmail();           // simply sets port to 465 and setServer("smtp.gmail.com");           
                                 
  if(SMTP.Send(email, nome+" Foi cobrado em seu cartão o valor de R$ 10,00")) {  // Emails separados por virgula e texto a ser enviado.
    Serial.println(F("Email enviado!"));
  } else {
    Serial.print(F("Erro ao Enviar e-mail: "));
    Serial.println(SMTP.getError());
  } 
  
  
  }


  /*------------------  FUNCÇÕES DE CONTROLE SERVO ------------------------------*/

void cancelaSobe(){
  
   servo.write(0); // Faz a cancela subir
  
  }


  void cancelaDesce(){
     servo.write(70); // Faz a cancela descer
    
    }
