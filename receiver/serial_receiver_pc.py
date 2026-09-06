import serial
import os

import sys
import serial.tools.list_ports


#------------------------------|
#DETECTAR PORTAS USB:
def selecionar_porta():
    portas = list(serial.tools.list_ports.comports()) #Vê as portas e guarda cada uma com o que esta conectado
    if not portas: #Avisa que nenhum dispotivo serial foi encontrado e encerra o programa 
        print("Nenhum dispositivo serial encontrado! Verifique a conexão USB.")
        input("Pressione Enter para sair...")
        sys.exit(1)

    print("--- Dispositivos Seriais Encontrados ---") #apenas avisa que encontrou algo no Serial
    for i, p in enumerate(portas): #para cada dispositivo serial é atribuido um numero para identificar, o nome do dispotivo e uma pequena descrição dele
        print(f"{i+1} {p.device} - {p.description}") #apenas mostra na tela o que foi dito no comentario anterior, por estar em um 'for', os itens estação em formato de lista

    while True:
        escolha = input(f"Selecione a porta [ 11{len(portas)}]: ").strip()
        if escolha.isdigit() and 1 <= int(escolha) <= len(portas):
            return porta[int(escolha) - 1].device
        print("Opção invalida. Tente novamente.")

#Selecao de portas:
porta = selecionar_porta()
baud_rate = 115200

#------------------------------|
"""
#ESP:
porta = "/dev/ttyUSB0" # DEPENDE DO SISTEMA! avalie qual sistema opercaional esta usando (Windows, MAC, Linux, FreeBSD...) e mude essa linha baseado no que esta usando
#geralmente no windows seria "COM03" ou algo nesse sentido, voce vai precisar ver o que esta fazendo e usando. 
baud_rate = 115200 # tem que ser igual esta na placa do ESP, sem isso a comunicação não vai existir

"""

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

try:
    conexao_porta_serial = serial.Serial(porta, baud_rate, timeout=1)
except Exception as exceto:
    print(f"Erro ao abrir a porta {porta}: {exceto}")
    input("Pressione Enter para sair...")
    sys.exit(1)

#------------------------------|
#ESCRITA DE ARQUIVO:

#se o arquivo for novo:
if not os.path.exists(dados_telemetria) or os.path.getsize(dados_telemetria) == 0:
    with open(dados_telemetria, "w", encoding="utf-8") as documento:
        documento.write(cabecalho)

# abrir o arquivo
with open(dados_telemetria, "a", encoding="utf-8") as documento:
    print ("Conectado! dados de telemetria sendo gravados...")
    while True:
        try:
            linha = conexao_porta_serial.readline().decode("utf-8", errors="ignore").strip()
            if linha:
                #ignora mensagens de debug que não sejam dados
                documento.write(linha + "\n")
                documento.flush() #Salva a linha no disco do PC
                print(f"Salvo: {linha}")
        except KeyboardInterrupt:
                print("Gravação encerrada.")
                break
conexao_porta_serial.close()
input("\nPressione Enter para fechar...")
