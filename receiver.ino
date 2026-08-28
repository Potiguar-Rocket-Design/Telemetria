#include <SPI.h>
#include <LoRa.h>

#define ss 22
#define rst 14
#define dio0 2


// ======================================================
// STRUCT
// ======================================================

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


// ======================================================
// VARIÁVEIS DO PRR
// ======================================================

// Quantidade de pacotes íntegros recebidos
uint32_t pacotesRecebidos = 0;

// Quantidade de pacotes identificados como perdidos
uint32_t pacotesPerdidos = 0;

// ID do último pacote recebido
uint32_t ultimoPacketID = 0;

// Indica se já recebemos o primeiro pacote
bool primeiroPacote = true;


// ======================================================
// CHECKSUM
// ======================================================

uint8_t calcularChecksum(TelemetryPacket_t* pacote) {

    uint8_t* ptr = (uint8_t*)pacote;

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


// ======================================================
// SETUP
// ======================================================

void setup() {

    Serial.begin(115200);

    while (!Serial);

    Serial.println("Inicializando LoRa Node: ESTAÇÃO SOLO");

    LoRa.setPins(ss, rst, dio0);


    if (!LoRa.begin(433E6)) {

        Serial.println("Falha ao iniciar o LoRa!");

        while (1);
    }


    // ==================================================
    // CONFIGURAÇÃO DO TRIAL
    // ==================================================

    // Trial 01
    LoRa.setSpreadingFactor(7);
    LoRa.setSignalBandwidth(125E3);
    LoRa.setCodingRate4(5);
    LoRa.setTxPower(2);

    // Trial 02
    // LoRa.setSpreadingFactor(7);
    // LoRa.setSignalBandwidth(125E3);
    // LoRa.setCodingRate4(8);
    // LoRa.setTxPower(17);

    // Trial 03
    // LoRa.setSpreadingFactor(9);
    // LoRa.setSignalBandwidth(125E3);
    // LoRa.setCodingRate4(8);
    // LoRa.setTxPower(17);


    // Sync Word
    LoRa.setSyncWord(0xF3);

    // Ativa CRC
    LoRa.enableCrc();


    Serial.println("LoRa inicializado com sucesso.");

    Serial.println("PRR iniciado.");

    Serial.println();

    Serial.println(
        "Tempo(ms);ID;RSSI(dBm);SNR(dB);"
        "Recebidos;Perdidos;PRR(%)"
    );
}


// ======================================================
// LOOP
// ======================================================

void loop() {

    // ==================================================
    // AGUARDA PACOTES
    // ==================================================

    int packetSize = LoRa.parsePacket();


    // ==================================================
    // VERIFICA TAMANHO
    // ==================================================

    if (packetSize == sizeof(TelemetryPacket_t)) {


        // ==================================================
        // LÊ PACOTE
        // ==================================================

        LoRa.readBytes(
            (uint8_t*)&rxPacote,
            sizeof(TelemetryPacket_t)
        );


        // ==================================================
        // VALIDA CHECKSUM
        // ==================================================

        uint8_t checksumCalculado =
            calcularChecksum(&rxPacote);


        if (rxPacote.checksum == checksumCalculado) {


            // ==================================================
            // PACOTE ÍNTEGRO
            // ==================================================

            pacotesRecebidos++;


            // ==================================================
            // IDENTIFICA PACOTES PERDIDOS
            // ==================================================

            if (primeiroPacote) {

                // Primeiro pacote recebido
                ultimoPacketID =
                    rxPacote.packet_id;

                primeiroPacote = false;

            } else {

                // Verifica se existem IDs faltando
                if (rxPacote.packet_id >
                    ultimoPacketID + 1) {

                    uint32_t quantidadePerdida =
                        rxPacote.packet_id -
                        ultimoPacketID -
                        1;

                    pacotesPerdidos +=
                        quantidadePerdida;
                }

                // Atualiza último ID
                ultimoPacketID =
                    rxPacote.packet_id;
            }


            // ==================================================
            // CALCULA PACOTES ESPERADOS
            // ==================================================

            uint32_t pacotesEsperados =
                pacotesRecebidos +
                pacotesPerdidos;


            // ==================================================
            // CALCULA PRR
            // ==================================================

            float prr = 0;

            if (pacotesEsperados > 0) {

                prr =
                    ((float)pacotesRecebidos /
                     (float)pacotesEsperados)
                    * 100.0;
            }


            // ==================================================
            // RSSI
            // ==================================================

            float rssi =
                LoRa.packetRssi();


            // ==================================================
            // SNR
            // ==================================================

            float snr =
                LoRa.packetSnr();


            // ==================================================
            // DADOS NO SERIAL
            // ==================================================

            Serial.print(millis());

            Serial.print(";");

            Serial.print(rxPacote.packet_id);

            Serial.print(";");

            Serial.print(rssi);

            Serial.print(";");

            Serial.print(snr);

            Serial.print(";");

            Serial.print(pacotesRecebidos);

            Serial.print(";");

            Serial.print(pacotesPerdidos);

            Serial.print(";");

            Serial.println(prr, 2);


            // ==================================================
            // INFORMAÇÕES DETALHADAS
            // ==================================================

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


            // ==================================================
            // ENVIA O PONG / ACK
            // ==================================================

            LoRa.beginPacket();

            LoRa.print("ACK");

            LoRa.endPacket();
        }


        else {

            // ==================================================
            // CHECKSUM INVÁLIDO
            // ==================================================

            Serial.println(
                "ERRO: Checksum inválido. "
                "Pacote corrompido!"
            );
        }
    }


    // ==================================================
    // PACOTE DESCONHECIDO
    // ==================================================

    else if (packetSize > 0) {

        Serial.print(
            "Pacote desconhecido recebido. "
            "Tamanho: "
        );

        Serial.println(packetSize);
    }
}