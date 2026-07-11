#include <SPI.h>
#include <LoRa.h>

// Definição dos pinos SPI para o ESP32 (Padrão VSPI)
#define ss 5
#define rst 14
#define dio0 2

// Struct com atributo packed para evitar padding na memória
struct __attribute__((packed)) TelemetryPacket_t {
    // reduzir de 4 bytes para 2 bytes
    // o voo dura menos de 1 horas?
    uint16_t timestamp_ms;
    // quantos estados de voo nós temos?
    uint8_t flightState;
    // int (-32 a + 32 metros) -> uint (0 a 65 metros)
    uint16_t altitude_m;
    // aceleração total
    int16_t accel_total;
    int8_t  snr_db;
    uint8_t checksum;
    // GPS
    int32_t latitude;
    int32_t longitude;
    // temperatura interna do circuito (útil para monitorar em altas altitudes)
    int8_t temperature_c;
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
    // Vamos fazer o teste também com o valor 9
    LoRa.setSpreadingFactor(7);
    LoRa.setSignalBandwidth(125E3);
    // Atual CR 4/5 (1 bit de correção para cada 4 de dados) = É o mais rápido, mas oferece menos proteção contra ruído;
    // Mudança: CR 4/8 = Mais adequado para o caso da telemetria que é onde o sinal sofre reflexão e atenuação severa na subida.
    // Isso torna o link mais resiliente a interferências externas.
    LoRa.setCodingRate4(8);
	LoRa.setSyncWord(0xF3); // O valor pode ser de 0x00 a 0xFF
    LoRa.enableCrc();
    // Potência de transmissão, o atual valor é 2 dBm que é um valor baixo;
    // Vamos usar a Potência Máxima do SX1276 (entre 17 dBm a 20 dBm), que é o ideal para longas distãncias;
    LoRa.setTxPower(17);
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
