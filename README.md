# Sistemas Embebidos
Proyecto final de Sistemas embebidos: Resafeve

Mateo Bernal Bonil - Valentine Chimezie Muodum Prieto - Esteban Ladino Fajardo - Jose Ricardo Rincon Pelaez


Se quiere hacer un sistema de monitoreo en reservas naturales para reservas naturales, en el cual se instalan microfonos en arboles para el regustro de audio y su procesamiento con IA en un servidor aparte. Los micrfonos se controlan con ESP32, la comunicación se plantea por medio de MQTT al servidor Mosquitto situado en una Raspberry, conectada tambien a un servidor online.

![Esquema general](https://i1.wp.com/randomnerdtutorials.com/wp-content/uploads/2018/05/ESP32-MQTT-bme280.png?w=600&quality=100&strip=all&ssl=1)

Luego la inteligencia artificial usada se encuentra en: https://github.com/IBM/MAX-Audio-Classifier . Además la ESP32 deberá correr un sistema operativo en tiempo real para su mejor rendimiento, en este caso se usa FreeRTOS.

El proceso planteado es el siguiente:
* Registrar alrededor de 10 segundos en un arreglo de bytes en el ESP32.
* Enviar la señal en bytes desde el ESP32 hasta la raspberry por medio de mqtt.
* Luego en la raspberry convertir los datos en bytes en un archivo de audio .wav.
* Procesar con la IA el archivo de audio y obtener resultados.
* Finalmente mostrar los resultados en una pagina web para la toma de decisiones.


Adicionalmente en la carpeta del SistemasEmbebidos/ESP32/EjemplosGuia/ se encuentran los siguientes archivos que sirven como base para realizar pruebas sencillas y entender el manejo de las herramientas usadas:
* freeRTOS.ino, que es un codigo desarrollado en arduino para la ESP32 en el que se ejemplifica el uso del FreeRTOS con multiples tareas.
* esp32_Multitasking.ino, que es otro ejemplo del uso de FreeRTOS pero aprovechando los dos cores del ESP32.
* mqtt_ESP32.ino, es un ejemplo de uso de MQTT en el ESP32 para muestrar datos, publicarlos, y recibir información. Como servidor de prueba se usa test.mosquitto.org, que es gratuito y disponible en internet para todo publico.
* I2C_Mic.ino, es el codigo de prueba más sencillo posible el microfono INMP441 con la interfaz I2S en el ESP32.
* python_mqtt_client.py, es un script de python sencillo que recibe y publica mensajes por medio de MQTT.

## Funcionalidad de la ESP32
El microcontrolador usado en el proyecto es el ESP32 de referencia Wemos Loolin32. Este dispositivo cuenta con dos cores y la posibilidad de conectarse a internet. 

![Pin out ESP32](https://http2.mlstatic.com/D_NQ_NP_668961-MCO44328132325_122020-O.jpg)

Con el desarrollo del proyecto se evidenciaron y señalaron las limitaciones del dispositivo.

### Microfono electret
En primera instancia se probo para el muestreo de audio el uso de un micrfono electret con una configuración de amplifiación ya usada en otros proyectos. Luego el ESP32 en alguno de sus pines de lectura deberia de ser capaz de muestrear la señal a la frecuencia deseada con el uso de un while y un delay. El archivo ESP32_Audio_mqtt_electret.ino en la carpeta del proyecto, se muestra el codigo en cuestion.

Luego, una primera pregunta se plantea ¿Cuantos segundos puede grabar el ESP32?¿De que tamaño es el Packete que se enviara?
De acuerdo a la documentación de pubsubclient el paquete máximo que se puede enviar son 256 bytes con la configuración. Es decir, hay un limite en el tamaño de paqute que se puede enviar, más en la propia documentación se explica que con la funcion client.setBufferSize(MQTT_MAX_PACKET_SIZE); 
Luego la duda es por el lado de que tamaño es limite máximo que podemos tener. La cuestion es que entre más grande el paqute más se depende de la conexión a internet.

Cuando se implemento el codigo se ecnotro con el primer problema, la lectura de los pines del ESP32 no funcionaba. ¿Por qué no funciona el AnalogRead en el código?
En cuanto al error de lectura del ESP32 cuando se usa el WiFi y MQTT el problema son que pines se eligen. Tal como indica la documentación y un foro, algunos Pines (como el 13 que se estaba usando) usan registros ADC2 que son los que manejan las funciones de WiFi de forma que si el WiFi esta activado quedan inhabilitados. Los pines que no se pueden usar son:  GPIO0, GPIO2, GPIO4, GPIO12 - GPIO15, GOIO25 - GPIO27. Luego de cambiar a los pines 34 para la lectura la lectura ya era posible. 

Con el montaje inicial se desarrollo un script de python que recibia la información enviada del ESP32 y usaba multiples librerias. El proceso era:
* Se divide la información en el ESP32 en dos bytes por dato. La lectura es de 12 bits escalada a 16 bits. Pero el canal es de solo un byte de ancho. Además la inteligencia artificial pide idealmente un audio de 10s con una frecuencia de muestreo de 44100Hz y una resolución de 16 bits.
* En python se recibió esta información y con la funcion numpy.frombuffer se llega a la solución ya que retorna un arreglo unidimensional a partir de la señal de entrada https://numpy.org/doc/stable/reference/generated/numpy.frombuffer.html?highlight=frombuffer#numpy.frombuffer 
* En el archivo python se usa luego la función numpy.concatenate para crear una variable global con toda la información recibida en todos los paquetes. https://es.stackoverflow.com/questions/10768/crear-variables-globales-en-python 
* Finalmente se usan variables para contar los paquetes y datos y cuando se llega al punto deseado se usa https://docs.scipy.org/doc/scipy/reference/generated/scipy.io.wavfile.write.html#scipy.io.wavfile.write para crear el .wav

Este proceso funcionaba pero la calidad del sonido era mala y los tiempos de eran mucho más largos de lo esperado (enviar 9 segundos costaba casi minuto y medio). Una alternativa era usar las funciones beginplubish() y endpublish() de PubSubClient para que el proceso fuese más bien un stream de datos. Más sin embargo, estas dos funciones no enviaban toda la información por algún error desconocido. 

La solución inicial fue bajar la frecuencia de muestreo, de 44100Hz a 8000Hz. Esto soluciono el problema del tiempo que ahora en total solo 16 segundos, es decir 2 segundos de envio por paquetes de 3 segundos. Paquetes mayores a 3 segundos con las condiciones descritas llenan la memoria. En terminos generales se espera un tamaño de paquete alrededor de 48000 bytes. 
Por otro lado el problema de la calidad del audio seguia existiendo. Se mejoro entonces la etapa de amplificación como muestran las imagenes.
![Montaje con amplificación y micrfono electret](https://drive.google.com/file/d/1DOD7S09CEUXWwYKmG7pCqd7SxsyzbBJW/view)

### Microfono I2s INMp4

### PubSubClient
Para la comunicación se usa el protocolo de MQTT, que en el caso de la ESP32 se conecta con la librería PubSubClient, y para los demás elementos se usa la líbreria de python paho-mqtt. MQTT usa un sistema de subscripciones para la publiación de mensajes a topicos especificos en el servidor.

### FreeRTOS

## Funcionalidad DockerServicios

Se encuentra implementado para una arquitectura AMD y ARMv7. En general los siquientes comandos se usan para ambas arquitecturas:

* Hacer una imagen (no olvidar estar en el directorio del Dockerfile y el punto al final):
```bash
sudo docker build -t <nombre_imagen>:tag . 
```
* Para montar servicios desde docker compose:
```bash
sudo docker-compose up -d 
```

* Ver servicios montados:

```bash
sudo docker ps 
```

* Para reiniciar los servicios de docker compose (útil cuando se hace algún cambio):
```bash
sudo docker-compose restart 
```

* Para parar y eliminar los servicios de docker compose:
```bash
sudo docker-compose down 
```



### AMD 

### ARM



## Referencias
Recursos adicionales
* MQTT - Resumen de todo con explicaciones y links de temas relacionados: https://aprendiendoarduino.wordpress.com/2018/11/19/mqtt/ 
* Esp32 Mqtt explicado: https://randomnerdtutorials.com/esp32-mqtt-publish-subscribe-arduino-ide/
* Inteligencia Artificial que clasifica los sonidos: https://www.google.com/url?q=https://github.com/IBM/MAX-Audio-Classifier&sa=D&source=editors&ust=1630442220276000&usg=AOvVaw06XKVdFrBWjtAnfdxmKH8F 
* Documentacion Espressif del ESP32: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/index.html
