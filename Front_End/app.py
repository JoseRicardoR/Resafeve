import influxdb
import json
from datetime import datetime
import os
import time

tiempoEspera=1
Host='192.168.1.86'

client = influxdb.InfluxDBClient(host=Host, port=8086, username="admin", password="admin",database="embebidos")
mem = "a";

while True:
    client = influxdb.InfluxDBClient(host=Host, port=8086, username="admin", password="admin",database="embebidos")
    
    Lecturas_1 = client.query("SELECT * FROM embebidos")
    Lecturas_1 = list(Lecturas_1) 

    Lecturas_2 = client.query(f"SELECT * FROM embebidos WHERE time > now()-14d")
    Lecturas_2 = list(Lecturas_2)

    time_vector = []
    vector = []

    for dic in Lecturas_2[0]:
        date_time_obj = datetime.strptime(dic['time'], '%Y-%m-%dT%H:%M:%S.%fZ')
        time_vector.append(date_time_obj)
        vector.append(dic['predictions_0_probability'])


        

    tamano = len(Lecturas_1[0])
    mi_diccionario = Lecturas_1[0][tamano-1]
    app_json = json.dumps(mi_diccionario)


    if(mi_diccionario['time'] != mem):
        mem = mi_diccionario['time']
        with open('./ejemplos/ult_lect.json', 'w') as f:
            f.write(app_json)
            f.close()
    print(app_json)
    print("funciona")
    time.sleep(tiempoEspera)

