/*
 * Código-Fonte
 * Autor: Wagner Loch
 * Curso de Engenharia de Computação - UFPel
 * 2016
 * 
 * Leitor RFID:
 * CLK - Pino 13
 * MISO - Pino 12
 * MOSI - Pino 11
 * SDA - Pino 10
 * RST - Pino 9
 * GND - GND Arduino
 * 3.3 - 3.3v do Arduino
 * NC - Não conectado
 * 
 * BOTAO - pino 8
 * BUZZER - pino 7
 * 
 * Cartão SD:
 * MOSI - pino 11
 * MISO - pino 12
 * CLK - pino 13
 * CS - pino 4
 * 
 * ESP8266:
 * TX - pino 2                          | GND - GND do Arduino
 * CH_PD - 3.3v com resistore de 1k     | GPIO2 - não conectar
 * RST - não conectado                  | GPIO0 - não conectar
 * 3.3V - 3.3V do Arduino               | RX - pino 3 com divisor de tensão para 3.3V
 * 
 */

//DECLARAÇÃO DAS BIBLIOTECAS
#include <SPI.h> //biblioteca que permite comunicação com dispositivos SPI (Serial Peripheral Interface), sendo o Arduino como dispositivo principal
#include <MFRC522.h> //Biblioteca do Leitor RFID
#include <SoftwareSerial.h>
#include <SD.h>
#include "SoftwareSerial.h"

//DECLARAÇÃO DOS PINOS UTILIZADOS NO ARDUINO
#define PINO_SDA 10
#define PINO_RST 9
#define BUZZER 7    //constante que define onde o buzzer está ligado
#define BUTTON 8    //constante que dfine onde o pushbutton está ligado (para fazer cadastro de um novo cartão) 

String REDE = "BATCAVERNA"; //nome da rede wifi
String SENHA = "C953W029TL"; //senha wifi
String SITE = "192.168.0.102"; //defina aqui o site que deseja se conectar
String COMANDOSITE  = "GET / HTTP/1.1\r\nHost: "; //22
int TAMANHOSTRING;
  

MFRC522 mfrc522 (PINO_SDA, PINO_RST); //Cria instância MFRC522
SoftwareSerial esp8266 (2,3); //Pinos utilizados pelo ESP8266

//ESCOPO DAS FUNÇÕES

String lerID ();
String enviarComando (String comando, const int tempoResposta, boolean debug);
int verificarCartao (String dados);
void cadastrarCartao ();
void acessarCartao ();
void beep (int valor);
int iniciarSD ();
void configurarWiFi (String SSSID, String Senha);
void mostrarIP ();
void conectarSite (String site);
String lerSite (String comandoSite, int tamanhoString);
void resetarWiFi ();
void conectarWiFi (String SSSID, String Senha);
boolean buscarIDNoServidor (String cartaoAtual);

//VARIÁVEIS GLOBAIS
File ARQUIVO;

void setup() {
  COMANDOSITE += SITE;            //20
  COMANDOSITE += "\r\n\r\n\r\n"; //6
  TAMANHOSTRING = COMANDOSITE.length ();
  
  Serial.begin (9600);
  while (!Serial) {
    ; //aguarda até a Serial iniciar
  }
  
  esp8266.begin (9600);
  SPI.begin ();
  pinMode (BUZZER, OUTPUT);
  pinMode (BUTTON, INPUT);
  mfrc522.PCD_Init (); // Inicia MFRC522 (leitor RFID)
  if (iniciarSD () != NULL) {
    conectarWiFi (REDE, SENHA);
  }
  mostrarIP ();
  conectarSite (SITE);
  acessarCartao ();
}

void loop() {
  int valorBotao = digitalRead (BUTTON);
  //acessarCartao ();  
  /*if (valorBotao == HIGH) {
    acessarCartao ();
  }
  else {
    //cadastrarCartao (); //Provável que essa função não exista, então tudo será implementado sem necessidade do if
  }*/
}

String enviarComando (String comando, const int tempoResposta, boolean debug) {
  String resposta = "";
  esp8266.print (comando);
  long int time = millis();
  while ((time + tempoResposta) > millis ()) {
    while (esp8266.available ()) {
      char c = esp8266.read ();
      resposta += c;
    }
  }
  if (debug) {
    Serial.print (resposta);
  }
  return resposta;
}

void conectarWiFi (String SSSID, String Senha) {
  enviarComando ("AT+CWMODE=1\r\n", 1000, true);
  enviarComando ("AT+CIPMUX=0\r\n", 1000, true);
  String cmd = "";
  cmd =  "AT+CWJAP="; //Início da sequência de comandos para se conectar na rede WiFi
  cmd += "\"";
  cmd += SSSID;
  cmd += "\"";
  cmd += ",";
  cmd += "\"";
  cmd += Senha;
  cmd += "\"";
  cmd += "\r\n";
  enviarComando (cmd, 7000, true);
  return;
}

