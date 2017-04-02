#include <EtherCard.h>

static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };

byte Ethernet::buffer[500];
BufferFiller bfill;
static uint32_t timer;

/**
 * Endereço IP do servidor no Raspberry Pi
 */
const char website[] PROGMEM = "192.168.0.104";

String conteudo;        //Armazena nessa variável todas as informações contidas no servidor
unsigned int tamanho;   //variável para guardar o tamanho do retorno do servidor

/**
 * Função que faz a leitura do servidor armazenando seu conteudo numa String
 */
static void my_callback (byte status, word off, word len) {  
  Serial.println ("BUSCANDO...");
  Ethernet::buffer[off+500] = 0;

  conteudo = (const char*) Ethernet::buffer + off;
  tamanho = conteudo.length();
  if (conteudo.indexOf("10 D6 DF C1") != -1) {
    Serial.println("ACHOU");
    for(;;) {
      
    }
  }
  else {
    Serial.println ("NAO ACHOU!");  
  }
}

void setup() {
  timer = 0;
  Serial.begin(9600);
  Serial.println("Inicializando Modulo Ethernet");
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
  Serial.println("Setup finalizado!");
}

void loop() {
  ether.packetLoop(ether.packetReceive());
  if (millis() > timer) {
    Serial.println("IF");
    timer = millis() + 5000;
    Serial.println();
    ether.browseUrl(PSTR("/"), "", website, my_callback);
  }
}
