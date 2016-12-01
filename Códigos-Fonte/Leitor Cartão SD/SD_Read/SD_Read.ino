/*
  LEITURA DE UM ARQUIVO A PARTIR DE UM CARTÃO SD
  ** MOSI - pino 11
  ** MISO - pino 12
  ** CLK - pino 13
  ** CS pino 4
  * 
  * Toda vez que abrir um arquivo, deve fechá-lo antes de abrir outro.
*/

#include <SPI.h>
#include <SD.h>

File arquivo;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; //espera pela porta serial conectar;
  }
  Serial.println ("Inicializando cartao SD...");
  if (!SD.begin (4)) {
    Serial.println ("Falha na inicializacao do Cartao SD!");
    return;
  }
  Serial.println ("Inicializado com sucesso!");

  arquivo = SD.open ("ArquivoTexto.txt", FILE_READ);
  if (arquivo) {
    Serial.println ("Arquivo Texto aberto com sucesso!");

    while (arquivo.available()) {
      Serial.write (arquivo.read());
    }
    arquivo.close();
  }

  else {
    Serial.println ("Erro ao abrir arquivo texto!");
  }

}

void loop() {
  // put your main code here, to run repeatedly:

}
