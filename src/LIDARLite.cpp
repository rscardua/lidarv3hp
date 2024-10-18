/*------------------------------------------------------------------------------

  Biblioteca LIDARLite para Arduino
  LIDARLite.cpp

  Esta biblioteca fornece acesso rápido às funções básicas do LIDAR-Lite
  via a interface Arduino. Além disso, pode fornecer a um usuário de qualquer
  plataforma um modelo para seu próprio código de aplicação.

  Copyright (c) 2016 Garmin Ltd. ou suas subsidiárias.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

------------------------------------------------------------------------------*/

#include <Arduino.h>
#include <Wire.h>
#include <stdarg.h>
#include "LIDARLite.h"

// Endereço padrão I2C do LiDAR-Lite v3HP
#define LIDAR_LITE_ADDRESS 0x62
// Registradores do LiDAR-Lite v3HP
#define REGISTER_ACQ_COMMAND 0x00
#define REGISTER_DISTANCE_MSB 0x8f

#define SCL_PIN 22
#define SDA_PIN 21
#define CLOCKSPEED 400000UL


/*------------------------------------------------------------------------------
  Construtor

  Use LIDARLite::begin para inicializar.
------------------------------------------------------------------------------*/
LIDARLite::LIDARLite() {}

/*------------------------------------------------------------------------------
  Begin

  Inicia o sensor e o I2C.

  Parâmetros
  ------------------------------------------------------------------------------
  configuration: Padrão 0. Seleciona uma das várias configurações predefinidas.
  fasti2c: Padrão 100 kHz. Frequência base do I2C.
    Se true, a frequência do I2C é definida para 400kHz.
  lidarliteAddress: Padrão 0x62. Preencha com o novo endereço aqui se alterado. Veja
    o manual de operação para instruções.
------------------------------------------------------------------------------*/
void LIDARLite::begin(int configuration, bool fasti2c, char lidarliteAddress)
{
    
    Wire.begin(SDA_PIN, SCL_PIN, CLOCKSPEED);

    configure(configuration, lidarliteAddress); // Configurações de configuração
} /* LIDARLite::begin */

/*------------------------------------------------------------------------------
  Configure

  Seleciona uma das várias configurações predefinidas.

  Parâmetros
  ------------------------------------------------------------------------------
  configuration:  Padrão 0.
    0: Modo padrão, desempenho equilibrado.
    1: Curto alcance, alta velocidade. Usa contagem máxima de aquisição 0x1d.
    2: Alcance padrão, maior velocidade em curto alcance. Ativa a detecção de
        terminação rápida para medições mais rápidas em curto alcance (com precisão
        reduzida)
    3: Alcance máximo. Usa contagem máxima de aquisição 0xff.
    4: Detecção de alta sensibilidade. Sobrescreve o algoritmo de detecção de medição
        válida padrão, e usa um valor de limiar para alta sensibilidade e ruído.
    5: Detecção de baixa sensibilidade. Sobrescreve o algoritmo de detecção de medição
        válida padrão, e usa um valor de limiar para baixa sensibilidade e ruído.
  lidarliteAddress: Padrão 0x62. Preencha com o novo endereço aqui se alterado. Veja
    o manual de operação para instruções.
------------------------------------------------------------------------------*/
void LIDARLite::configure(int configuration, char lidarliteAddress)
{
    switch (configuration)
    {
    case 0:                                  // Modo padrão, desempenho equilibrado
        write(0x02, 0x80, lidarliteAddress); // Padrão
        write(0x04, 0x08, lidarliteAddress); // Padrão
        write(0x1c, 0x00, lidarliteAddress); // Padrão
        break;

    case 1: // Curto alcance, alta velocidade
        write(0x02, 0x1d, lidarliteAddress);
        write(0x04, 0x08, lidarliteAddress); // Padrão
        write(0x1c, 0x00, lidarliteAddress); // Padrão
        break;

    case 2:                                  // Alcance padrão, maior velocidade em curto alcance
        write(0x02, 0x80, lidarliteAddress); // Padrão
        write(0x04, 0x00, lidarliteAddress);
        write(0x1c, 0x00, lidarliteAddress); // Padrão
        break;

    case 3: // Alcance máximo
        write(0x02, 0xff, lidarliteAddress);
        write(0x04, 0x08, lidarliteAddress); // Padrão
        write(0x1c, 0x00, lidarliteAddress); // Padrão
        break;

    case 4:                                  // Detecção de alta sensibilidade, medições errôneas altas
        write(0x02, 0x80, lidarliteAddress); // Padrão
        write(0x04, 0x08, lidarliteAddress); // Padrão
        write(0x1c, 0x80, lidarliteAddress);
        break;

    case 5:                                  // Detecção de baixa sensibilidade, medições errôneas baixas
        write(0x02, 0x80, lidarliteAddress); // Padrão
        write(0x04, 0x08, lidarliteAddress); // Padrão
        write(0x1c, 0xb0, lidarliteAddress);
        break;
    }
} /* LIDARLite::configure */

