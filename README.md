# Sistemas Embebidos
Proyecto final de Sistemas embebidos: Resafeve

Mateo Bernal Bonil - Valentine Chimezie Muodum Prieto - Esteban Ladino Fajardo - Jose Ricardo Rincon Pelaez


Se quiere hacer un sistema de monitoreo en reservas naturales para reservas naturales, en el cual se instalan microfonos en arboles para el regustro de audio y su procesamiento con IA en un servidor aparte. Los micrfonos se controlan con ESP32, la comunicación se plantea por medio de MQTT al servidor Mosquitto situado en una Raspberry, conectada tambien a un servidor online.
![Esquema general](https://i1.wp.com/randomnerdtutorials.com/wp-content/uploads/2018/05/ESP32-MQTT-bme280.png?w=800&quality=100&strip=all&ssl=1)

Luego la inteligencia artificial usada se encuentra en: https://github.com/IBM/MAX-Audio-Classifier 


## MQTT


## Funcionalidad de la ESP32


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
