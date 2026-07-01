#include <SPI.h>
#include <LoRa.h>

#define ss 22
#define rst 14
#define dio0 2

struct __attribute__((packed)) TelemetryPacket_t {
    uint32_t timestamp_ms;
    uint8_t flightState;
    int16_t altitude_m;
    int16_t accel_total;
    int8_t  snr_db;
    uint8_t checksum;
};

TelemetryPacket_t rxPacote;

uint8_t calcularChecksum(TelemetryPacket_t* pacote) {
    uint8_t* ptr = (uint8_t*)pacote;
    uint8_t calc = 0;
    for (size_t i = 0; i < sizeof(TelemetryPacket_t) - 1; i++) {
        calc ^= ptr[i];
    }
    return calc;
}

void setup() {
    Serial.begin(115200);
    while (!Serial);

    Serial.println("Inicializando LoRa Node: ESTAÇÃO SOLO");
    LoRa.setPins(ss, rst, dio0);

    if (!LoRa.begin(433E6)) {
        Serial.println("Falha ao iniciar o LoRa!");
        while (1);
    }

    LoRa.setSpreadingFactor(7);
    LoRa.setSignalBandwidth(125E3);
    LoRa.setCodingRate4(5);
	LoRa.setSyncWord(0xF3); // O valor pode ser de 0x00 a 0xFF
    LoRa.enableCrc();
    LoRa.setTxPower(2);
}

void loop() {
    // Fica aguardando novos pacotes ativamente
    int packetSize = LoRa.parsePacket();

    // Verifica se recebemos um pacote e se o tamanho bate com a nossa struct
    if (packetSize == sizeof(TelemetryPacket_t)) {

        // Lê os bytes recebidos diretamente para o endereço de memória da struct
        LoRa.readBytes((uint8_t*)&rxPacote, sizeof(TelemetryPacket_t));

        // Validação de Checksum
        uint8_t checksumCalculado = calcularChecksum(&rxPacote);

        if (rxPacote.checksum == checksumCalculado) {
            // Sucesso! Dados íntegros.
            Serial.println("--- PACOTE ÍNTEGRO RECEBIDO ---");
            Serial.print("Tempo (ms): ");
            Serial.println(rxPacote.timestamp_ms);
            Serial.print("Estado de Voo: ");
            Serial.println(rxPacote.flightState);
            Serial.print("Altitude (m): ");
            Serial.println(rxPacote.altitude_m);
            Serial.print("Aceleração (G): ");
            Serial.println(rxPacote.accel_total_g);

            // Calculando métricas de sinal (atualizando SNR)
            float snr = LoRa.packetSnr();
            Serial.print("SNR do link de rádio: "); Serial.println(snr);

            // Envia o PONG (Acknowledge) de volta para o foguete
            LoRa.beginPacket();
            LoRa.print("ACK");
            LoRa.endPacket();

        } else {
            Serial.println("ERRO: Checksum inválido. Pacote corrompido!");
        }
    } else if (packetSize > 0) {
        // Recebeu um pacote de tamanho não reconhecido (pode ser interferência)
        Serial.print("Pacote desconhecido recebido. Tamanho: ");
        Serial.println(packetSize);
    }
}