/*------------------------------------------------------------------------------
  Reset

  Reinicia o dispositivo. O dispositivo recarrega as configurações padrão de registro,
  incluindo o endereço I2C padrão. A reinicialização leva aproximadamente 22ms.

  Parâmetros
  ------------------------------------------------------------------------------
  lidarliteAddress: Padrão 0x62. Preencha com o novo endereço aqui se alterado. Veja
    o manual de operação para instruções.
------------------------------------------------------------------------------*/
void LIDARLite::reset(char lidarliteAddress)
{
    write(0x00, 0x00, lidarliteAddress);
} /* LIDARLite::reset */

/*------------------------------------------------------------------------------
  Distance

  Faz uma medição de distância e lê o resultado.

  Processo
  ------------------------------------------------------------------------------
  1.  Escreva 0x04 ou 0x03 no registro 0x00 para iniciar uma aquisição.
  2.  Leia o registro 0x01 (isto é tratado no comando read())
      - se o primeiro bit for "1" então o sensor está ocupado, faça loop até que o primeiro
        bit seja "0"
      - se o primeiro bit for "0" então o sensor está pronto
  3.  Leia dois bytes do registro 0x8f e salve
  4.  Desloque o primeiro valor de 0x8f << 8 e adicione ao segundo valor de 0x8f.
      O resultado é a distância medida em centímetros.

  Parâmetros
  ------------------------------------------------------------------------------
  biasCorrection: Padrão true. Faz aquisição com correção de bias do receptor.
    Se definido como false, as medições serão mais rápidas. A correção de bias do
    receptor deve ser realizada periodicamente. (por exemplo, 1 a cada 100 leituras).
  lidarliteAddress: Padrão 0x62. Preencha com o novo endereço aqui se alterado. Veja
    o manual de operação para instruções.
------------------------------------------------------------------------------*/
int LIDARLite::distance(bool biasCorrection, char lidarliteAddress)
{
    if (biasCorrection)
    {
        // Faz aquisição e processamento de correlação com correção de bias do receptor
        write(0x00, 0x04, lidarliteAddress);
    }
    else
    {
        // Faz aquisição e processamento de correlação sem correção de bias do receptor
        write(0x00, 0x03, lidarliteAddress);
    }
    // Array para armazenar bytes alto e baixo da distância
    byte distanceArray[2];
    // Leia dois bytes do registro 0x8f (autoincremento para ler 0x0f e 0x10)
    read(0x8f, 2, distanceArray, true, lidarliteAddress);
    // Desloca o byte alto e adiciona ao byte baixo
    int distance = (distanceArray[0] << 8) + distanceArray[1];
    return (distance);
} /* LIDARLite::distance */

/*------------------------------------------------------------------------------
  Write

  Executa escrita I2C para o dispositivo.

  Parâmetros
  ------------------------------------------------------------------------------
  myAddress: endereço do registro para escrever.
  myValue: valor para escrever.
  lidarliteAddress: Padrão 0x62. Preencha com o novo endereço aqui se alterado. Veja
    o manual de operação para instruções.
------------------------------------------------------------------------------*/
void LIDARLite::write(char myAddress, char myValue, char lidarliteAddress)
{
    Wire.beginTransmission((int)lidarliteAddress);
    Wire.write((int)myAddress); // Define o registro para escrita
    Wire.write((int)myValue);   // Escreve myValue no registro

    // Um nack significa que o dispositivo não está respondendo, relata o erro via serial
    int nackCatcher = Wire.endTransmission();
    if (nackCatcher != 0)
    {
        Serial.println("> nack");
    }

    delay(1); // Atraso de 1 ms para robustez com leituras e escritas sucessivas
} /* LIDARLite::write */

