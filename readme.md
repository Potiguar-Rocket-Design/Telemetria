# TELEMETRIA

### Estrutura do Projeto

```text
TELEMETRIA/
├── dados de teste UFRN/        # pasta com os dados coletados
│   ├── trial_01/   # teste 1
│   ├── trial_02/   # teste 2
│   ├── trial_03/   # teste 3
│   └── trial_04/   # teste 4
├── receiver/                   # pata com os arquivos do receiver
│   ├── build/
│   ├── dist/   
│   │   └── serial_receiver_pc  #executavel do app que grava os dados em um arquivo csv
│   ├── receiver.ino    #codigo do receiver para o ESP/Arduino
│   ├── serial_receiver_pc.py   #app em Python para gravar os dados do receiver em um arquivo .csv
│   └── serial_receiver_pc.spec
└── sender/                     #pasta com o arquivo do sender
    ├── Codigo de IA pra simular com valores random, pode ser ignorado/
    │   └── sander pra teste de placa.ino
    └── sender.ino  #codigo do sender, envia os dados/pacotes, precisa ser confugurado
```

## Receiver:
- ### receiver.ino:

- ### serial_receiver_pc:

## Sender:
- ### sander.ino: