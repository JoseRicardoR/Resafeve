#Lo que se busca con este scrip es utilizar la inteligencia artificial para analizar los archuvos .wav y publicarlos por medio
#Del broker.

import paho.mqtt.client as mqtt #import the client1
import time
import os

class Tool():
    def publish(self,broker_address,port,topico,mensaje):
        '''
        Esta función permite publicar en un broker mqtt
        - broker_address: IP donde se escuche el broker mqtt
        - port: Puerto expuesto para el broker mqtt
        - topico: tópico al que se quiere publicar
        - mensaje: mensaje para publicar en el tópico
        '''
        try:
            client = mqtt.Client("P1") #create new instance
            client.connect(broker_address,port) #connect to broker
            client.publish(topico,mensaje)#publish
        except ConnectionRefusedError:
            print(f'Por favor, revise la conexión del broker en {broker_address}:{port}')
        return
    

    def procesar(self,direccionSamples,AI_puerto,AI_address,archivoResultados):
        '''
        Esta funcíon permite mandar una orden a la linea de comandos
        direccionSamples: Dirección relativa de las muestras con la extensión .wav
        puerto: Puerto expuesto para el MAX-Audio-Classifier 
        archivoResultados: Es el archivo que guarda los resultados con la extensión .json(Puede ser otra)
        '''    
    
        os.system(f'curl -F "audio=@{direccionSamples};type=audio/wav" -XPOST \
            http://{AI_address}:{AI_puerto}/model/predict > {archivoResultados}')
        # Comando tomado de: https://github.com/IBM/MAX-Audio-Classifier en command line
        return
    
    
    def leer(self,archivoResultados):
        '''
        Esta función lee lo que hay dentro de un documento
        - archivoResultados: Archivo que se desea leer
        '''
        try:
            with open(os.getcwd()+'/'+archivoResultados, encoding='utf-8') as f:
                contents = f.read()
                f.close()
                print('H:',contents)
        except FileNotFoundError:
            print(f"Sorry, the file {os.getcwd()+'/'+archivoResultados} does not exist.")

        return str(contents)
     

if __name__=="__main__":
    #Instanciamiento
    tool=Tool() 
    #ip del broker mqtt
    broker_address="localhost"
    #broker_address=AI_address="localhost"
    #Puerto Expuesto del broker
    port=1883
    #Topico al que se desea publicar
    topico="Esteban/IA"
    #Periodo de espera .wav
    tiempoEspera=1
    #Archivo para guardar los resultados de la inteligencia artificial (AI)
    archivoResultados='resultados.json'
    #Dirección relativa de la muestra que se quiere analiza con la AI
    direccionSamples='samples/audio.wav'
    #Puerto que expone la aplicación de la AI
    AI_puerto=5000
    #Dirección de AI
    AI_address="localhost"
    #AI_address="localhost"
    # Se inicializa el mensaje de publicación en el broker mqtt
    mensaje=''


    while True:
    # Ciclo infinito de análisis
    #while True:
        
        tool.procesar(direccionSamples,AI_puerto,AI_address,archivoResultados)
        mensaje=tool.leer(archivoResultados)
        tool.publish(broker_address,port,topico,f'"{mensaje}"')
        time.sleep(tiempoEspera)



