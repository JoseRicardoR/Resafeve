# Para ejecutarlo: sudo docker build -t myapp:1.0.0 .
FROM python:3.6.9
ENV FLASK_APP run.py


COPY run.py gunicorn-cfg.py requirements.txt ./
COPY . .

RUN pip3 install -r requirements.txt


#Para correr dentro del docker: gunicorn --bind app:5000 --config gunicorn-cfg.py run:app
# app es la ip que se crea en el docker-composite de la aplicación
# Gracias a Dios eso se identificó
CMD ["gunicorn","--bind","ser_app:5000", "--config", "gunicorn-cfg.py", "run:app","&"]
#CMD [ "python3", "-m" , "flask", "run", "--host=0.0.0.0"]
#python3 flask -m flask run --host=0.0.0.0




