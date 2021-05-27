# Para ejecutarlo: sudo docker build -t testdb:2.0 .
FROM python:3.6.9
ENV FLASK_APP run.py


#gunicorn-cfg

COPY run.py gunicorn-cfg.py requirements.txt ./
COPY . .

RUN pip3 install -r requirements.txt
RUN apt update
RUN apt-get install nano



#Para correr dentro del docker gunicorn --bind app:5000 --config gunicorn-cfg.py run:app
# app es la ip que se crea en el docker-composite de la aplicación
# Gracias a Dios eso se identificó
CMD ["gunicorn","--bind","app:5000", "--config", "gunicorn-cfg.py", "run:app"]



