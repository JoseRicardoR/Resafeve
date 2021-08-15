import paho.mqtt.client as mqtt
import numpy as np
from scipy.io.wavfile import write

## SETUP Constants
#broker_address="192.168.1.184"  #as ip
broker_address="test.mosquitto.org" #use external broker or server
port = 1883
sub_topic = "raiz/output"
pub_topic = "raiz/input"
## Variables globales
samplerate = 8000
packet_size = 24000
packet_number = 3
value = np.array([])
data_audio = np.array([])
num_mess = 0

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe(sub_topic)

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    # Hey Python, voy a usar una variable global
    global value
    global num_mess
    global data_audio
    # Se reciben los datos en bytes del payload. Cada dos bytes hay un valor. La funcion frombuffer
    # los convierte en uint16. Luego se transforman a int16 y se centran en cero.
    Uvalue = np.frombuffer(msg.payload, dtype=np.uint16, count=-1)
    TransitionValue = np.int32(Uvalue)-32768
    value = np.int16(TransitionValue)
    # Se usa un contador del tamaño de paquetes de interes y de cantidad de paquetes
    if value.size == packet_size:       # paquetes deseados
        if num_mess <= packet_number:     # cantidad de paquetes deseados
            data_audio = np.concatenate([data_audio,value])     # se vavn añadiendo los paquetes a uno solo
        else :
            num_mess = 1
            data_audio = np.concatenate([data_audio,value])
        num_mess = num_mess+1         # contador de paquetes
    # impresion para verificar el contenido, tipo de dato y contador de cada paquete
    #print(msg.topic+" "+str( value) +" "+str( value.dtype) + " Size:" +str( value.size)+ " Numero:" +str(num_mess))
    print(data_audio.size)  # visualizacion de cantidad de datos

    if data_audio.size >= (packet_number*packet_size):   # una vez se alcanza la cantidad de datos deseada se crea el .wav
        #print("Audio: "+ str(data_audio) + " Size:" +str( data_audio.size) ) # en realidad esto no se puede imprimir
        write("example.wav", samplerate, data_audio.astype(np.int16) )
        
        num_mess = 0 
        # deberia limpiarse la variable global para iniciar de cero pero hay 
        # que darle un tiempo al .wav de crearse
        # tengo crear un delay
        #data_audio = np.array([])      

    #print(msg.payload)

client = mqtt.Client()      # se instancia el cliente
client.on_connect = on_connect      # funcion que se ejecuta una vez conectado
client.on_message = on_message      #  funcion que se ejecuta cada nuevo mensaje 

client.connect(broker_address, port)   # conexion al broker/servidor

client.loop_forever() # Loop que mantiene el cliente funcionando. Crt + c en consola para acabar script
