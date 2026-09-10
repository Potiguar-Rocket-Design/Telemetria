# Projeto de Telemetria LoRa

Este repositório contém a implementação completa do sistema de telemetria sem fio utilizando módulos **LoRa (433 MHz)** e microcontroladores **(ESP32/Arduino)**. O sistema é dividido entre o transmissor de bordo (nodo embarcado/foguete), o receptor em solo (estação de solo) e uma aplicação em Python para gravação e análise de dados em tempo real no computador.

---

# Visão Geral do Sistema

O objetivo principal deste sistema é transmitir métricas de voo e parâmetros de qualidade da conexão de rádio em tempo real, garantindo a integridade dos dados transmitidos a longas distâncias com baixo consumo e tolerância a ruídos.

## Principais Recursos
* **Transmissão via LoRa 433 MHz**: Suporte a ajustes dinâmicos de *Spreading Factor* (SF7 a SF12), *Bandwidth* (125 kHz) e *Coding Rate* (4/5 a 4/8).
* **Estrutura Compacta e Empacotada**: Utilização de `struct packed` para transmissão eficiente de dados binários brutos via rádio.
* **Verificação de Integridade (Checksum XOR)**: Validação byte a byte antes do processamento dos pacotes recebidos.
* **Cálculo de Desempenho do Link em Tempo Real**: Cálculo de **PRR** (*Packet Reception Rate*), detecção automática de pacotes perdidos por id de sequência, medidores **RSSI** e **SNR**.
* **Gravação Automática no PC**: Script em Python com detecção dinâmica de portas USB/Seriais e geração contínua de relatórios `.csv`.

---

#  Estrutura de Arquivos

```text
teste_telemetria_prd/
└── Telemetria/
    ├── dados de teste UFRN/         # Registros de ensaios e testes de campo realizados
    │   ├── trial_01/                # Teste 1 (SF7, CR 4/5, Power 2dBm)
    │   ├── trial_02/                # Teste 2 (SF7, CR 4/8, Power 17dBm)
    │   ├── trial_03/                # Teste 3 (SF9, CR 4/8, Power 17dBm)
    │   └── trial_04/                # Teste 4 (SF12, CR 4/8, Power 20dBm)
    │
    ├── receiver/                    # Código e utilitários da Estação Solo (Receptor)
    │   ├── receiver.ino             # Código C++/Arduino para o microcontrolador receptor LoRa
    │   ├── serial_receiver_pc.py    # Script Python para ler a Serial e salvar logs em .csv
    │   └── dist/                    # Executável compilado do receptor em Python
    │
    ├── sender/                      # Código do Módulo Embarcado (Transmissor)
    │   └── sender.ino               # Código principal de telemetria do transmissor
    │   
    │       
    │
    ├── TODO.md                      # Checklist de pendências e roadmap de desenvolvimento
    └── readme.md                    # Esboço inicial da documentação
```

---

##  Estrutura de Dados do Pacote (`TelemetryPacket_t`)

Os dados são transmitidos em binário direto da memória através da seguinte estrutura alinhada:

```cpp
struct __attribute__((packed)) TelemetryPacket_t {
    uint32_t packet_id;      // Identificador sequencial incremental do pacote
    uint16_t timestamp_ms;   // Tempo decorrido desde a inicialização (ms)
    uint8_t  flightState;    // Estado de voo (ex: 2 = Ascensão)
    uint16_t altitude_m;     // Altitude estimada em metros
    int16_t  accel_total;    // Aceleração resultante
    int8_t   snr_db;         // SNR reportado (se aplicável)
    uint8_t  checksum;       // Byte de validação XOR
    int32_t  latitude;       // Latitude GPS (graus * 10^7)
    int32_t  longitude;      // Longitude GPS (graus * 10^7)
    int8_t   temperature_c;  // Temperatura interna da placa/ambiente (°C)
};
```

---

# Detalhamento e Funcionamento dos Códigos

## Transmissor (`sender/sender.ino`)
* **Pinos Utilizados (ESP32 - SPI VSPI)**:
  * `NSS / SS`: GPIO 4
  * `RST`: GPIO 14
  * `DIO0`: GPIO 2
* **Mecanismo de Envio**:
  1. Coleta os sensores (ou dados simulados) e preenche a estrutura `telemetria`.
  2. Zera temporariamente o campo `checksum`, executa o cálculo XOR de todos os bytes do pacote e atribui o resultado ao campo `checksum`.
  3. Envia o pacote completo em formato binário bruto (`LoRa.write((uint8_t*)&telemetria, sizeof(TelemetryPacket_t))`).
  4. Aguarda confirmação (opcional) ou o intervalo especificado (`interval = 1000ms`).

