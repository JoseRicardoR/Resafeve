# Sistemas Embebidos
Proyecto final de Sistemas embebidos



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