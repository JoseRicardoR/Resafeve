import paho.mqtt.client as mqtt  # Libreria MQTT

# ... Archivo encendido...
print("Receptor de audio encendido")

# .............. SETUP Constants........................
broker_address = "179.32.157.54"  # use external broker or server
port = 1883
sub_topic = "ESP32/Data_output"
pub_topic = "ESP32/Signal_input"
# Variables globales
samplerate = 16000  # Frecuencia de muestreo
t = 6  # tiempo total en segundos
buffersize = 24000  # numero de bytes en cada paquete recibido
resolution = 16   #resolucion en bits
recordsize = samplerate * resolution / 8 * t
# n_pack = (fs*16bits*t/2)/buffersize
packet_number = round((recordsize/buffersize) + 0.49)
num_mess = 0                    # numero de mensajes recibidos
AudioListo = False             # booleano que indica si el audio ya puede ser enviado a la IA
Escribiendo = False            # boolenao que indica si se esta escribiendo el audio en este momento
# ..............................................................

# .......... .wav Setup.....................................
filename = "Audio.wav"  # nombre del archivo de audio
f = open(filename, 'wb')   # se crea el archivo de audio, wb = write-bytes

header = bytearray(b'\x01') * 44
header[0] = 0x52
header[1] = 0x49
header[2] = 0x46
header[3] = 0x46
fileSize = int(recordsize + 44 - 8)
byte_filesize = fileSize.to_bytes(4,'little')
header[4] = byte_filesize[0]  # Audio size..
header[5] = byte_filesize[1]
header[6] = byte_filesize[2]
header[7] = byte_filesize[3]  # ..
header[8] = 0x57
header[9] = 0x41
header[10] = 0x56
header[11] = 0x45
header[12] = 0x66
header[13] = 0x6D
header[14] = 0x74
header[15] = 0x20
header[16] = 0x10  # Resolucion en bits..
header[17] = 0x00
header[18] = 0x00
header[19] = 0x00  # ..
header[20] = 0x01  # Audio format= 1(PCM) ..
header[21] = 0x00  # ..
header[22] = 0x01  # numero de canales ..
header[23] = 0x00  # ..
header[24] = 0x80  # Samplerate ..
header[25] = 0x3E
header[26] = 0x00
header[27] = 0x00  # ..
header[28] = 0x00  # Byte Rate .. SampleRate * NumChannels * BitsPerSample/8
header[29] = 0x7D
header[30] = 0x00
header[31] = 0x00  # ..
byte_blockalign = ( int(resolution/8) ).to_bytes(2,'little') 
header[32] = byte_blockalign[0]  # Block align .. NumChannels * BitsPerSample/8
header[33] = byte_blockalign[1]
byte_resolution = (resolution).to_bytes(2,'little')
header[34] = byte_resolution[0] # BitsPerSample = resolution
header[35] = byte_resolution[1]  # ..
header[36] = 0x64
header[37] = 0x61
header[38] = 0x74
header[39] = 0x61
byte_recordsize = int(recordsize).to_bytes(4,'little')
header[40] = byte_recordsize[0]  # Audio size..
header[41] = byte_recordsize[1]
header[42] = byte_recordsize[2]
header[43] = byte_recordsize[3]  # ..


#............. Recibimiento de datos .........................
# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT Broker! Result:"+str(rc))
    else:
        print("Failed to connect, return code %d\n", rc)
    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe(sub_topic)

#Called when the client disconnects from the broker
def on_disconnect(client, userdata, rc):
    if rc != 0:
        print("Unexpected disconnection.") 

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    # Hey Python, voy a usar una variable global
    global num_mess
    global Escribiendo
    global AudioListo
    global f

    #print(msg.payload)
    #si el es el primer paquete se crea el file con el header
    if str(msg.payload) == "b'Empieza'" :
        f = open(filename, 'wb')   # se crea el archivo de audio, wb = write-bytes
        f.seek(0)          #se escribe desde cero
        num_mess = 0
        f.write(header)        # se le añade el header al file
        print("Inicia escritura audio...")
        Escribiendo = True    
        AudioListo = False   # a partir de aqui se esta escribiendo el audio, no esta listo para la IA
    elif Escribiendo == True :
        #Luego escribe el paquete de datos recibido y cuenta el pauete recibido
        f.write(msg.payload)
        num_mess = num_mess+1
        #Se imprime en consola el % de mensaje recibido
        print("Writing... " + str(num_mess/(packet_number)*100)+"%")

    #si ya llegaron todos los pauetes esperados se cierra el file
    if num_mess == (packet_number):
        f.close()       # cerramos el archivo
        print("Acaba escritura audio...")
        Escribiendo = False  
        AudioListo = True   #El audio ya se puede enviar a la IA, hay 20 segundos para ello    

    


# ...............Conexion con MQTT..............................
client = mqtt.Client()              # se instancia el cliente

client.on_connect = on_connect      # funcion que se ejecuta una vez conectado
client.on_disconnect = on_disconnect  #Called when the client disconnects from the broker
client.on_message = on_message      # funcion que se ejecuta cada nuevo mensaje
client.connect(broker_address, port)  # conexion al broker/servidor
# Loop que mantiene el cliente funcionando. Crt + c en consola para acabar script
client.loop_forever()
