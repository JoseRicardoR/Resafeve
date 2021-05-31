[IoT Dashboard - Sensores, MQTT, Telegraf, InfluxDB y Grafana](http://josejuansanchez.org/iot-dashboard/)

Comandos para entrar a la base de datos y observar lo que se ha publicado
sudo docker exec -it 8b043de730ec bash
influx -username root -password root
show databases*
use esteban_db
show measurements
select * from mqtt_consumer


