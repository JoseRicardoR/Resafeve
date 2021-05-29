#importar la libreria principal (flask alchemy)
from flask import Flask, redirect, url_for, request 
from flask_sqlalchemy import SQLAlchemy

import json
import pymysql

pymysql.install_as_MySQLdb()


# Se crea la aplicacion

app = Flask(__name__)
# Permitir encriptamiento desde la app, se configura una clave principal
app.secret_key = 'dasfkn:LVnaWOGVN;Lk DLILHS [Q'

# Establece la cadena de conexion a la base de datos
#app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
# De forma local descomentar:
#app.config['SQLALCHEMY_DATABASE_URI'] = 'mysql://root:root@localhost:3307/Embebidos'


#En el docker descomentar:

# El 'ser_mariadb' es el alias de la ip de la base de datos que se crea por defecto en el docker-composite
# Gracias a Dios se pudo llegar a ese razonamiento
# Estructura: 'mysql://usuario:contraseña@ip_docker_base_datos/NombreDeBaseDeDatos'
app.config['SQLALCHEMY_DATABASE_URI'] = 'mysql://root:root@ser_mariadb/Embebidos'

#De manera local:
#app.config['SQLALCHEMY_DATABASE_URI'] = 'mysql://root:ELFMar#20@localhost/Embebidos3'

# realizamos la conexion
db = SQLAlchemy(app)

# Se crea un modelo de la tabla donde esta la informacion (Modelo ORM)
class Lectura(db.Model):
    # defino como se llama la tabla en la base de datos
    __tablename__ = 'Lecturas'
    id = db.Column('id', db.Integer, primary_key = True)
    topico = db.Column(db.String(100))
    payload = db.Column(db.String(255))

    def __init__(self, topico, payload):
        self.topico = topico
        self.payload = payload

@app.route('/Lecturas')
def get_lecturas():
    lecturas = Lectura.query.all()
    print(lecturas)
    respuesta = {}
    for lectura in lecturas:
        print(lectura)
        data={
            'id': lectura.id,
            'topico': lectura.topico,
            'payload': lectura.payload
            
        }
        respuesta[lectura.id]=data
        #return 'ok'
    return json.dumps(respuesta, ensure_ascii=False).encode('utf-8')
# Filtra por id
@app.route('/LecturasById/<valor>')
def get_lecturas_by_id(valor):
    lecturas = Lectura.query.filter_by(id=valor).all()
    respuesta = []
    for lectura in lecturas:
        data={
            'id': lectura.id,
            'topico': lectura.topico,
            'payload': lectura.payload
            
        }
        respuesta.append(data)
        #return 'ok'
    return json.dumps(respuesta, ensure_ascii=False).encode('utf-8')

@app.route('/LecturasByTopico/<valor>')
def get_lecturas_by_topico(valor):
    #Model.query.filter(Model.columnName.contains('sub_string'))
    lecturas = Lectura.query.filter(Lectura.topico.contains(valor)).all()
    respuesta = []
    for lectura in lecturas:
        data={
            'id': lectura.id,
            'topico': lectura.topico,
            'payload': lectura.payload
            
        }
        respuesta.append(data)
        #return 'ok'
    return json.dumps(respuesta, ensure_ascii=False).encode('utf-8')

@app.route('/')
def index():
    return 'Hello World'