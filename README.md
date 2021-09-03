# Sistemas Embebidos
Proyecto final de Sistemas embebidos: Resafeve

Mateo Bernal Bonil - Valentine Chimezie Muodum Prieto - Esteban Ladino Fajardo - Jose Ricardo Rincón Peláez


Se quiere hacer un sistema de monitoreo en reservas naturales para reservas naturales, en el cual se instalan microfonos en arboles para el regustro de audio y su procesamiento con IA en un servidor aparte. Los micrfonos se controlan con ESP32, la comunicación se plantea por medio de MQTT al servidor Mosquitto situado en una Raspberry, conectada tambien a un servidor online.
En el siguiente esquema se ve la logica de funcionamiento, aunque en nuestro caso se hace uso de un servidor Mosquito, y el sensor es un microfono. Además si bien se pueden recibir mensajes con el ESP32 (y además se implemento dicha función) no fue necesario una aplicación que usara ese camino.

![Esquema general](https://i1.wp.com/randomnerdtutorials.com/wp-content/uploads/2018/05/ESP32-MQTT-bme280.png?w=600&quality=100&strip=all&ssl=1)

Luego la inteligencia artificial usada se encuentra en: https://github.com/IBM/MAX-Audio-Classifier . Además la ESP32 deberá correr un sistema operativo en tiempo real para su mejor rendimiento, en este caso se usa FreeRTOS.

El proceso planteado es el siguiente:
* Registrar alrededor de 10 segundos en un arreglo de bytes en el ESP32.
* Enviar la señal en bytes desde el ESP32 hasta la raspberry por medio de mqtt.
* Luego en la raspberry convertir los datos en bytes en un archivo de audio .wav.
* Procesar con la IA el archivo de audio y obtener resultados.
* Finalmente mostrar los resultados en una pagina web para la toma de decisiones.

En cuanto a este repositorio, el archivo README es una documentación extensa y detallada de lo desarrollado, y se encuentran cuatro carpetas con las partes del proyecto. La carpeta de DockerServices contiene todas las aplicaciones de Docker además de las herramientas necesarias, la carpeta ESP32 contiene todos los archivos relacionados con el microcontrolador, la carpeta Front_end la infromación relacionada a la pagina web y la carpeta Prototipo la información relacionada con el producto fisico en cuestion.

## Funcionalidad de la ESP32
En la carpeta del SistemasEmbebidos/ESP32/EjemplosGuia/ se encuentran los siguientes archivos que sirven como base para realizar pruebas sencillas y entender el manejo de las herramientas usadas:
* freeRTOS.ino, que es un codigo desarrollado en arduino para la ESP32 en el que se ejemplifica el uso del FreeRTOS con multiples tareas.
* esp32_Multitasking.ino, que es otro ejemplo del uso de FreeRTOS pero aprovechando los dos cores del ESP32.
* mqtt_ESP32.ino, es un ejemplo de uso de MQTT en el ESP32 para muestrar datos, publicarlos, y recibir información. Como servidor de prueba se usa test.mosquitto.org, que es gratuito y disponible en internet para todo publico.
* I2C_Mic.ino, es el codigo de prueba más sencillo posible el microfono INMP441 con la interfaz I2S en el ESP32.
* python_mqtt_client.py, es un script de python sencillo que recibe y publica mensajes por medio de MQTT.

El microcontrolador usado en el proyecto es el ESP32 de referencia Wemos Loolin32. Este dispositivo cuenta con dos cores y la posibilidad de conectarse a internet. 

![Pin out ESP32](https://http2.mlstatic.com/D_NQ_NP_668961-MCO44328132325_122020-O.jpg)

Con el desarrollo del proyecto se evidenciaron y señalaron las limitaciones del dispositivo.

### PubSubClient
Para la comunicación se usa el protocolo de MQTT, que en el caso de la ESP32 se conecta con la librería PubSubClient, y para los demás elementos se usa la líbreria de python paho-mqtt. MQTT usa un sistema de subscripciones para la publiación de mensajes a topicos especificos en el servidor.

Luego, unas primeras preguntas fueron ¿Cuantos segundos puede grabar el ESP32?¿De que tamaño es el Packete que se enviara?
De acuerdo a la documentación de pubsubclient el paquete máximo que se puede enviar son 256 bytes con la configuración. Es decir, hay un limite en el tamaño de paqute que se puede enviar, más en la propia documentación se explica que con la funcion client.setBufferSize(MQTT_MAX_PACKET_SIZE); se puede elegir el tamaño máximo del Buffer.
Luego la duda es por el lado de que tamaño es limite máximo que podemos tener. La cuestion es que entre más grande el paqute más se depende de la conexión a internet.

### Microfono electret
En primera instancia se probo para el muestreo de audio el uso de un micrfono electret con una configuración de amplifiación ya usada en otros proyectos. Luego el ESP32 en alguno de sus pines de lectura deberia de ser capaz de muestrear la señal a la frecuencia deseada con el uso de un while y un delay. El archivo ESP32_Audio_mqtt_electret.ino en la carpeta del proyecto, se muestra el codigo en cuestion.

Cuando se implemento el codigo se encotro con el primer problema, la lectura de los pines del ESP32 no funcionaba. ¿Por qué no funciona el AnalogRead en el código?
En cuanto al error de lectura del ESP32 cuando se usa el WiFi y MQTT el problema son que pines se eligen. Tal como indica la documentación y un foro, algunos Pines (como el 13 que se estaba usando) usan registros ADC2 que son los que manejan las funciones de WiFi de forma que si el WiFi esta activado quedan inhabilitados. Los pines que no se pueden usar son:  GPIO0, GPIO2, GPIO4, GPIO12 - GPIO15, GOIO25 - GPIO27. Luego de cambiar a los pines 34 para la lectura la lectura ya era posible. 

Con el montaje inicial se desarrollo un script de python (en la carpeta SistemasEmbebidos/ESP32/Proyecto/python_mqtt_client_packages.py ) que recibia la información enviada del ESP32 y usaba multiples librerias. El proceso era:
* Se divide la información en el ESP32 en dos bytes por dato. La lectura es de 12 bits escalada a 16 bits. Pero el canal es de solo un byte de ancho. Además la inteligencia artificial pide idealmente un audio de 10s con una frecuencia de muestreo de 44100Hz y una resolución de 16 bits.
* En python se recibió esta información y con la funcion numpy.frombuffer se llega a la solución ya que retorna un arreglo unidimensional a partir de la señal de entrada https://numpy.org/doc/stable/reference/generated/numpy.frombuffer.html?highlight=frombuffer#numpy.frombuffer 
* En el archivo python se usa luego la función numpy.concatenate para crear una variable global con toda la información recibida en todos los paquetes. https://es.stackoverflow.com/questions/10768/crear-variables-globales-en-python 
* Finalmente se usan variables para contar los paquetes y datos y cuando se llega al punto deseado se usa https://docs.scipy.org/doc/scipy/reference/generated/scipy.io.wavfile.write.html#scipy.io.wavfile.write para crear el .wav

Este proceso funcionaba pero la calidad del sonido era mala y los tiempos de eran mucho más largos de lo esperado (enviar 9 segundos costaba casi minuto y medio). Una alternativa era usar las funciones beginplubish() y endpublish() de PubSubClient para que el proceso fuese más bien un stream de datos. Más sin embargo, estas dos funciones no enviaban toda la información por algún error desconocido. 

La solución inicial fue bajar la frecuencia de muestreo, de 44100Hz a 8000Hz. Antes de ello se probo en la inteligencia artificial que aún con audios de 8000Hz se obtuvieran resultados decentes. En comparación los resultados se muestran a continuación:
```
* 44100 Hz: {"status": "ok", "predictions": [{"label_id": "/m/01j4z9", "label": "Chainsaw", "probability": 0.6540094614028931}, {"label_id": "/m/04_sv", "label": "Motorcycle", "probability": 0.1308945119380951}, {"label_id": "/t/dd00066", "label": "Medium engine (mid frequency)", "probability": 0.12884050607681274}, {"label_id": "/m/0_ksk", "label": "Power tool", "probability": 0.11332365870475769}, {"label_id": "/m/07yv9", "label": "Vehicle", "probability": 0.10684427618980408}]}
```
```
* 8000 Hz: {"status": "ok", "predictions": [{"label_id": "/m/01j4z9", "label": "Chainsaw", "probability": 0.4117594063282013}, {"label_id": "/m/04_sv", "label": "Motorcycle", "probability": 0.206061452627182}, {"label_id": "/m/07yv9", "label": "Vehicle", "probability": 0.1337493658065796}, {"label_id": "/m/0_ksk", "label": "Power tool", "probability": 0.1071915328502655}, {"label_id": "/t/dd00066", "label": "Medium engine (mid frequency)", "probability": 0.07981646060943604}]}
```

Evidentemente con la frecuencia de muestreo mayor son mejores los resultados más con la frecuencia un poco más baja igual funciona.
Esto soluciono el problema del tiempo que ahora en total solo 16 segundos, es decir 2 segundos de envio por paquetes de 3 segundos. Paquetes mayores a 3 segundos con las condiciones descritas llenan la memoria. En terminos generales se espera un tamaño de paquete alrededor de 48000 bytes. 
Por otro lado el problema de la calidad del audio seguia existiendo. Se probaron entonces mejores etapas de amplifiación como las mostradas en las imagenes.

![Circuitos de amplificación señal de microfono electret](https://user-images.githubusercontent.com/36318647/131569777-574a6480-3d16-4da9-88e6-5e8b576d438a.PNG)

Pese a esto el montaje mostrado en la siguiente figura no tenia una buena calidad de sonido, además de tener una frecuencia de muestreo baja.
Montaje con amplificación y micrfono electret.

<img src="https://user-images.githubusercontent.com/36318647/131569044-76cfa8a4-bb35-49d9-85ca-5633d9144608.jpg" width="350">

En todo caso no se consiguio que la señal de audio fuera mejor, ni añadiendo condensadores de desacople, en su mayoria es ruido como se ve en la siguiente imagen. Por ende el sonido no es legible.

![Audio Con microfono electret](https://user-images.githubusercontent.com/36318647/131570448-9fa7ce11-8515-4e02-b2bc-db267fbcb3ff.PNG)

Se buscaron entonces alternativas y se encontro una de cambiar el sensor del proyecto.

### Microfono I2S INMP441
Tal parece que la conversion analoga digital del ESP32 consume mucho más CPU de la que se espera y hace que los procesos sean más demorados. Además si bien el circuito de amplificación etsa bien, se hace necesario el uso de un filtro para limpiar la señal. Se encontro que un micrfono Mems que usa la interfaz I2S se desarrollo para este tipo de proyectos. Su bloque funcional es como se ve a continuación.

![INMP441](https://user-images.githubusercontent.com/36318647/131572006-06363d38-0d76-4d52-8974-ec905b930f80.PNG)

Luego como ya todo esta incorporado en este micrfono la labor consiste en configurar el software para su buen funcionamiento. Al incio se obtuvo un problema visible en la imagen a continuación. 

![Audio con errores](https://user-images.githubusercontent.com/36318647/131572463-9cfbcac7-a6b4-4371-929d-dd87cb254780.PNG)

El error consiste en que a veces el audio enviado no se graba correctamente, las muestras no se llenan por completo y hay muchos datos en 00. En consecuencia el audio suena entrecortado y “robótico”. Pese a las múltiples pruebas no he descubierto que genera estas inconsistencias, ni el tamaño ni cantidad de dma, ni los pines, ni el APLL-CLK, ni el tamaño de paquete ni audio total. Sencillamente, a veces funciona y a veces no.

Revisando el montaje se encuentra que uno de los pines esta mal conectado. Si se usa solo la señal mono izquierda es decir, .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,  se debe conectar el pin L/R a ground obligatoriamente. Dejarlo sin conectar como sugieren algunos tutoriales puede ocasionar el problema encontrado.

El montaje final es como se muestra a continuación. Muy sencillo.

<img src="https://user-images.githubusercontent.com/36318647/131571799-88136bbe-3e79-4f17-9165-17f1c73cafd3.jpg" width="500">

El codigo completo se encuentra en la carpeta del proyecto con el nombre ESP32_I2S_Sampling.ino.
La señal de audio de salida es en definitiva clara y sin interrupciones. Se puede además muestrear a mayores frecuencias, se elige la de 16000Hz para no saturar de datos la red y la memoria del ESP32.

<img src="https://user-images.githubusercontent.com/36318647/131574616-8b3a6e98-f21a-474e-bff3-a4ae040e81f5.PNG" width="800">

Tambien se cambio el script de python para que ahora solo se requiera la libreria de Pahoo, así pues lo que se hace es generar un header al inicio del archivo que lo hace legible como .wav. Dicho header varia en relación a la frecuencia de muestreo, el numero de bits de resolución y el largo del archivo .wav.
El archivo definitivo tiene el nombre de ESP32_audio_receiver.py, y la explicación del header se puede encontrar en: http://soundfile.sapp.org/doc/WaveFormat/.

### FreeRTOS

En cuanto al componente FreeRTOS del proyecto, como se utiliza solo una función para todo el proceso en el ESP32, se procedio a hacer una tarea que contuviese todos estos elementos. En el archivo ESP32_I2S_Sampling.ino se ve como la tarea i2s_adc es instanciada como una task de FreeRTOS y este se reincia una vez cumple su ciclo. En una implementación donde se quisiese muestrear con dos microfonos al tiempo se podria crear una segunda tarea i2s_adc_2 con la misma funcionalidad. 



## Funcionalidad de la FPGA

La tarjeta FPGA usada para este proyecto es la Blackice II. Este dispositivo cuenta con la posibilidad de ser programado mediante un entorno grafico llamado
Icestudio mediante bloques que representan los modulos que normalmente se insatancian al ser usado un lenguaje de descripción de hardware convencional
(Verilog o VHDL).
![Blackice II](https://user-images.githubusercontent.com/42346359/131914705-aa9a34cd-44ce-4965-8141-9e75d155edac.jpg)

### Tareas
La tarea principal a realizar por la FPGA en este proyecto consiste en la deteccion de audios validos que puedan ser analizados por medio de una inteligencia artificial,
esto con el fin de no saturarla con muchas peticiones procedentes de varios microfonos. Para que un audio se considere como valido, un porcentaje arbitrario de las
muestras que conforman una grabación (este porcentaje es determinado por el diseñador), deberá superar un umbral de intensidad (tambien establecido por el diseñador),
esto con el fin de determinar que el audio en efecto si puede contener información relevante y no es simplemente silencio o ruido intermitente.

Como ya se mencionó el ESP32 enviará la información referente a la Raspberry por medio de un servidor MQTT. Esta información será enviada a la FPGA por medio de un protocolo
de comunicacion serial (SPI), para su analisis y posterior devolución de resultados. En esta aplicacion la Raspberry sera el denominado "Maestro" y la FPGA el "esclavo" debido
a como se realizan las solicitudes y quien es quien necesita enviar los comandos para la correcta operación del modulo.

![Maestro_Esclavo](https://user-images.githubusercontent.com/42346359/131917107-d548e8ba-ecaf-44e4-b512-ff519bc68c69.PNG)

Para esta implementación se hara uso del bloque esclavo presente en una de las librerias de Icestudio.


![Bloque_esclavo](https://user-images.githubusercontent.com/42346359/131917700-eeedaf32-6fe0-4fb4-bad7-48f44ca58fde.PNG)


Cada muestra de audio es representada por 2 Bytes de datos (16 bits), Sin embargo el bus de datos de este modulo es de 8 bits (que son recibidos serialmente desde el 
maestro y organizados en un registro propio del modulo). Por tanto se asignara 2 posiciones de memoria (2 registros de 8 bits mapeados en derterminadas direcciones de memoria),
a una muestra de audio en un instante. Para el envio de esta señal a la FPGA se cuenta con el bloque "SPI-cmd-regs" que se encargara de primero apuntar a la dirección de
alguno de los registros de la muestra instantanea y luego escribir el dato designado por el maestro, tambien es posible leer este dato en caso de que se requiera hacer alguna comparación (notese que se requerirá hacer dos veces este proceso para llenar totalmente los registros asignados a la muestra). 
![SPI_Control_Mapeo](https://user-images.githubusercontent.com/42346359/131919736-5fa2e7a1-381f-472d-ac27-f6eae8ded62c.PNG)

![image](https://user-images.githubusercontent.com/42346359/131919916-3c2372d1-f5ba-42b9-a13d-55d4d770a59c.png)

Arriba se puede ver el control de las posiciones de memoria de todos los registros asociados a la FPGA junto al diagrama que hace posible la concatenanción para la creación de
la muestra instantanea.

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
* Para mirar las imágenes creadas:
```bash
sudo docker images
```

* Para eliminarlas:
```bash
sudo rmi <id/name>
```

Se eligió montar todos los servicios en contenedores Docker dada la portabilidad.
### Servicios implementados en Docker

Se tomaron los mismos servicios que Sanches [Sanches] y las mismas configuraciones con algunas excepciones que se van a explicar.


#### Docker-compose

En el curso introductorio del Youtuber [Pelado Nerd](https://www.youtube.com/watch?v=CV_Uf3Dq-EU) sobre docker se comenzó a montar contenedores desde comandos básicos hasta el punto de usar docker compose. Docke-compose hace parte de docker y su principal ventaja radica en organizar los volúmenes, las dependecias, los puertos y demás cosas en un archivo con el nombre *docker-compose.yml*. Sin embargo, la principal característica es que se crea por defecto una red entre los sevicios que se crean con el alias de cada servicio de tal forma que entre ellos pueden resolverse la ip mediante su alias. Por ejemplo en la siguiente sección de código el Broker MQTT tiene la ip en el alias *mosquitto*.

```
services:
    mosquitto:
        image: eclipse-mosquitto:2.0.10
```

Esto es muy necesario dado que los servicios (o contenedores) pueden cambiar la IP y no se hace práctico en un proyecto dado que generaría fallas de comunicación.
#### Broker MQTT

Para esto se eligió mosquitto y esta es su configuración en docker compose:
```
mosquitto:
        image: eclipse-mosquitto:2.0.10
        restart: always
        ports: 
            - 1883:1883  
        volumes:
            - ./BaseDatos/mosquitto/config:/mosquitto/config
            - ./BaseDatos/mosquitto/data:/mosquitto/data
            - ./BaseDatos/mosquitto/log:/mosquitto/log
    
```

* image: Se elige una imagen de [Docker Hub](https://hub.docker.com/_/eclipse-mosquitto) con el tag 2.0.10 y se verifica que esté disponible para la arquitectura deseada que en este caso es AMD y ARMv7. 

* restart: restart: always

Esto permite que funcione cada vez que se prende el dispositivo que lo contiene o cada vez que tenga un error.

* Ports: Se mapea el puerto interno del docker :1883 al puerto externo del dispositivo 1883:.

* Volumes: el único que en realidad es importante es la configuración que hay en el archivo mosquitto.config y que se coloca en un archivo local en el mapeo <archivo local>:/mosquitto/config para que cada vez que se reinicialice el docker no se pierda la información. El contenido de dicho archivo es:

```
listener 1883 0.0.0.0
allow_anonymous true
```
Recomentaciones:
- Debe recordarse que toda información que se coloque en un docker se va a eliminar y por ende se deben colocar volúmenes para que esta sea persistente. Un error que se tuvo fue el pensar que si se configuraba el archivo desde el interior del contenedor el cambio en el volumen se iba a ver de manera local pero en realidad sucede lo contrario.

#### InfluxDB

Se escoge una base de datos de serie temporal dada la naturaleza de nuestro proyecto.

influxdb:
        image: influxdb:1.8
        restart: always
        ports: 
            - 8086:8086
        volumes: 
            - ./BaseDatos/influxdb_data:/var/lib/influxdb
        environment: 
            - INFLUXDB_DB=embebidos
            - INFLUXDB_ADMIN_USER=admin
            - INFLUXDB_ADMIN_PASSWORD=admin
            - INFLUXDB_HTTP_AUTH_ENABLED=true

1. image: influxdb:1.8 se elige esta con el tag en específico y se verifica en [Docker Hub](https://hub.docker.com/_/influxdb) la compatibilidad de arquitecuras. Se eligió otro tag con el objetivo de usar uno más actualizado pero se encontró que no se podía acceder a ella con el método que decía Sanchez y que se expone a continuación:

Para acceder al interior del contenedor se hace así:
```
sudo docker exec -it <id/nombre contenedor InfluxDB> bash
```
Registrarse con el usuario y contraseñas

```
influx -username <usuario> -password <contraseña>
```

Observar las bases de datos creadas
```
show databases
```
Usar una base de datos:

```
use <base de datos>
```
Mirar las distintas 'tablas' creadas:
```
show measurements
```

Observar todos los resultados de la base de BaseDatos
```
select * from <tabla>

```
Esto fue de ayuda para verificar que se estaban guardando los datos de manera correcta cuando no se tenía una interfaz gráfica. 

2. Se mapeo el puerto :8086 del contenedor al puerto 8086: del dispositivo local.

3. Se guarda la información de la base de datos dentro de la ruta /var/lib/influxdb que está en el contenedor a la ruta local ./BaseDatos/influxdb_data.

4. Se expecifica por medio de variables de entorno propios de esta imagen que la base de datos se llama *embebidos* (INFLUXDB_DB=embebidos), se coloca como usuario y contraseña para *admin* (INFLUXDB_ADMIN_USER=admin, INFLUXDB_ADMIN_PASSWORD=admin) y se habilita la comunicación HTTP (INFLUXDB_HTTP_AUTH_ENABLED=true).

#### telegraf
Con este servicio se busca enviar toda la información publicada en un tópico registrado a la base de datos influxdB
```
telegraf: 
        image: telegraf:1.18.3
        restart: always
        volumes:
            - ./BaseDatos/telegraf/telegraf.conf:/etc/telegraf/telegraf.conf
        depends_on: 
            - influxdb
    
```
1. image: Se busca en [docker hub](https://hub.docker.com/_/telegraf) la imagen compatible con las arquitecturas.

2. volumes:
            - ./BaseDatos/telegraf/telegraf.conf:/etc/telegraf/telegraf.conf

El archivo de la ruta local ./BaseDatos/telegraf/telegraf.conf se vincula con el archivo dentre del contenedor /etc/telegraf/telegraf.conf y este es su contenido fundamental (lo que no significa la falta de la demás configuración):

```
[[outputs.influxdb]]
urls = ["http://influxdb:8086"]
database = "embebidos"
skip_database_creation = true
username = "admin"
password = "admin"
```
* La ip de la base de datos y el puerto está representado por http://influxdb:8086.
* Se especifica que la base de datos que se va a usar es *embebidos* que debe coincidir con la creada en InfluxDB
* No se crea nueva base de datos de datos.
* Se le coloca el mismo usuario y contraseña usados en InfluxDB.

```
[[inputs.mqtt_consumer]]
 servers = ["tcp://mosquitto:1883"]
 topics = ["Embebidos/#"]
     name_override = "embebidos"

    json_string_fields = [
      "predictions_0_probability",
      "predictions_1_probability",
      "predictions_2_probability",
      "predictions_3_probability",
      "predictions_4_probability"
      ]
      
    tag_keys = [
      "predictions_0_label",
      "predictions_1_label",
      "predictions_2_label",
      "predictions_3_label",
      "predictions_4_label"
      ]
    data_format = "json"
```

* Se especifica el protocolo de comunicación que este caso es TCP, la dirección ip *mosquitto* y el puerto en que se va a estar publicando información que se debe guardar en influxDB (*1883*).

* Se va a guardar la información que se publique en el tópico *Embebidos/#*
* Se especifica el nombre de la "tabla" en el que se va a guardar la información (embebidos).
* Dada la estructura del json que se expuso anteriormente se escoge la información que se va a guardar que en este caso es el label y la probabilidad de ocurrencia asociada con dicho label.
* Finalmente se especifica que se va a guardar como un json (Se hace distinto que Sanches [Sanches].

Este último tema fue el que más causó dificultades dado que solo se estaban guardando la probabilidades más no los labels asociados. Para esto se buscó en la documentación de telegraf con respecto a los [tipos de datos json](https://github.com/influxdata/telegraf/tree/master/plugins/parsers/json). Se notó que telegraf solo guarda datos que tengan la estructura configurada, esto generó un problema de comprobación en donde no se guardaban los datos y el error era que se debía publicar un json con la estructura que se había establecido.

3.  depends_on: 
            - influxdb

Se espera hasta que el servicio con nombre influxdb se haya creado para este hacerlo. Esto ayuda a que la base de datos de la configuración ya se haya creado.

#### grafana

Para realizar gráficos con respecto a los datos que se tengan en la base de datos InfluxDB se usa Grafana. 
```
grafana: 
        image: grafana/grafana:7.5.7 
        restart: always
        ports:
            - 3000:3000 
        volumes:
            - grafana-storage:/var/lib/grafana  
        depends_on: 
            - influxdb
    
```

1. image: grafana/grafana:7.5.7

Se busca una imagen compatible con las arquitecturas que se tienen en [Docker Hub](https://hub.docker.com/r/grafana/grafana)

2. Se mapea el puerto :3000 del contenedor al puerto 3000: del dispositivo local.

3. volumes:
    - grafana-storage:/var/lib/grafana  

Se guarda de manera local en el volumen *grafana-storage* la configuración hecha en el contenedor para graficar los datos que se localiza en su ruta /var/lib/grafana.

4. volumes:
    grafana-storage:
        external: false

Se tuvo que crear un volumen *especial* para grafana porque fue la única forma que se encontró para que no se borraran los cambios editados en su interfaz después de varios intentos mediante la forma convencional con la que se venía haciendo.

#### ser_brokerconection

Este fue el servicio que se creo de manera local y cuya estructura varía un poco con respecto a las que se se expusieron.
```
    ser_brokerconection:
        build: ./BrokerMqttPython/BrokerConection
        restart: always
        depends_on:
            - mosquitto
```
1. build: ./BrokerMqttPython/BrokerConection

En la dirección relativa *./BrokerMqttPython/BrokerConection* al archivo docker-compose.yml se tiene una archivo Dockerfile que crea una imagen con la siguiente descripción:

```
FROM python:3.7.3

WORKDIR /usr/src/app

COPY requirements.txt ./
RUN pip install --no-cache-dir -r requirements.txt
RUN apt update && apt install -y curl && apt install -y avahi-daemon

COPY . .

CMD ["python3", "./app.py" ]
```
a.  FROM python:3.7.3

Se descarga de Docker Hub la imagen python:3.7.3 que corresponde a la misma versión con el que se comprobó el correcto funcionamiento del script *app.py* en la raspberry.

b. Establece el directorio de trabajo como /usr/src/app según la documentación de [Docker Hub](https://hub.docker.com/_/python).

c. COPY requirements.txt ./

Copia el archivo requirements.txt que contiene las librería necesarias para el script *app.py* en el directorio de trabajo dentro del contedor.

d. RUN pip install --no-cache-dir -r requirements.txt

Se instalan las librerías que están en requirements.txt.

e. RUN apt update && apt install -y curl && apt install -y avahi-daemon

Se instala curl y avahi-daemon necesarios para el funcionamiento *app.py* (avahi-daemon no reconoció la ip de un dispositivo externo al que contenía el docker).

f. COPY . .

Se copia todo lo que hay en el directorio actual dentro del directorio de trabajo del Docker (excepto lo que está en .dockerignore).

e. CMD ["python3", "./app.py" ]

Se ejecuta dentro del contenedor el comando ```python3 app.py``` por defecto.

2. Se reinicia siempre y se crea después del servicio moquitto.

#### docker-compose final 

Uniendo todos los servicios en un docker-compose.yml se tiene el siquiente archivo:

```
version: '3.1'

services:
    mosquitto:
        image: eclipse-mosquitto:2.0.10
        restart: always
        ports: 
            - 1883:1883  
        volumes:
            - ./BaseDatos/mosquitto/config:/mosquitto/config
            - ./BaseDatos/mosquitto/data:/mosquitto/data
            - ./BaseDatos/mosquitto/log:/mosquitto/log
    
    telegraf: 
        image: telegraf:1.18.3
        restart: always
        volumes:
            - ./BaseDatos/telegraf/telegraf.conf:/etc/telegraf/telegraf.conf
        depends_on: 
            - influxdb
    

    
    influxdb:
        image: influxdb:1.8
        restart: always
        ports: 
            - 8086:8086
        volumes: 
            - ./BaseDatos/influxdb_data:/var/lib/influxdb
        environment: 
            - INFLUXDB_DB=embebidos
            - INFLUXDB_ADMIN_USER=admin
            - INFLUXDB_ADMIN_PASSWORD=admin
            - INFLUXDB_HTTP_AUTH_ENABLED=true
            
    grafana: 
        image: grafana/grafana:7.5.7 
        restart: always
        ports:
            - 3000:3000 
        volumes:
            - grafana-storage:/var/lib/grafana  
        depends_on: 
            - influxdb
           
    ser_brokerconection:
        build: ./BrokerMqttPython/BrokerConection
        restart: always
        depends_on:
            - mosquitto


volumes:
    grafana-storage:
        external: false
```



### FRONT END
Para el front end(Interfaz hombre maquina) se realizo una pagina web que hace uso de lenguaje HTML, JAVASCRIPT, NODE.js, CSS y PYTHON para su funcionamiento como diseño. Particularmente NODE.js y PYTHON se uso para parte del backend que principalmente es la lectura de la base de datos INFLUXDB. Para hacer una descripcion de cada partede la pagina clara y consisa dividio la pagina en "Encabezado" y "Main".

### ENCABEZADO
Para el encabezado se siguio la siguiente maqueta de HTML.
```bash
nav class="navbar">
        <div class="navbar_container">
            <a href="/" class="" id="navbar_logo"><i class="fab fa-pagelines"></i>Resafeve</a>
        </div>
        <div class="navbar_containerlinks">
            <ul class="navbar_menu">
                <li class="navbar_items">
                    <a href="https://github.com/esteban11519/SistemasEmbebidos" class="navbar_links">Documentacion</a>
                </li>
                <li class="navbar_items">
                    <a href="/" class="navbar_links">Equipo</a>
                </li>
                <li class="navbar_button">
                    <a href="http://192.168.1.86:3000/d/MZVyWBmRk/embebidos?from=1629944777057&to=1630549577057&orgId=1" class="button">Monitorear</a>
                </li>
            </ul>
        </div>
    </nav>
```

En el anterior codigo vemos varios elementos <a> que hacen referencia a los "Links" a otras paginas. Para este caso tenemos los siguientes botones. 
  
![Screenshot from 2021-09-02 09-09-01](https://user-images.githubusercontent.com/55359032/131862028-b4d13deb-9292-4983-9d54-55efb4f82829.png)
        
Que como vemos en la maqueta de HTML al presionar "Documentacion" nos redirecciona al github del proyecto mientras que al presionar Monitorear nos direcciona a Grafana a un Dashboard. Ademas de estos elementos tenemos el titulos de la pagina que tiene un estilo particular ya que se le agrego un degrade en la hoja de estilos. Ademas para el "icono" se uso un script prestado por la pagina https://fontawesome.com/, donde al registrarse y poner la etiqueta del script que nos dan para nuestra cuenta ya con solo hacer referencia a la clase "fab fa-pagelines" de la imagen esta aparece en el elemento deseado.
       
![Screenshot from 2021-09-02 09-33-40](https://user-images.githubusercontent.com/55359032/131862928-ac8917b4-a6de-4273-8974-699d427a4d28.png)
 
```bash    
        
 #navbar_logo {
    background-color: #0a8124;
    background-image: linear-gradient(to bottom, #19e946 0%, #00a524 100%);
    background-size: 100%;
    -webkit-background-clip: text;
    -moz-background-clip: text;
    -webkit-text-fill-color: transparent;
    -moz-text-fill-color: transparent;
    display: flex;
    cursor: pointer;
    text-decoration: none;
    font-size: 2rem;
} 
        
```
Hay algunos otros elementos y utilidades de CSS que son interensantes, como el HOVER que nos permite interactuar con los elementos, como ponerlos opacos o cambiarlos de color. Esto hace la experiencia de usuario mucho mas amena.
        
### MAIN
        
Para el main se uso un contenedor que cotiene una imagen con una frase y adicionalmente tiene algunos widgets para la visualizacion de la informacion obtenida a su vez con la base de datos. Para el primer widget se uso programó un reloj con fecha mediate el usod de variables propias de javascript Date() y mediante a una iteracion se actuliza la fecha y la hora por cada intervalo de tiempo. Adicionalmente se programo un indicador de alamrma basado en la ultima lectura de la base de datos y algunos criterios que se especificaran posteriormente. Este widget cambiara su color y su texto dependiendo del dato almacenado en por el analisis de la inteligencia artificial.

 ![Screenshot from 2021-09-02 09-44-24](https://user-images.githubusercontent.com/55359032/131864814-60d85911-37fd-4eb9-9a98-00abbdd86d8e.png)
        
 Ahora este Widget resume todo el funcionamiento principal del backend. Primero tenemos una aplicacion de Python la cual cuenta con la libreria mas importante para la lectura de datos desde la base de datos influxDB (INFLUXDB). Esta libreria nos permite crear un cliente y  conectarnos a la base de datos, para posteriormente usar la funcion .query para hacer solicitudes tipo "SQL". 

```bash 
        client = influxdb.InfluxDBClient(host='192.168.1.86', port=8086, ....)
        Lecturas_2 = client.query(f"SELECT * FROM embebidos WHERE time > now()-14d")
```
De esta manera se crea un archivo json que posteriomente nos permite usar Node.js junto con algunos paquetes como  EXPRESS, CORS y FS. Estas librerias primero como EXPRESS sirven para montar servicios para hacer solicitudes APIS , CORS soluciona un problema de comunicacion entre HTTPS y HTTP. Finalmente FS nos ayuda a leer los json. Finalmente con ayuda de FETCH se hace la solicitud desde la pagina web y mediante el metodo .then() podemos enviar los resultados retornados por FETCH a alguna funcion para de esta manera analizar y modificar el widget en cuestion.
 
```bash 
     fetch('http://localhost:1500')
    .then(response => response.json())
    .then(data => printIt(data));
```
 

## Bibliografia
Recursos adicionales
* MQTT - Resumen de todo con explicaciones y links de temas relacionados: https://aprendiendoarduino.wordpress.com/2018/11/19/mqtt/ 
* Esp32 Mqtt explicado: https://randomnerdtutorials.com/esp32-mqtt-publish-subscribe-arduino-ide/
* Inteligencia Artificial que clasifica los sonidos: https://www.google.com/url?q=https://github.com/IBM/MAX-Audio-Classifier&sa=D&source=editors&ust=1630442220276000&usg=AOvVaw06XKVdFrBWjtAnfdxmKH8F 
* Documentacion Espressif del ESP32: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/index.html
* Datasheet del Microfono INMP441: https://www.google.com/url?q=https://invensense.tdk.com/wp-content/uploads/2015/02/INMP441.pdf&sa=D&source=editors&ust=1630447016673000&usg=AOvVaw1flXUa5FAnurC2niqlp07R
*  Documentacion SPI y Icestudio: https://github.com/Obijuan/Cuadernos-tecnicos-FPGAs-libres/wiki/CT.5:-SPI-esclavo


* [Sanches] J. J. Sánchez Hernández, «IoT Dashboard - Sensores, MQTT, Telegraf, InfluxDB y Grafana». may 20, 2021. [En línea]. Disponible en: http://josejuansanchez.org/iot-dashboard/


* «Docker Hub». https://hub.docker.com/ (accedido sep. 02, 2021).
Pelado Nerd, DOCKER 2021 - De NOVATO a PRO! (CURSO COMPLETO), (2021). Accedido: sep. 02, 2021. [En línea Video]. Disponible en: https://www.youtube.com/watch?v=CV_Uf3Dq-EU