### Receptor (`receiver/receiver.ino`)
* **Pinos Utilizados (ESP32)**:
  * `NSS / SS`: GPIO 22
  * `RST`: GPIO 14
  * `DIO0`: GPIO 2
* **Mecanismo de Recepção**:
  1. Verifica se a quantidade de bytes recebidos pela interface LoRa é igual ao tamanho exato da `TelemetryPacket_t`.
  2. Carrega os bytes diretamente para a variável `rxPacote`.
  3. Calcula o checksum XOR local e compara com `rxPacote.checksum`. Se for diferente, descarta o pacote como corrompido.
  4. **Cálculo do PRR**:
     * Detecta o salto entre `packet_id` atual e `ultimoPacketID` para contabilizar pacotes perdidos.
     * Calcula o PRR: $\text{PRR} = \frac{\text{pacotesRecebidos}}{\text{pacotesRecebidos} + \text{pacotesPerdidos}} \times 100\%$
  5. Imprime a linha de telemetria na porta Serial formatada com delimitador `;`.

### Leitor Serial do Computador (`receiver/serial_receiver_pc.py`)
* **Funcionamento**:
  1. Utiliza `pyserial` para listar todas as portas USB/Serial ativas no computador e solicita a escolha do usuário.
  2. Cria automaticamente um novo arquivo sequencial com o padrão `dados_telemetria(1).csv`, `dados_telemetria(2).csv`, etc.
  3. Escreve o cabeçalho padrão no formato CSV.
  4. Mantém um loop de leitura contínuo salvando cada linha na porta Serial diretamente no disco rígido usando `flush()`.

---

## Formato dos Dados CSV

As informações geradas e gravadas no arquivo `.csv` seguem o seguinte cabeçalho e delimitador:

```text
rxPacote.packet_id ; rssi ; pacotesRecebidos ; pacotesPerdidos ; prr ; snr ; rxPacote.timestamp_ms
```

| Coluna | Descrição |
| :--- | :--- |
| `rxPacote.packet_id` | Número de identificação único e sequencial do pacote transmitido. |
| `rssi` | Potência do sinal recebido em dBm (*Received Signal Strength Indicator*). |
| `pacotesRecebidos` | Total de pacotes válidos recebidos pela estação solo até o momento. |
| `pacotesPerdidos` | Total de pacotes perdidos identificados pela quebra na sequência de IDs. |
| `prr` | Taxa de recepção de pacotes (*Packet Reception Rate*) em porcentagem (%). |
| `snr` | Relação Sinal-Ruído em dB (*Signal-to-Noise Ratio*). |
| `rxPacote.timestamp_ms` | Timestamp do momento do envio gerado pelo transmissor (em milissegundos). |

---

# Como Compilar e Executar

### Pré-requisitos
* **Hardware**:
  * 2x Placas ESP32 ou Arduino compatíveis.
  * 2x Módulos LoRa SX1276 / SX1278 (433 MHz).
* **Software**:
  * [Arduino IDE](https://www.arduino.cc/en/software) com as bibliotecas:
    * `LoRa` (por Sandeep Mistry)
    * `SPI` (nativa)
  * [Python 3.x](https://www.python.org/) com a biblioteca `pyserial`:
    ```bash
    pip install pyserial
    ```

### Passo a Passo
1. **Transmissor**:
   * Abra `sender/sender.ino` na Arduino IDE.
   * Selecione o *Trial* de rádio desejado descomentando as linhas apropriadas em `setup()`.
   * Faça o upload para a placa transmissora (embarcada).

2. **Receptor**:
   * Abra `receiver/receiver.ino` na Arduino IDE.
   * Certifique-se de usar a mesma configuração de rádio (*Spreading Factor*, *Bandwidth*, *Coding Rate*) configurada no transmissor.
   * Faça o upload para a placa receptora (estação solo).

3. **Gravação no PC**:
   * Conecte o receptor ao computador via cabo USB.
   * Execute o script Python:
     ```bash
     python receiver/serial_receiver_pc.py
     ```
   * Selecione a porta serial correspondente (ex: `/dev/ttyUSB0` no Linux ou `COM3` no Windows).
   * O arquivo `dados_telemetria(N).csv` começará a ser preenchido automaticamente com os dados recebidos.

---

## Regras e Boas Práticas do Projeto (Diretrizes dos Desenvolvedores)

Conforme documentado nos arquivos internos do projeto:
* **Documentação Extensa**: Todo código novo deve ser devidamente comentado de forma clara e legível.
* **Prioridade para Simplicidade**: Evitar complexidades desnecessárias no código para facilitar manutenções futuras por outros membros da equipe.
* **Segurança e Confiabilidade**: Sempre validar o Checksum de pacotes antes de processar dados sensíveis de voo.