/*------------------------------------------------------------------------------
  Read

  Executa leitura I2C do dispositivo. Detectará um dispositivo não responsivo e
  relatará o erro via serial. A opção de monitoramento do sinalizador de ocupado
  pode ser usada para ler registros que são atualizados no final de uma medição
  de distância para obter os novos dados.

  Parâmetros
  ------------------------------------------------------------------------------
  myAddress: endereço do registro para ler.
  numOfBytes: número de bytes para ler. Pode ser 1 ou 2.
  arrayToSave: um array para armazenar os valores lidos.
  monitorBusyFlag: se true, a rotina irá ler repetidamente o registro de status
    até que o sinalizador de ocupado (LSB) seja 0.
------------------------------------------------------------------------------*/
void LIDARLite::read(char myAddress, int numOfBytes, byte arrayToSave[2], bool monitorBusyFlag, char lidarliteAddress)
{
    int busyFlag = 0; // busyFlag monitora quando o dispositivo termina uma medição
    if (monitorBusyFlag)
    {
        busyFlag = 1; // Inicia leitura imediatamente se não estiver monitorando sinalizador de ocupado
    }
    int busyCounter = 0; // busyCounter conta o número de vezes que o sinalizador de ocupado é verificado, para timeout

    while (busyFlag != 0) // Loop até o dispositivo não estar ocupado
    {
        // Lê o registro de status para verificar o sinalizador de ocupado
        Wire.beginTransmission((int)lidarliteAddress);
        Wire.write(0x01); // Define o registro de status para ser lido

        // Um nack significa que o dispositivo não está respondendo, relata o erro via serial
        int nackCatcher = Wire.endTransmission();
        if (nackCatcher != 0)
        {
            Serial.println("> nack");
        }

        Wire.requestFrom((int)lidarliteAddress, 1); // Lê o registro 0x01
        busyFlag = bitRead(Wire.read(), 0);         // Atribui o LSB do registro de status a busyFlag

        busyCounter++; // Incrementa busyCounter para timeout

        // Lida com a condição de timeout, sai do loop while e vai para bailout
        if (busyCounter > 9999)
        {
            goto bailout;
        }
    }

    // Dispositivo não está ocupado, inicia leitura
    if (busyFlag == 0)
    {
        Wire.beginTransmission((int)lidarliteAddress);
        Wire.write((int)myAddress); // Define o registro para ser lido

        // Um nack significa que o dispositivo não está respondendo, relata o erro via serial
        int nackCatcher = Wire.endTransmission();
        if (nackCatcher != 0)
        {
            Serial.println("> nack");
        }

        // Executa leitura de 1 ou 2 bytes, salva em arrayToSave
        Wire.requestFrom((int)lidarliteAddress, numOfBytes);
        int i = 0;
        if (numOfBytes <= Wire.available())
        {
            while (i < numOfBytes)
            {
                arrayToSave[i] = Wire.read();
                i++;
            }
        }
    }

    // bailout relata erro via serial
    if (busyCounter > 9999)
    {
    bailout:
        busyCounter = 0;
        Serial.println("> read failed");
    }
} /* LIDARLite::read */

/*------------------------------------------------------------------------------
  Correlation Record To Serial

  O registro de correlação usado para calcular a distância pode ser lido do dispositivo.
  Ele tem uma forma de onda bipolar, transicionando de uma porção positiva para um
  pulso negativo aproximadamente simétrico. O ponto onde o sinal cruza zero representa
  o atraso efetivo para os sinais de referência e retorno.

  Processo
  ------------------------------------------------------------------------------
  1.  Faça uma leitura de distância (não há registro de correlação sem pelo menos
      uma leitura de distância sendo feita)
  2.  Selecione o banco de memória escrevendo 0xc0 no registro 0x5d
  3.  Configure o modo de teste selecionando 0x07 no registro 0x40
  4.  Para tantas leituras quanto quiser (máximo é 1024)
      1.  Leia dois bytes de 0xd2
      2.  O byte baixo é o valor do registro
      3.  O byte alto é o sinal do registro

  Parâmetros
  ------------------------------------------------------------------------------
  separator: o separador entre as palavras de dados seriais
  numberOfReadings: Padrão: 256. Máximo de 1024
  lidarliteAddress: Padrão 0x62. Preencha com o novo endereço aqui se alterado. Veja
    o manual de operação para instruções.
------------------------------------------------------------------------------*/
void LIDARLite::correlationRecordToSerial(char separator, int numberOfReadings, char lidarliteAddress)
{

    // Array para armazenar valores lidos
    byte correlationArray[2];
    // Var para armazenar o valor do registro de correlação
    int correlationValue = 0;
    // Seleciona banco de memória
    write(0x5d, 0xc0, lidarliteAddress);
    // Habilita modo de teste
    write(0x40, 0x07, lidarliteAddress);
    for (int i = 0; i < numberOfReadings; i++)
    {
        // Seleciona byte único
        read(0xd2, 2, correlationArray, false, lidarliteAddress);
        // Byte baixo é o valor do registro de correlação
        correlationValue = correlationArray[0];
        // se o lsb do byte alto estiver definido, o valor é negativo
        if ((int)correlationArray[1] == 1)
        {
            correlationValue |= 0xff00;
        }
        Serial.print((int)correlationValue);
        Serial.print(separator);
    }
    // desabilita modo de teste
    write(0x40, 0x00, lidarliteAddress);
} /* LIDARLite::correlationRecordToSerial */
