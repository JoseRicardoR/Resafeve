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
## Referencias

[Sanches] J. J. Sánchez Hernández, «IoT Dashboard - Sensores, MQTT, Telegraf, InfluxDB y Grafana». may 20, 2021. [En línea]. Disponible en: http://josejuansanchez.org/iot-dashboard/


«Docker Hub». https://hub.docker.com/ (accedido sep. 02, 2021).
Pelado Nerd, DOCKER 2021 - De NOVATO a PRO! (CURSO COMPLETO), (2021). Accedido: sep. 02, 2021. [En línea Video]. Disponible en: https://www.youtube.com/watch?v=CV_Uf3Dq-EU

