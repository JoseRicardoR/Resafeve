# Para ejecutarlo: sudo docker build -t brokerconection:1.0 .
# Para instalar el docker: sudo docker run brokerconection:1.0
FROM python:3.7.3

WORKDIR /usr/src/app

COPY requirements.txt ./
RUN pip install --no-cache-dir -r requirements.txt
RUN mkdir ejemplos

COPY ./app.py ./

CMD ["python3", "./app.py" ]

