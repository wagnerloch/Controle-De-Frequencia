/**
 * Código Fonte Final com módulo Ethernet
 * Autor: Wagner Loch
 * Curso de Engenharia de Computação - UFPel
 * 2017
 */

/**
 * Pinagem Leitor RFID:
 * CLK     - Pino 13
 * MISO    - Pino 12
 * MOSI    - Pino 11
 * SDA     - Pino 10
 * RST     - Pino 09
 * GND     - GND Arduino
 * 3.3     - 3.3v do Arduino
 * NC      - Não Conectado
 */

/**
 * Pinagem Módulo Ethernet:
 * 5v     - Não Conectado
 * LNT    - Não Conectado
 * CLK    - Não Conectado
 * SO     - Pino 12
 * WOL    - Não Conectado
 * SCK    - Pino 13
 * ST     - Pino 11
 * RST    - Não Conectado
 * CS     - Pino 8
 * Q3     - 3.3v do Arduino
 * GND    - GND Arduino
 */

/**
 * Pinagem de Outros componentes:
 * BUZZER  - Pino 7
 */

//DECLARAÇÃO DAS BIBLIOTECAS UTILIZADAS NO PROJETO
#include <SPI.h> //Biblioteca que permite a comunicação com dispositivos SPI (Serial Peripheral Interface), sendo o Arduino como dispositivo principal
#include <MFRC522.h> //Biblioteca do Leitor RFID
#include <EtherCard.h> //Biblioteca do Módulo Ethernet


//DECLARAÇÃO DOS PINOS UTILIZADOS NO ARDUINO
#define PINO_SDA 10 //Constante que define onde o pino SDA do leitor RFID está ligado
#define PINO_RST 9 
#define BUZZER 7

MFRC522 mfrc522 (PINO_SDA, PINO_RST); //Cria instância MFRC522
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 }; //mac do módulo ethernet
byte Ethernet::buffer[500];
static uint32_t timer;
const char website[] PROGMEM = "192.168.0.103"; //endereço IP do servidor no Raspberry Pi
String conteudo;        //Armazena nessa variável todas as informações contidas no servidor
unsigned int tamanho;   //variável para guardar o tamanho do retorno do servidor
char *IDCartao;

//ESCOPO DAS FUNÇÕES
void lerID ();
void my_callback (byte status, word off, word len);
void acessoLiberado();
void acessoNegado();

void setup() {
  Serial.begin(9600);
  Serial.println ("Iniciando setup");

  
  SPI.begin();
  pinMode (BUZZER, OUTPUT);
  mfrc522.PCD_Init(); //Inicia MFRC522 (Leitor RFID)
  delay(100);
  if (ether.begin(sizeof Ethernet::buffer, mymac, 8) == 0) {
    Serial.println("Falha ao acessar o Modulo Ethernet");
  }
  else {
    Serial.println("Modulo Ethernet conectado!");
  }
  if (!ether.dhcpSetup()) {
    Serial.println("DHCP failed");
  }
  ether.printIp("IP:  ", ether.myip);
  ether.printIp("GW:  ", ether.gwip);  
  ether.printIp("DNS: ", ether.dnsip);

  if (!ether.dnsLookup(website)) {
    Serial.println("DNS failed");
  }
  ether.printIp("SRV: ", ether.hisip);
  delay(100);
  IDCartao = (char*) calloc (12, sizeof(char));
  Serial.println("Setup finalizado!");
  Serial.println ("Aproxima");
  *IDCartao = "";
}

void loop() {
  ether.packetLoop(ether.packetReceive());
  //String card = lerID();
  //card.toCharArray(IDCartao, card.length()+1);
  //*IDCartao = lerID();
  lerID();
  if (IDCartao[0] != '\0') { //Se um cartão tiver sido aproximado e sua ID lida com sucesso
    Serial.print ("ID DO CARTAO: ");
    Serial.write (IDCartao);
    delay(100);
    if (millis() > timer) {
      timer = millis() + 5000;
      Serial.println();
      
      ether.browseUrl(PSTR("/"), "", website, my_callback);
    }
  }
  
}

void lerID () {
     if (mfrc522.PICC_IsNewCardPresent ()) {
       if (mfrc522.PICC_ReadCardSerial ()) {
         String conteudorfid = "";
         byte letra;
         for (byte i = 0; i < mfrc522.uid.size; i++) {
           conteudorfid.concat (String (mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
           conteudorfid.concat (String (mfrc522.uid.uidByte[i], HEX));
         }
         conteudorfid.toUpperCase ();
         int i;
         for (i = 0; i < conteudorfid.substring(1).length(); i++) {
          IDCartao[i] = conteudorfid.substring(1)[i];
         }
         i++;
         IDCartao[i] = '\0';
       }
       
     Serial.println(IDCartao); //ID do cartão lida com sucesso
     }
     else {
      IDCartao[0] = '\0';
     }
}

/**
 * Função que faz a leitura do servidor armazenando seu conteudo numa String
 */
void my_callback (byte status, word off, word len) {  
  Ethernet::buffer[off+500] = 0;

  conteudo = (const char*) Ethernet::buffer + off;
  
  String verifica(IDCartao);
  
  if (conteudo.indexOf(verifica) > 0) { //procura pela ID do cartão no servidor
    Serial.println("Acesso liberado");
    Serial.println(verifica);
    Serial.print ("POSICAO: ");
    Serial.println (conteudo.indexOf(verifica));
    acessoLiberado();
  }
  else {
    Serial.println ("ACESSO NEGADO!");
    Serial.print ("CARTAO VERIFICA: ");
    Serial.println (verifica);
    acessoNegado();  
  }
}

void acessoLiberado() {
  tone(BUZZER, 1000); //1kHz
  //digitalWrite (LED_VERDE, HIGH);
  delay (300);
  noTone (BUZZER);
  //digitalWrite (LED_VERDE, LOW);
  *IDCartao = "";
}

void acessoNegado() {
  tone (BUZZER, 1000);
  //digitalWrite (LED_VERMELHO, HIGH);
  delay (150);
  noTone (BUZZER);
  delay (150);
  tone (BUZZER, 1000);
  delay (150);
  noTone (BUZZER);
  //digitalWrite (LED_VERMELHO, LOW);
  *IDCartao = "";
}
