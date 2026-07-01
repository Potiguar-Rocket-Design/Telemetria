#include <SPI.h>
#include <LoRa.h>

// Definição dos pinos SPI para o ESP32 (Padrão VSPI)
#define ss 5
#define rst 14
#define dio0 2

// Struct com atributo packed para evitar padding na memória
struct __attribute__((packed)) TelemetryPacket_t {
    uint32_t timestamp_ms;
    uint8_t flightState;
    int16_t altitude_m;
    int16_t accel_total;
    int8_t  snr_db;
    uint8_t checksum;
};

TelemetryPacket_t telemetria;
unsigned long lastSendTime = 0;
const int interval = 1000; // Envia a cada 1 segundo

// Função para calcular um checksum simples (XOR de todos os bytes, exceto o próprio checksum)
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

    Serial.println("Inicializando LoRa Node: FOGUETE");
    LoRa.setPins(ss, rst, dio0);

    if (!LoRa.begin(433E6)) { // Frequência de 433 MHz
        Serial.println("Falha ao iniciar o LoRa!");
        while (1);
    }

    // Configurações recomendadas para link inicial
    LoRa.setSpreadingFactor(7);
    LoRa.setSignalBandwidth(125E3);
    LoRa.setCodingRate4(5);
	LoRa.setSyncWord(0xF3); // O valor pode ser de 0x00 a 0xFF
    LoRa.enableCrc();
    LoRa.setTxPower(2);
}

void loop() {
    // ESCUTA: Verifica se chegou o "Pong" da Estação Solo
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
        Serial.print("Recebido da base: ");
        while (LoRa.available()) {
            Serial.print((char)LoRa.read());
        }
        Serial.print(" | RSSI: ");
        Serial.println(LoRa.packetRssi());
    }

    // ENVIO: Cadenciado via millis() (Não-bloqueante)
    if (millis() - lastSendTime > interval) {
        // Simulando dados de voo
        telemetria.timestamp_ms = millis();
        telemetria.flightState = 2; // 2 = Ascensão
        telemetria.altitude_m += 15; // Subindo!
        telemetria.accel_total = 2; //
        telemetria.snr_db = 0; // Preenchido pela estação que recebe, aqui pode ir 0

        telemetria.checksum = calcularChecksum(&telemetria);

        // Transmissão
        LoRa.beginPacket();
        LoRa.write((uint8_t*)&telemetria, sizeof(TelemetryPacket_t));
        LoRa.endPacket();

        Serial.print("Pacote de Telemetria Enviado. Tempo: ");
        Serial.println(telemetria.timestamp_ms);

        lastSendTime = millis();
    }
}
