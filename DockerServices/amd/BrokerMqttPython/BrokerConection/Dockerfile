# Para ejecutarlo: sudo docker build -t brokerconection:1.0 .
# Para instalar el docker: sudo docker run brokerconection:1.0
FROM python:3.8.10
#FROM python:3.7.3

WORKDIR /usr/src/app

COPY requirements.txt ./
#RUN ["/bin/sh ", "-c", "pip install --no-cache-dir -r requirements.txt"]
#RUN apt update
RUN pip install --no-cache-dir -r requirements.txt
#RUN apt update -y && apt install -y avahi-daemon

COPY . .

CMD [ "python3", "./BrokerConection.py" ]