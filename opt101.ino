#include <SPI.h>
#include "printf.h"
#include "RF24.h"

#define CE_PIN 7
#define CSN_PIN 8
#define TIMEOUT 500 //millis

#define DATA 0
#define RTS 1
#define CTS 2
#define ACK 3

 
RF24 radio(CE_PIN, CSN_PIN);

uint64_t address[2] = { 0x3030303030LL, 0x3030303030LL};
uint8_t origem = 43;
byte payload[5] = {0,1,2,3,4};
byte payloadRX[5] = {0,1,2,3,4};


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial) {
    // some boards need to wait to ensure access to serial over USB
  }

  // initialize the transceiver on the SPI bus
  if (!radio.begin()) {
    Serial.println(F("radio hardware is not responding!!"));
    while (1) {}  // hold in infinite loop
  }

  radio.setPALevel(RF24_PA_MAX);  // RF24_PA_MAX is default.
  radio.setChannel(37);
  radio.setPayloadSize(sizeof(payload));  // float datatype occupies 4 bytes
  radio.setAutoAck(false);
  radio.setCRCLength(RF24_CRC_DISABLED);
  radio.setDataRate(RF24_250KBPS);

  radio.openWritingPipe(address[0]);  // always uses pipe 0
  radio.openReadingPipe(1, address[1]);  // using pipe 1

  //For debugging info
  printf_begin();             // needed only once for printing details
  //radio.printDetails();       // (smaller) function that prints raw register values
  radio.printPrettyDetails(); // (larger) function that prints human readable data
}

void printPacote(byte *pac, int tamanho){
      Serial.print(F("Rcvd "));
      Serial.print(tamanho);  // print the size of the payload
      Serial.print(F(" O: "));
      Serial.print(pac[0]);  // print the payload's value
      Serial.print(F(" D: "));
      Serial.print(pac[1]);  // print the payload's value
      Serial.print(F(" C: "));
      Serial.print(pac[2]);  // print the payload's value
      Serial.print(F(" i: "));
      Serial.print(pac[3]);  // print the payload's value
      Serial.print(F(" : "));
      for(int i=4;i<tamanho;i++){
        Serial.print(pac[i]);
      }
      Serial.println();  // print the payload's value
}

void envia(int destino, int tipo){
 radio.flush_tx();
 payload[0] = origem;
 payload[1] = destino; 
 payload[2] = tipo;
 unsigned long inicio = millis();
 while(millis() - inicio < TIMEOUT){
   radio.startListening();
   delayMicroseconds(50);
   radio.stopListening();
   if (!radio.testCarrier()) {
     //Serial.println("a");
     if(tipo == ACK || tipo == RTS || tipo == CTS){
      radio.write(&payload[0], 3);
      //printPacote(&payload[0], radio.getPayloadSize());
      Serial.println("Enviando sinal...");
     }else if(tipo == DATA){
      radio.write(&payload[0], 5);
      //printPacote(&payload[0], radio.getPayloadSize());
      //Serial.println("Enviando data...");
     }
     return;
   }else{
     Serial.println("Meio Ocupado");
     delayMicroseconds(270);
   }
   radio.flush_tx();
 }
 Serial.println("TimeOut!");
}

int recebe(int tipo){
  int tamanho;
  radio.startListening();
  unsigned long inicio = millis();
  while(millis() - inicio < TIMEOUT){
    if (radio.available()) {
      delayMicroseconds(160);
      tamanho = radio.getPayloadSize();
      radio.read(&payloadRX[0], tamanho);
      if (payloadRX[1] == origem){
        //Serial.println("é pra mim");
        if (payloadRX[2] == 0){
          //printPacote(&payloadRX[0], tamanho);
        }
        if (payloadRX[2] == tipo){   
          return 1;
        }      
      }
      radio.flush_rx();
    }
  }
  //Serial.println("timeout recebe");
  return 0;
}

void loop() {
  envia(14,RTS);
  if(recebe(CTS)){
    envia(14, DATA);
    if(recebe(ACK)){
      Serial.println("Enviado com sucesso!");
    }
  }
  delay(1000);
}
