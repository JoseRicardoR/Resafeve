[IoT Dashboard - Sensores, MQTT, Telegraf, InfluxDB y Grafana](http://josejuansanchez.org/iot-dashboard/)

Comandos para entrar a la base de datos y observar lo que se ha publicado
sudo docker exec -it 8b043de730ec bash
influx -username root -password root
show databases*
use esteban_db
show measurements
select * from mqtt_consumer


Para solucionar el error de los datos persistentes: https://stackoverflow.com/questions/37705017/create-named-docker-volume-with-docker-compose

Aprender a manejar Grafana:https://www.youtube.com/watch?v=GLE71pIHUU8


Poner variables de entorno a los servicios: https://github.com/docker/compose/issues/3079

Variables de entorno en Docker Compose


Problemas de reconocimiento de root: https://github.com/influxdata/influxdb/issues/8157

create user admin with password 'admin' with all privileges

create user root with password 'root' with all privileges

Error: No se conecta la base de datos a mosquitto:

Solución: No colocar las redes con las que se hace el docker-compose sino la de la Raspberry
https://dzone.com/articles/raspberry-pi-iot-sensors-influxdb-mqtt-and-grafana


No esa no era la solución: Gracias a Dios la solución era enviar bien la información o en el formato establecido en telegraf (Yo establecí un json):
mosquitto_pub -t Esteban/Hoooo -m "{\"numero\":10}"

Nota: Colocar con lowercase las bases de datos

Las variables de entorno no son muy buenas en arm o almenos como las tengo yo declaradas


Guardar string en la base de datos

https://github.com/influxdata/telegraf/issues/7536