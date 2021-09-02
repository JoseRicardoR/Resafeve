import influxdb
from requests.models import PreparedRequest
import pandas as pd
import json

client = influxdb.InfluxDBClient(host='192.168.1.86', port=8086, username="admin", password="admin",database="embebidos")
Lecturas = client.query("SELECT * FROM embebidos")
Lecturas = list(Lecturas) 

tamano = len(Lecturas[0])
mi_diccionario = Lecturas[0][tamano-1]
app_json = json.dumps(mi_diccionario)

with open('ult_lect.json', 'w') as f:
    f.write(app_json)
    f.close()



