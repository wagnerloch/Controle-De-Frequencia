#include "SoftwareSerial.h"
#define REDE "BATCAVERNA" //nome da rede wifi
#define SENHA "C953W029TL" //senha wifi
#define SITE "www.arduinors.esy.es" //defina aqui o site que deseja se conectar

SoftwareSerial esp8266 (2,3);

String enviarComando (String comando, const int tempoResposta, boolean debug);
void configurarWiFi (String SSSID, String Senha);
void mostrarIP ();
void conectarSite (String site);
void lerSite (String comandoSite, int tamanhoString);
void resetarWiFi ();

void setup() {
  String comando  = "GET / HTTP/1.1\r\nHost: "; //22
  comando += SITE;            //20
  comando += "\r\n\r\n\r\n"; //6
  int tamanhoString = 0;
  Serial.begin (9600); //Inicia as seriais
  esp8266.begin (9600);
  tamanhoString = comando.length (); //pega o tamanho da string
  Serial.println ("Seriais iniciadas!");
  Serial.print ("Tamanho String:");
  Serial.println (tamanhoString);
  //resetarWiFi ();
  conectarWiFi (REDE, SENHA);
  mostrarIP ();
  conectarSite (SITE);
  lerSite (comando, tamanhoString);
  Serial.println ("\n\nFim Setup!");
}

void loop() {
  //programa loop
}

String enviarComando (String comando, const int tempoResposta, boolean debug) {
  String resposta = "";
  esp8266.print (comando);
  long int time = millis();
  while ((time + tempoResposta) > millis()) {
    while (esp8266.available()) {
      char c = esp8266.read();
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

void lerSite (String comandoSite, int tamanhoString) {
  String cmd = "";
  cmd = "AT+CIPSEND=";
  cmd += tamanhoString; //tamanho da string tem que ser EXATAMENTE o tamanho do COMANDO
  cmd += "\r\n";
  enviarComando (cmd, 1000, true);
  enviarComando (comandoSite, 1000, true); //aqui baixa todas as informações contidas na URL COMANDO.
  return;
}

void resetarWiFi () {
  enviarComando ("AT+RST\r\n", 1000, true); //Faz reset no ESP8266
  return;
}

