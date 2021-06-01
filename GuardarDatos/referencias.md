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
