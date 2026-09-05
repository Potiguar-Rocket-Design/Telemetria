/*

                        ! A T E N C A O !

            Esse codigo ainda deve ser editado e limpo, não apague os comentarios referentes a escrita de dados
em cartaoSD pois esses trechos esta servido apenas como esborco/sketch para a versão receiver_v2.ino que 
ainda será escrita para a versão da estacao de telemetria!
    Por enquanto apenas ignore esses trechos de codigo. 

    Se tiver interesse, melhore esse codigo mantendo a sua funcionalidade e lembre-se sempre dessas regras:

        - SEMPRE DEVE SER COMENTADO TUDO QUE FAZ, GASTE TODAS AS SUAS PALAVRAS E IDEIAS, COMENTE MUITO!;
        - SEMPRE PREFIRA FAZER O MAIS FACIL DE SER LIDO E MANTIDO, EVITE TUDO QUE FOR COMPLEXO!;
        - LEMBRAR QUE TUDO AQUI DEVE SER MANTIDO OU CONTINUADO POR OUTRA PESSOA, DEIXE TUDO O MAIS FACIL POSSIVEL;
        - FACILIDADE;
        - PRATICIDADE;
        - CONFIANCA;
        - SEGURANCA;


*/

//BIBLIOTECAS, INCLUIDES E DEMAIS COISAS:
#include <Arduino.h> 
#include <SPI.h>
#include <LoRa.h>
// #include <ArduinoJson.h> 
// Biblioteca para estrutrar/salvar os dados da telemetria (e o que mais for necessario) 


// DEFINES, CONSTANTES, PINOS E DEMAIS COISAS NESSE SENTIDO:
#define ss 22
#define rst 14
#define dio0 2


// S T R U C T : 

/*

Essa struct esta usando um atribute que garante que os dados na memoria fiquem alocados na mesma sequencia. 
No geral é só uma struct mesmo tendo esse atributo para proteção da alocação dos dados na memoria de forma
mais segura e confiavel.

*/
struct __attribute__((packed)) TelemetryPacket_t {

    // Identificador sequencial do pacote
    uint32_t packet_id;

    uint16_t timestamp_ms;

    uint8_t flightState;

    uint16_t altitude_m;

    int16_t accel_total;

    int8_t snr_db;

    uint8_t checksum;

    // GPS
    int32_t latitude;
    int32_t longitude;

    // Temperatura
    int8_t temperature_c;
};

TelemetryPacket_t rxPacote;


// V A R I Á V E I S  D O  P R R :

// Quantidade de pacotes íntegros recebidos
uint32_t pacotesRecebidos = 0;

// Quantidade de pacotes identificados como perdidos
uint32_t pacotesPerdidos = 0;

// ID do último pacote recebido
uint32_t ultimoPacketID = 0;

// Indica se já recebemos o primeiro pacote
bool primeiroPacote = true;



// C H E C K S U M :

/*

essa função vai verificar como os pacotes estao sendo recebidos, esta usando um padrão seguro
para a industria/meio/ramo (pelo amor de Deus, revisa esses comentarios e deixa eles mais bem estruturadoskkkkkk)

Ao bater o olho pela primeira vez ela pode assustar com essa manipulacao de ponteiros, mas ela funciona exatamente assim
e no geral é assim que vai ser encontrada.

*/
uint8_t calcularChecksum(TelemetryPacket_t* pacote) { //esta recebendo o local que esta guardado o pcatoe

    uint8_t* ptr = (uint8_t*)pacote; //entao "ptr" tambem esta apontando pra esse local que "pacote" esta na memotira

    uint8_t calc = 0;

    // Salva checksum original
    uint8_t checksumOriginal = pacote->checksum;

    // Zera temporariamente
    pacote->checksum = 0;

    // Calcula usando todos os bytes
    for (size_t i = 0; i < sizeof(TelemetryPacket_t); i++) {
        calc ^= ptr[i];
    }

    // Restaura
    pacote->checksum = checksumOriginal;

    return calc;
}


// =================
// =  S E T U P    =
// =================

