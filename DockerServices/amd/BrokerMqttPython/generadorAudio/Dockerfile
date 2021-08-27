# Para ejecutarlo: sudo docker build -t intercambiaraudio:1.0 Dockerfile_generador_audio
# Para instalar el docker: sudo docker run intercambiaraudio:1.0
FROM python:3.8.10

WORKDIR /usr/src/app

COPY . .
RUN mkdir samples


CMD [ "python3", "./generadorAudio.py" ]