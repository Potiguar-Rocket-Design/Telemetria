import serial
import os

#------------------------------|
#ESP:
porta = "/dev/ttyUSB0" # DEPENDE DO SISTEMA! avalie qual sistema opercaional esta usando (Windows, MAC, Linux, FreeBSD...) e mude essa linha baseado no que esta usando
#geralmente no windows seria "COM03" ou algo nesse sentido, voce vai precisar ver o que esta fazendo e usando. 
baud_rate = 115200 # tem que ser igual esta na placa do ESP, sem isso a comunicação não vai existir

#------------------------------|
#DADOS:
prefixo= "dados_telemetria" #sera o nome do arquivo
extensao = ".csv" #sera a extensao do arquivo
contador = 1 #sera o identificador do arquivo

while os.path.exists(f"{prefixo}({contador}){extensao}"):
    contador += 1 
    
    
    #vou terminar de fazer, basicamente só vai ficar olhando se tem algum arquivo de telemetria existente e cria outro quando ligado. 
    


#nome do arquivo de telemetria:
dados_telemetria = f"{prefixo}({contador}){extensao}"
print(f"Novo arquivo criado: {dados_telemetria}")

#cabecalho do arquivo:
cabecalho = "timestamp;packet_id;RSSI;SNR;pacotes_recebidos;pacotes_perdidos;PRR\n"


#------------------------------|
#CONEXAO COM ESP:
conexao_porta_serial = serial.Serial(porta, baud_rate, timeout=1)

#------------------------------|
#ESCRITA DE ARQUIVO:

# abrir o arquivo
with open(dados_telemetria, "a", encoding="utf-8") as f:
    print ("Conectado! dados de telemetria sendo gravados...")
    while True:
        try:
            linha = conexao_porta_serial.readline().decode("utf-8", errors="ignore").strip()
            if linha:
                #ignora mensagens de debug que não sejam dados
                f.write(linha + "\n")
                f.flush() #Salva a linha no disco do PC
                print(f"Salvo: {linha}")
        except KeyboardInterrupt:
                print("Gravação encerrada.")
                break
conexao_porta_serial.close()