void setup() {

    Serial.begin(115200);

    while (!Serial);

    Serial.println("Inicializando LoRa Node: ESTAÇÃO SOLO");

    LoRa.setPins(ss, rst, dio0);


    if (!LoRa.begin(433E6)) {

        Serial.println("Falha ao iniciar o LoRa!");

        while (1);
    }


    
    // CONFIGURAÇÃO DO TRIAL:
/*
 - Devido o datasheet, eh necessario setar ente 11 ou 12 de LoRa.setSpreadingFactor():
        Ver se serah necessario mudar o valor da banda no LoRa.setSignalBandwidth(125E3), a unica frequencia disponivel é o RFS_L7.8_LF;
- serah avaliado o LoRa.setCodingRate4(5) e o LoRa.setTxPower(2);

*/


    // Trial 01
    LoRa.setSpreadingFactor(7); 
    LoRa.setSignalBandwidth(125E3);
    LoRa.setCodingRate4(5);
    LoRa.setTxPower(2);
/*
    // Trial 02
     LoRa.setSpreadingFactor(7);
     LoRa.setSignalBandwidth(125E3);
     LoRa.setCodingRate4(8);
     LoRa.setTxPower(17);
*/
 
 /*
    // Trial 03
     LoRa.setSpreadingFactor(9);
     LoRa.setSignalBandwidth(125E3);
     LoRa.setCodingRate4(8);
     LoRa.setTxPower(17);
*/

    // Sync Word
    LoRa.setSyncWord(0xF3);

    // Ativa CRC
    LoRa.enableCrc();


    Serial.println("LoRa inicializado com sucesso.");

    Serial.println("PRR iniciado.");

    Serial.println();

    Serial.println(
        "- Tempo(ms); ID; RSSI(dBm); SNR(dB); \n"
        "- Recebidos; Perdidos; PRR(%)\n"
    );

    /*
    
    BLOCO PARA SALVAR OS DADOS EM UM ARQUIVO CSV:

        1 - Atenção! se vc esta lendo esse bloco de comentarios a partir dessa linha
        é sinal que ainda falta ser ajustado ou confirmado alguma coisa!
        provavelmente o formato de como esta sendo gravado os dados ou os dados a serem salvos.

        2 - A ideia inicial é deixar os dados salvos no ESP/Arduino (a depender da plataforma usada) 
        e no PC, se não precisar de algo é comentar o bloco, um não interferir no outro.
    
    */
/*
    // SALVAR DADOS EM ARQUIVO CSV:
        const char* arquivo_dados_telemetria = "dados_telemetria.csv"; //Atribui a string "dados_telemetria.csv" a constante arquivo_dados_telemetria
        File arquivoCSV = SD.open(arquivo_dados_telemetria, FILE_WRITE); //cria arquivoCSV do tipo File, abre o arquivo e deixa ele "escrevivel" (to ocupado e cheio de coisa pra fazer, lute pra entenderkkkkkkkk)

        if (!arquivoCSV) { //avisa se o arquivo CSV não foi aberto
            Serial.println("Falha ao abrir o arquivo CSV para gravação dos dados!");
            return;
        }

    // PARA ESCREVER O CABEÇALHO DO ARQUIVO
    arquivoCSV.print("timestam;packet_id;RSSI;SNR;pacotes_recebidos;pacotes_perdidos;PRR\n"); //escreve no arquivo CSV exatamente o que esta dentro dessa função
    Serial.println("Arquivo CSV criado!") //notifica no monitor serial que o arquivo CSV foi criado
*/
}


// ============
// = L O O P  =
// ============

