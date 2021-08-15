# Para ejecutarlo: sudo docker build -t brokerconection:1.0 .
# Para instalar el docker: sudo docker run brokerconection:1.0
FROM python:3.8.10

WORKDIR /usr/src/app

COPY requirements.txt ./
RUN pip install --no-cache-dir -r requirements.txt
RUN mkdir samples

COPY . .

CMD [ "python3", "./BrokerConection.py" ]