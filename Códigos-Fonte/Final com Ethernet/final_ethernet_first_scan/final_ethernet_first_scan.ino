/**
 * Código Fonte Final com módulo Ethernet
 * Nessa versão, a lista do servidor é baixada primeira e fica armazenada na memória.
 * Tornando assim MUITO mais rápida a leitura e verificação dos cartões
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
 * LED VERDE - Pino 2
 * LED VERMELHO - Pino 3
 */

//DECLARAÇÃO DAS BIBLIOTECAS UTILIZADAS NO PROJETO
#include <SPI.h> //Biblioteca que permite a comunicação com dispositivos SPI (Serial Peripheral Interface), sendo o Arduino como dispositivo principal
#include <MFRC522.h> //Biblioteca do Leitor RFID
#include <EtherCard.h> //Biblioteca do Módulo Ethernet


//DECLARAÇÃO DOS PINOS UTILIZADOS NO ARDUINO
#define PINO_SDA 10 //Constante que define onde o pino SDA do leitor RFID está ligado
#define PINO_RST 9 
#define BUZZER 5
#define LEDVERDE 2
#define LEDVERMELHO 3

MFRC522 mfrc522 (PINO_SDA, PINO_RST); //Cria instância MFRC522
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 }; //mac do módulo ethernet
byte Ethernet::buffer[500];
static uint32_t timer;
const char website[] PROGMEM = "192.168.0.102"; //endereçoIP do servidor no Raspberry Pi
String conteudo;        //Armazena nessa variável todas as informações contidas no servidor
unsigned int tamanho;   //variável para guardar o tamanho do retorno do servidor
static char *IDCartao;
static char *ultimoCartao;
unsigned int indiceServidor;

//ESCOPO DAS FUNÇÕES
void lerID ();
void my_callback (byte status, word off, word len);
void acessoLiberado();
void acessoNegado();
void testaValor();

/**
 * Executado uma única vez na inicialização
 */
void setup() {
  conteudo[0] = '\0';
  pinMode (LEDVERDE, OUTPUT);
  pinMode (LEDVERMELHO, OUTPUT);
  pinMode (BUZZER, OUTPUT);
  digitalWrite (LEDVERDE, HIGH);
  digitalWrite (LEDVERMELHO, HIGH);
  Serial.begin(9600);
  Serial.println ("Iniciando setup");

  
  SPI.begin();
  
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
  IDCartao = (char*) calloc (12, sizeof(char));
  ultimoCartao = (char*) calloc (12, sizeof(char));
  
  // Fica num loop enquanto não conseguir acessar os dados do servidor
  while (conteudo[0] == '\0') {
    ether.packetLoop(ether.packetReceive());
    if (millis() > timer) {
      timer = millis() + 5000;
      Serial.println();
      ether.browseUrl(PSTR("/"), "", website, my_callback);
    }
  }
  
  //Pega só a parte final, que é a parte que contém as informações do servidor
  indiceServidor = conteudo.indexOf("SERVIDOR"); //Procura pela palavra SERVIDOR e armazena seu índice 
  int i;
  for (i = 0; conteudo[indiceServidor] != '\0'; i++) { //elimina a parte inicial que são informações inúteis e pega a parte que interessa, os dados dos usuários
    conteudo[i] = conteudo[indiceServidor++];
  }
  if (conteudo[indiceServidor] == '\0') {
      conteudo[++i] = '\0';
  }
  Serial.println("Setup finalizado!");
  Serial.println ("Aproxima");
  IDCartao[0] = '\0';
}

/**
 * Constantemente executada
 */
void loop() {
  digitalWrite (LEDVERDE, LOW);
  digitalWrite (LEDVERMELHO, LOW);
  
  lerID();
  if ((IDCartao[0] != '\0') && (*ultimoCartao != *IDCartao)) { //Se um cartão tiver sido aproximado e sua ID lida com sucesso
    int i = conteudo.indexOf(IDCartao) + 12;
    if (conteudo.indexOf(IDCartao) > 0) { //procura pela ID do cartão no servidor
      digitalWrite (LEDVERDE, HIGH); //Acende o Led Verde indicando que o acesso está liberado
      Serial.println("Acesso liberado!");
      Serial.print ("Bem vindo ");
      while (conteudo[i] != '\n') { //Escreve na Serial o nome do Usuário do Cartão
        Serial.print (conteudo[i++]);
      }
      Serial.print("\nIDCartao: ");
      Serial.println(IDCartao); //Escreve na Serial a ID do cartão
      Serial.print("\nULTIMO CARTAO: ");
      Serial.println(ultimoCartao);
      acessoLiberado();
    }
    else { //ID não encontrada no Servidor
      digitalWrite (LEDVERMELHO, HIGH);
      Serial.println ("ACESSO NEGADO! Não registrado no servidor!");
      Serial.print ("IDCartao: ");
      Serial.println (IDCartao);
      acessoNegado();  
    }
  }  
}

/**
 * Função que faz a leitura do servidor armazenando seu conteudo numa String
 */
void my_callback (byte status, word off, word len) {  
  Ethernet::buffer[off+500] = 0;
  conteudo = (const char*) Ethernet::buffer + off;
}

/**
 * Função que faz a leitura da ID dos cartões do leitor RFID
 */
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
     }
     else {
      //Precisa ser vazio
     }
}

/**
 * ID do cartão foi encontrada no servidor
 * Emite um bip e o LED verde é aceso
 * Dentro dessa função pode ser implementado outras funcionalidades
 * Por exemplo: liberação de uma catraca ou fechadura eletrônica, armazenar os ID's acessados num LOG, remover quais já foram acessados do servidor para que não seja utilizado
 * duas vezes a mesma ID
 */
void acessoLiberado() {
  tone(BUZZER, 1000); //1kHz
  delay (300);
  noTone (BUZZER);
  digitalWrite (LEDVERDE, LOW);
  *ultimoCartao = *IDCartao; //Essa atribuição impede que o cartão seja novamente lido depois que o acesso já fora liberado
  IDCartao[0] = '\0';
}

/**
 * ID do cartão NÃO foi encontrada no servidor
 * Emite dois bips de alerta e o LED vermelho é aceso
 */
void acessoNegado() {
  tone (BUZZER, 1000);
  delay (150);
  noTone (BUZZER);
  delay (150);
  tone (BUZZER, 1000);
  delay (150);
  noTone (BUZZER);
  digitalWrite (LEDVERMELHO, LOW);
  *ultimoCartao = *IDCartao; //Essa atribuição impede que o cartão seja novamente lido depois que o acesso já fora liberado
  IDCartao[0] = '\0';
}
