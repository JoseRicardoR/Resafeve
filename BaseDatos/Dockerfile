FROM python:3.6
ENV FLASK_APP run.py


#gunicorn-cfg

COPY run.py requirements.txt ./
COPY . .

RUN pip3 install -r requirements.txt

CMD ["python3","-m","flask","run","--host=0.0.0.0"] 
