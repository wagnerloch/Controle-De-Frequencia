/*Programa Acesso com Cartão
Autor: Wagner Loch
Curso de Engenharia de Computação / UFPel
2016/1
*/

//DECLARAÇÃO DAS BIBLIOTECAS
#include <SPI.h>  //biblioteca que permite comunicação com dispositivos SPI (Serial Peripheral Interface), sendo o Arduino como dispositivo principal
#include <MFRC522.h> //biblioteca leitor RFID

#include <SoftwareSerial.h>

//DECLARAÇÃO DOS PINOS UTILIZADOS NO ARDUINO
#define SS_PIN 10
#define RST_PIN 9
#define BUZZER 7    //constante que define onde o buzzer está ligado
#define BUTTON 8    //constante que dfine onde o pushbutton está ligado (para fazer cadastro de um novo cartão) 
//#define LED_VERMELHO 6
//#define LED_VERDE 5

SoftwareSerial esp8266(2, 3);

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Cria instância MFRC522

//ESCOPO FUNÇÕES

String lerID (int comando);
int verificarCartao (String dados);
void cadastrarCartao ();
void acessarCartao ();
void beep (int valor);

//VARIÁVEIS GLOBAIS
char string[20];
int quantidadeCartoesCadastrados;

typedef struct Cartao {
  String ID;
  int check;
} Cartao;

Cartao cartoes[10];

//FUNÇÃO SETUP, EXECUTA APENAS QUANDO O ARDUINO É LIGADO
void setup() {
  pinMode (BUZZER, OUTPUT); //define o pino 7 (onde está o buzzer) como saída digital
  pinMode (BUTTON, INPUT);  //define o pino 6 (onde está o pushbutton) como entrada digital
  //pinMode (LED_VERMELHO, OUTPUT);
  //pinMode (LED_VERDE, OUTPUT);
  Serial.begin(9600);   // Inicia a serial
  SPI.begin();      // Inicia  SPI bus
  mfrc522.PCD_Init();   // Inicia MFRC522 (leitor RFID)
  quantidadeCartoesCadastrados = -1;
  //digitalWrite (LED_VERMELHO, LOW);
  //digitalWrite (LED_VERDE, LOW);
  Serial.println ("Setup OK!");
}

void loop() {
  /*int valorButton = 0;

  //if (esp8266.available()) { //verifica se o módulo wifi está disponível
    //if (esp8266.find("+IPD,")) {
      //int connectionId = esp8266.read() - 48;
      valorButton = digitalRead(BUTTON); // ler a entrada de valor
      if (valorButton == LOW) { //botão sendo pressionado
        cadastrarCartao();
      }
      else {
        acessarCartao();
      }
    //}
  //}*/
  String cartao = lerID(1);
}

void cadastrarCartao() {
  Serial.println("Aproxime um cartao para cadastro...");
  int cards; //apenas para exibir o valor correto de cartões cadastrados
  int erro = 0;
  String cartaoAtual;
  
  cartaoAtual = lerID(1);      
  erro = verificarCartao (cartaoAtual);

  if (erro == 1) {
    beep (0);
    Serial.println ("");
    Serial.println ("Erro! Cartao ja cadastrado");
    return;
  }
  else {
    quantidadeCartoesCadastrados++;
    cards = quantidadeCartoesCadastrados + 1;
    cartoes[quantidadeCartoesCadastrados].ID = cartaoAtual;
    beep (1);
    Serial.println ("");
    Serial.println ("Cartao cadastrado com sucesso!");
    Serial.print ("Quantidade de cartoes cadastrados: ");
    Serial.println (cards);
    delay (1000);
    return;
  }
}
//FUNÇÃO QUE LÊ A ID DO CARTÃO, PODE SER USADA PARA ACESSAR OU PARA CADASTRAR
String lerID (int comando) {
  Serial.println ("Aproxime um cartao");
  if (comando == 1) { //cadastro de cartão
    while (true) {
      if (mfrc522.PICC_IsNewCardPresent()) {
        if (mfrc522.PICC_ReadCardSerial()) {
          //Serial.print("UID da tag:");
          String conteudo = "";
          byte letra;
          for (byte i = 0; i < mfrc522.uid.size; i++) {
            Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
            Serial.print(mfrc522.uid.uidByte[i], HEX);
            conteudo.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
            conteudo.concat(String(mfrc522.uid.uidByte[i], HEX));
          }
          conteudo.toUpperCase();
          return conteudo.substring(1);
          Serial.print ("Cartao: ");
          Serial.println (conteudo.substring(1)); 
        }
      }
    }
  }
  else {  //leitura de cartão
    while (true) {
      if (mfrc522.PICC_IsNewCardPresent()) {
        if (mfrc522.PICC_ReadCardSerial()) {
          //Serial.print("UID da tag:");
          String conteudo = "";
          byte letra;
          for (byte i = 0; i < mfrc522.uid.size; i++) {
            //Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
            //Serial.print(mfrc522.uid.uidByte[i], HEX);
            conteudo.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
            conteudo.concat(String(mfrc522.uid.uidByte[i], HEX));
          }
          conteudo.toUpperCase();
          return conteudo.substring(1);
        }
      }
      else {
        return "";
      }
    }
  }
}

//VERIFICA SE O CARTÃO JÁ ESTÁ CADASTRADO, SE ESTIVER, NÃO PODE CADASTRAR NOVAMENTE
int verificarCartao (String dados) {
  int i;
  for (i = 0; i <= quantidadeCartoesCadastrados; i++) {
    if (cartoes[i].ID == dados) {
      return 1;
    }
  }
  return 0;
}

void acessarCartao() {
  int i;
  String cartaoAtual;
  cartaoAtual = lerID(0);
  if (cartaoAtual == "") {
    return;
  }
  for (i = 0; i <= quantidadeCartoesCadastrados; i++) {
    if (cartaoAtual == cartoes[i].ID) {
      if (cartoes[i].check == 1) {  //cartão já usado para acessar a sala
        Serial.println ("Cartao ja usado!");
        beep (0);
        delay (1000);
        return;
      }
      else {
        cartoes[i].check = 1;
        Serial.println ("Acesso liberado!");
        Serial.println ("Bem-Vindo!");
        beep (1);
        delay (1000);
        return;
      }
    }
  }
  return;
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
