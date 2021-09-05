FROM nginx:1.21.1

WORKDIR /usr/share/nginx/html
RUN apt update && apt install -y python3
COPY requirements.txt ./
RUN pip install --no-cache-dir -r requirements.txt


COPY . .

CMD ["python3", "./app.py" ]