void loop() {

    // AGUARDA PACOTES
    int packetSize = LoRa.parsePacket();


    // VERIFICA TAMANHO
    if (packetSize == sizeof(TelemetryPacket_t)) {

        // LÊ PACOTE
        LoRa.readBytes((uint8_t*)&rxPacote, sizeof(TelemetryPacket_t));

        // VALIDA CHECKSUM
        uint8_t checksumCalculado = calcularChecksum(&rxPacote);

        if (rxPacote.checksum == checksumCalculado) {

            // PACOTE ÍNTEGRO
            pacotesRecebidos++;

            // IDENTIFICA PACOTES PERDIDOS
            if (primeiroPacote) {

                // Primeiro pacote recebido
                ultimoPacketID = rxPacote.packet_id;

                primeiroPacote = false;

            } else {

                // Verifica se existem IDs faltando
                if (rxPacote.packet_id > ultimoPacketID + 1) {

                    uint32_t quantidadePerdida = ( rxPacote.packet_id - ultimoPacketID - 1 );

                    pacotesPerdidos += quantidadePerdida;
                }

                // Atualiza último ID
                ultimoPacketID = rxPacote.packet_id;
            }


            // CALCULA PACOTES ESPERADOS
            uint32_t pacotesEsperados = pacotesRecebidos + pacotesPerdidos;

            // CALCULA PRR
            float prr = 0;

            if (pacotesEsperados > 0) {

                prr = ((float)pacotesRecebidos / (float)pacotesEsperados) * 100.0;

            }

            // RSSI
            float rssi = LoRa.packetRssi();

            // SNR
            float snr = LoRa.packetSnr();

            /*
                Para gravar os dados será convertido os dados em strings e entao gravados
            no arquivo.

            primeiro vai converter todas em strings, dps montar a string e salvar ela no arquivo

            */
/*
            //Criar o texto de registro:
            char texto[] = "timestamp;packet_id;RSSI;SNR;pacotes_recebidos;pacotes_perdidos;PRR\n";

            //converter timestam para string e gravar no CSV:
            String time_String = String(millis());
            String id_String = String(rxPacote.packet_id);
            String rssi_String = String(rssi);
            String snr_String = String(snr);
            String recebido_String = String(pacotes_recebidos);
            String perdido_String = String(pacotes_perdidos);
            String prr_String = String(prr, 2); 

            //montar texto para gravação:
            String linha = time_String + ";" + id_String + ";" + rssi_String + ";" + snr_String + ";" + recebido_String + ";" + perdido_String + ";" + prr_String;
            
            //escrevendo no arquivo:
            arquivoCSV.print(linha);

            //mostrar na tela:
            Serial.print("Registro gravado: ");
            Serial.println(linha);
*/


            // DADOS MOSTRADOS NO SERIAL. SAO GRAVADOS NO ARQUIVO DE TELEMETRIA!

            // DADOS NO SERIAL
            Serial.print(rxPacote.packet_id);
            Serial.println(";"); //okie!


            Serial.print(rssi);
            Serial.println(";"); //okie!

            Serial.print(pacotesRecebidos);
            Serial.println(";"); //okie!

            Serial.print(pacotesPerdidos);
            Serial.println(";"); //okie!

            Serial.println(prr, 2); //okie!

            Serial.print(snr);
            Serial.println(";");

            Serial.print(rxPacote.timestamp_ms);
            Serial.println(";");


//            Serial.print(millis());
//            Serial.print(";");

/*
            // INFORMAÇÕES DETALHADAS
            Serial.println("--- PACOTE ÍNTEGRO RECEBIDO ---");

            Serial.print("ID do pacote: ");
            Serial.println(rxPacote.packet_id);

            Serial.print("Tempo (ms): ");
            Serial.println(rxPacote.timestamp_ms);

            Serial.print("Estado de Voo: ");
            Serial.println(rxPacote.flightState);

            Serial.print("Altitude (m): ");
            Serial.println(rxPacote.altitude_m);

            Serial.print("Aceleração: ");
            Serial.println(rxPacote.accel_total);

            Serial.print("RSSI: ");
            Serial.print(rssi);
            Serial.println(" dBm");

            Serial.print("SNR: ");
            Serial.print(snr);
            Serial.println(" dB");

            Serial.print("Pacotes recebidos: ");
            Serial.println(pacotesRecebidos);

            Serial.print("Pacotes perdidos: ");
            Serial.println(pacotesPerdidos);

            Serial.print("PRR: ");
            Serial.print(prr, 2);
            Serial.println("%");

            Serial.println();



            // ENVIA O PONG / ACK
            LoRa.beginPacket();
            LoRa.print("ACK");
            LoRa.endPacket();
*/        }


        else {
            // CHECKSUM INVÁLIDO
            Serial.println(
                "ERRO: Checksum inválido. "
                "Pacote corrompido!"
            );
        }
    }


    // PACOTE DESCONHECIDO
    else if (packetSize > 0) {

        Serial.print(
            "Pacote desconhecido recebido. "
            "Tamanho: "
        );

        Serial.println(packetSize);
    }
}