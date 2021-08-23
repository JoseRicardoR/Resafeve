import os

# Clonar el repositorio: Tomado de https://github.com/IBM/MAX-Audio-Classifier
os.system('git clone https://github.com/IBM/MAX-Audio-Classifier.git')

# Instalar el docker-compose
os.system('docker-compose up -d')