void mostrarIP () {
  enviarComando ("AT+CIFSR\r\n", 1000, true); //mostra endereço IP
  return;
}

void conectarSite (String site) {
  String cmd;
  cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += site;
  cmd += "\",80\r\n";
  enviarComando (cmd, 1000, true); //Conecta no Site
  return;
}

String lerSite (String comandoSite, int tamanhoString) {
  String cmd = "";
  String resposta = "";
  do {
    cmd = "AT+CIPSEND=";
    cmd += tamanhoString; //tamanho da string tem que ser EXATAMENTE o tamanho do COMANDO
    cmd += "\r\n";
    enviarComando (cmd, 1000, true);
    resposta = enviarComando (comandoSite, 1000, true); //aqui baixa todas as informações contidas na URL COMANDO.
    Serial.println ("Lendo Site:");
    Serial.println (resposta);
  } while (resposta.substring(0) != "html"); //não é o mais efetivo
  return resposta;
}

void resetarWiFi () {
  enviarComando ("AT+RST\r\n", 1000, true); //Faz reset no ESP8266
  return;
}

int iniciarSD () {
  Serial.println ("Inicializando cartao SD...");
  if (!SD.begin (4)) {
    Serial.println ("Falha na inicializacao do Cartao SD!");
    return NULL;
  }
  Serial.println ("SD inicializado com sucesso!");

  ARQUIVO = SD.open ("ssid.txt", FILE_READ);
  if (ARQUIVO) {
    Serial.println ("ARQUIVO Texto aberto com sucesso!");

    while (ARQUIVO.available()) { //armazenar numa string os dados de conexão
      Serial.write (ARQUIVO.read());
      REDE = ARQUIVO.read();
    }
    ARQUIVO.close();
  }
  else {
    Serial.println ("Erro ao abrir ARQUIVO texto!");
  }

  ARQUIVO = SD.open ("password.txt", FILE_READ);
  if (ARQUIVO) {
    Serial.println ("ARQUIVO Texto aberto com sucesso!");

    while (ARQUIVO.available()) { //armazenar numa string os dados de conexão
      Serial.write (ARQUIVO.read());
      SENHA = ARQUIVO.read();
    }
    ARQUIVO.close();
  }
  else {
    Serial.println ("Erro ao abrir ARQUIVO texto!");
  }
  return 1;
}

void acessarCartao () {
  String cartaoAtual = lerID ();
  if (cartaoAtual == "") {
    Serial.println ("Nenhum cartao presente!");
    return;
  }
  /*
   * 
   * BUSCAR NO SERVIDOR ID DO CARTÃO
   * 
   */
   boolean sucesso = buscarIDNoServidor (cartaoAtual);
   if (sucesso == true) {
    //acesso liberado
    beep (1);
   }
   else {
    //acesso negado
    beep (2);
   }
   return;
}

String lerID () {
  Serial.println ("Aproxima");
   while (true) {
     if (mfrc522.PICC_IsNewCardPresent ()) {
       if (mfrc522.PICC_ReadCardSerial ()) {
         String conteudo = "";
         byte letra;
         for (byte i = 0; i < mfrc522.uid.size; i++) {
           conteudo.concat (String (mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
           conteudo.concat (String (mfrc522.uid.uidByte[i], HEX));
         }
         conteudo.toUpperCase ();
         Serial.println (conteudo.substring(1));
         return conteudo.substring(1);
       }
     }
     else {
       //return ""; //nenhum cartão presente
     }
   }
}

boolean buscarIDNoServidor (String cartaoAtual) {
  String conteudo = lerSite (COMANDOSITE, TAMANHOSTRING);
  if (conteudo.substring(0) == cartaoAtual) { //busca a partir da primeira posição até a última
    return true;
  }
  else {
    return false;
  }
}

void beep (int valor) {
  if (valor == 1) {  //OK
      tone(BUZZER, 1000); //1kHz
      //digitalWrite (LED_VERDE, HIGH);
      delay (300);
      noTone (BUZZER);
      //digitalWrite (LED_VERDE, LOW);
    }
    else {  //ERRO
      tone (BUZZER, 1000);
      //digitalWrite (LED_VERMELHO, HIGH);
      delay (150);
      noTone (BUZZER);
      delay (150);
      tone (BUZZER, 1000);
      delay (150);
      noTone (BUZZER);
      //digitalWrite (LED_VERMELHO, LOW);
    }
    return;
}

