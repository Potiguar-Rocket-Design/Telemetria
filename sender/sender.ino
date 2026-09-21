#include <SPI.h>
#include <LoRa.h>

// Definição dos pinos SPI para o ESP32 (Padrão VSPI)
#define ss 4 // anteriormente era 5
#define rst 14
#define dio0 2

// Struct com atributo packed para evitar padding na memória
struct __attribute__((packed)) TelemetryPacket_t {

    // Identificador sequencial do pacote
    uint32_t packet_id;

    // Timestamp do envio
    uint32_t timestamp_ms;

    uint8_t flightState;

    uint16_t altitude_m;

    int16_t accel_total;

    int8_t snr_db;

    uint8_t checksum;

    // GPS
    int32_t latitude;
    int32_t longitude;

    // Temperatura interna do circuito
    int8_t temperature_c;
};

TelemetryPacket_t telemetria;

unsigned long lastSendTime = 0;

const int interval = 1000; // Envia a cada 1 segundo

// Contador dos pacotes
uint32_t packet_id = 0;


// ======================================================
// FUNÇÃO PARA CALCULAR CHECKSUM
// ======================================================

uint8_t calcularChecksum(TelemetryPacket_t* pacote) {

    uint8_t* ptr = (uint8_t*)pacote;

    uint8_t calc = 0;

    // Salva o checksum atual
    uint8_t checksumOriginal = pacote->checksum;

    // Zera temporariamente o checksum
    pacote->checksum = 0;

    for (size_t i = 0; i < sizeof(TelemetryPacket_t); i++) {
        calc ^= ptr[i];
    }

    // Restaura o checksum
    pacote->checksum = checksumOriginal;

    return calc;
}


// ======================================================
// SETUP
// ======================================================

void setup() {

    Serial.begin(115200);

    while (!Serial);

    Serial.println("Inicializando LoRa Node: FOGUETE");

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
/*
    // trial 04
     LoRa.setSpreadingFactor(12);
     LoRa.setSignalBandwidth(125E3);
     LoRa.setCodingRate4(8);
     LoRa.setTxPower(20);
*/
    // Sync Word
    LoRa.setSyncWord(0xF3);

    // Ativa CRC
    LoRa.enableCrc();


    Serial.println("LoRa inicializado com sucesso.");
    Serial.println("Aguardando envio dos pacotes...");
}


// ======================================================
// LOOP
// ======================================================

void loop() {

    // ==================================================
    // ESCUTA: Verifica se chegou o "Pong"
    // da Estação Solo
    // ==================================================

    int packetSize = LoRa.parsePacket();

    if (packetSize) {

        Serial.print("Recebido da base: ");

        while (LoRa.available()) {
            Serial.print((char)LoRa.read());
        }

        Serial.print(" | RSSI: ");

        Serial.println(LoRa.packetRssi());
    }


    // ==================================================
    // ENVIO
    // ==================================================

    if (millis() - lastSendTime > interval) {

        // Identificador do pacote
        telemetria.packet_id = packet_id;


        // Simulando dados de voo
        telemetria.timestamp_ms = millis();

        telemetria.flightState = 2; // 2 = Ascensão

        telemetria.altitude_m += 15;

        telemetria.accel_total = 2;

        telemetria.snr_db = 0;


        // Dados GPS
        telemetria.latitude = 0;

        telemetria.longitude = 0;


        // Temperatura
        telemetria.temperature_c = 25;


        // Calcula checksum
        telemetria.checksum = 0;

        telemetria.checksum =
            calcularChecksum(&telemetria);


        // ==================================================
        // TRANSMISSÃO
        // ==================================================

        LoRa.beginPacket();

        LoRa.write( (uint8_t*)&telemetria, sizeof(TelemetryPacket_t) );

        LoRa.endPacket();


        // ==================================================
        // DEBUG
        // ==================================================

        Serial.print("Pacote de Telemetria Enviado");

        Serial.print(" | ID: ");
        Serial.print(telemetria.packet_id);

        Serial.print(" | Tempo: ");
        Serial.println(telemetria.timestamp_ms);


        // Próximo ID
        packet_id++;


        lastSendTime = millis();
    }
}