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
  Serial.begin(9600);   // Inicia a serial
  SPI.begin();      // Inicia  SPI bus
  mfrc522.PCD_Init();   // Inicia MFRC522 (leitor RFID)
  Serial.println ("Aproxime o cartao");
  lerID (1);
}

void loop() {
  
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
}

