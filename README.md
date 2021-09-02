# Sistemas Embebidos
Proyecto final de Sistemas embebidos: Resafeve

Mateo Bernal Bonil - Valentine Chimezie Muodum Prieto - Esteban Ladino Fajardo - Jose Ricardo Rincón Peláez


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

### PubSubClient
Para la comunicación se usa el protocolo de MQTT, que en el caso de la ESP32 se conecta con la librería PubSubClient, y para los demás elementos se usa la líbreria de python paho-mqtt. MQTT usa un sistema de subscripciones para la publiación de mensajes a topicos especificos en el servidor.

Luego, una primera pregunta se plantea ¿Cuantos segundos puede grabar el ESP32?¿De que tamaño es el Packete que se enviara?
De acuerdo a la documentación de pubsubclient el paquete máximo que se puede enviar son 256 bytes con la configuración. Es decir, hay un limite en el tamaño de paqute que se puede enviar, más en la propia documentación se explica que con la funcion client.setBufferSize(MQTT_MAX_PACKET_SIZE); 
Luego la duda es por el lado de que tamaño es limite máximo que podemos tener. La cuestion es que entre más grande el paqute más se depende de la conexión a internet.

### Microfono electret
En primera instancia se probo para el muestreo de audio el uso de un micrfono electret con una configuración de amplifiación ya usada en otros proyectos. Luego el ESP32 en alguno de sus pines de lectura deberia de ser capaz de muestrear la señal a la frecuencia deseada con el uso de un while y un delay. El archivo ESP32_Audio_mqtt_electret.ino en la carpeta del proyecto, se muestra el codigo en cuestion.

Cuando se implemento el codigo se encotro con el primer problema, la lectura de los pines del ESP32 no funcionaba. ¿Por qué no funciona el AnalogRead en el código?
En cuanto al error de lectura del ESP32 cuando se usa el WiFi y MQTT el problema son que pines se eligen. Tal como indica la documentación y un foro, algunos Pines (como el 13 que se estaba usando) usan registros ADC2 que son los que manejan las funciones de WiFi de forma que si el WiFi esta activado quedan inhabilitados. Los pines que no se pueden usar son:  GPIO0, GPIO2, GPIO4, GPIO12 - GPIO15, GOIO25 - GPIO27. Luego de cambiar a los pines 34 para la lectura la lectura ya era posible. 

Con el montaje inicial se desarrollo un script de python que recibia la información enviada del ESP32 y usaba multiples librerias. El proceso era:
* Se divide la información en el ESP32 en dos bytes por dato. La lectura es de 12 bits escalada a 16 bits. Pero el canal es de solo un byte de ancho. Además la inteligencia artificial pide idealmente un audio de 10s con una frecuencia de muestreo de 44100Hz y una resolución de 16 bits.
* En python se recibió esta información y con la funcion numpy.frombuffer se llega a la solución ya que retorna un arreglo unidimensional a partir de la señal de entrada https://numpy.org/doc/stable/reference/generated/numpy.frombuffer.html?highlight=frombuffer#numpy.frombuffer 
* En el archivo python se usa luego la función numpy.concatenate para crear una variable global con toda la información recibida en todos los paquetes. https://es.stackoverflow.com/questions/10768/crear-variables-globales-en-python 
* Finalmente se usan variables para contar los paquetes y datos y cuando se llega al punto deseado se usa https://docs.scipy.org/doc/scipy/reference/generated/scipy.io.wavfile.write.html#scipy.io.wavfile.write para crear el .wav

Este proceso funcionaba pero la calidad del sonido era mala y los tiempos de eran mucho más largos de lo esperado (enviar 9 segundos costaba casi minuto y medio). Una alternativa era usar las funciones beginplubish() y endpublish() de PubSubClient para que el proceso fuese más bien un stream de datos. Más sin embargo, estas dos funciones no enviaban toda la información por algún error desconocido. 

La solución inicial fue bajar la frecuencia de muestreo, de 44100Hz a 8000Hz. Antes de ello se probo en la inteligencia artificial que aún con audios de 8000Hz se obtuvieran resultados decentes. En comparación los resultados se muestran a continuación:
* 44100 Hz: {"status": "ok", "predictions": [{"label_id": "/m/01j4z9", "label": "Chainsaw", "probability": 0.6540094614028931}, {"label_id": "/m/04_sv", "label": "Motorcycle", "probability": 0.1308945119380951}, {"label_id": "/t/dd00066", "label": "Medium engine (mid frequency)", "probability": 0.12884050607681274}, {"label_id": "/m/0_ksk", "label": "Power tool", "probability": 0.11332365870475769}, {"label_id": "/m/07yv9", "label": "Vehicle", "probability": 0.10684427618980408}]}
* 8000 Hz: {"status": "ok", "predictions": [{"label_id": "/m/01j4z9", "label": "Chainsaw", "probability": 0.4117594063282013}, {"label_id": "/m/04_sv", "label": "Motorcycle", "probability": 0.206061452627182}, {"label_id": "/m/07yv9", "label": "Vehicle", "probability": 0.1337493658065796}, {"label_id": "/m/0_ksk", "label": "Power tool", "probability": 0.1071915328502655}, {"label_id": "/t/dd00066", "label": "Medium engine (mid frequency)", "probability": 0.07981646060943604}]}

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
El codigo completo se encuentra en la carpeta del proyecto con el nombre I2S_Sampling.ino.
La señal de audio de salida es en definitiva clara y sin interrupciones. Se puede además muestrear a mayores frecuencias, se elige la de 16000Hz para no saturar de datos la red y la memoria del ESP32.
<img src="https://user-images.githubusercontent.com/36318647/131574616-8b3a6e98-f21a-474e-bff3-a4ae040e81f5.PNG" width="800">

Tambien se cambio el script de python para que ahora solo se requiera la libreria de Pahoo, así pues lo que se hace es generar un header al inicio del archivo que lo hace legible como .wav. Dicho header varia en relación a la frecuencia de muestreo, el numero de bits de resolución y el largo del archivo .wav.
El archivo definitivo tiene el nombre de ESP32_audio_receiver.py, y la explicación del header se puede encontrar en: http://soundfile.sapp.org/doc/WaveFormat/.


### FreeRTOS

## Funcionalidad de la FPGA

La tarjeta FPGA usada para este proyecto es la Blackice II. Este dispositivo cuenta con la posibilidad de ser programado mediante un entorno grafico llamado
Icestudio mediante bloques que representan los modulos que normalmente se insatancian al ser usado un lenguaje de descripción de hardware convencional
(Verilog o VHDL).
![Blackice II](https://user-images.githubusercontent.com/42346359/131914705-aa9a34cd-44ce-4965-8141-9e75d155edac.jpg)

La tarea a realizar por la FPGA en este proyecto consiste en la deteccion de audios validos que puedan ser analizados por medio de una inteligencia artificial,
esto con el fin de no saturarla con muchas peticiones procedentes de varios microfonos. Para que un audio se considere como valido, un porcentaje arbitrario de las
muestras determinado por el diseñador, deberá superar 
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


### FRONT END
Para el front end(Interfaz hombre maquina) se realizo una pagina web que hace uso de lenguaje HTML, JAVASCRIPT, NODE.js, CSS y PYTHON para su funcionamiento como diseño. Particularmente NODE.js y PYTHON se uso para parte del backend que principalmente es la lectura de la base de datos INFLUXDB. Para hacer una descripcion de cada partede la pagina clara y consisa dividio la pagina en "Encabezado" y "Main".

###ENCABEZADO
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
        
###MAIN
        
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
