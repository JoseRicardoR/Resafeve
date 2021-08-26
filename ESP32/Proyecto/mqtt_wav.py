import paho.mqtt.client as mqtt

## SETUP Constants
#broker_address="192.168.1.184"  #as ip
broker_address="3.16.69.93" #use external broker or server
port = 1883
sub_topic = "ESP32/Data_output"
pub_topic = "ESP32/Signal_input"
## Variables globales
num_mess = 0
samplerate = 16000
time = 6        #en segundos
buffersize = 9600             #numero de bytes en cada paquete
packet_size = buffersize/2      #cada dato real requiere 2 bytes
packet_number = round( ((samplerate*16*time/8)/buffersize) +0.49)    #(fs*16bits*t/2)/buffersize 
print(packet_number)

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe(sub_topic)

# The callback for when a PUBLISH message is received from the server.

f = open('audio.wav', 'wb')
def on_message(client, userdata, msg):
    # Hey Python, voy a usar una variable global
    global num_mess
    num_mess = num_mess+1 
    print("Write")
    f.write(msg.payload) 
    print(str(num_mess/(packet_number+1)*100)+"%")
    if num_mess == (packet_number+1):
        f.close()
  
    #print(msg.payload)

client = mqtt.Client()      # se instancia el cliente
client.on_connect = on_connect      # funcion que se ejecuta una vez conectado
client.on_message = on_message      #  funcion que se ejecuta cada nuevo mensaje 

client.connect(broker_address, port)   # conexion al broker/servidor

client.loop_forever() # Loop que mantiene el cliente funcionando. Crt + c en consola para acabar script