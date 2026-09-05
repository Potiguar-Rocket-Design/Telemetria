uint32_t packet_id = 0;
uint32_t pacotesRecebidos = 0;
uint32_t pacotesPerdidos = 0;

void setup() {
    Serial.begin(115200);
    while (!Serial);

    // Linha de cabeçalho inicial
    Serial.println("timestamp;packet_id;RSSI;SNR;pacotes_recebidos;pacotes_perdidos;PRR");
}

void loop() {
    // 1. Gera e atualiza contadores simulados
    packet_id++;
    pacotesRecebidos++;
    
    // Simula uma perda ocasional de pacotes (1 chance em 10)
    if (random(0, 10) == 0) {
        pacotesPerdidos++;
    }

    uint32_t pacotesEsperados = pacotesRecebidos + pacotesPerdidos;
    float prr = ((float)pacotesRecebidos / (float)pacotesEsperados) * 100.0;

    // 2. Gera dados aleatórios com faixas realistas
    unsigned long timestamp = millis();
    float rssi = random(-1150, -400) / 10.0;  // -115.0 a -40.0 dBm
    float snr  = random(-80, 120) / 10.0;     // -8.0 a 12.0 dB

    // 3. Monta e envia a linha CSV pelo Serial
    // Formato: timestamp;packet_id;RSSI;SNR;pacotes_recebidos;pacotes_perdidos;PRR
    Serial.print(timestamp);
    Serial.print(";");
    Serial.print(packet_id);
    Serial.print(";");
    Serial.print(rssi, 1);
    Serial.print(";");
    Serial.print(snr, 1);
    Serial.print(";");
    Serial.print(pacotesRecebidos);
    Serial.print(";");
    Serial.print(pacotesPerdidos);
    Serial.print(";");
    Serial.println(prr, 2);

    // Intervalo de envio simulado (2 pacotes por segundo)
    delay(500);
}