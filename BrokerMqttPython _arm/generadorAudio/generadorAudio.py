import os
import time

tiempoIntercambio=10
archivoProcesar='samples/audio.wav'
archivoIntercambio='samples/audio1.wav'
auxArchivo='samples/audioaa.wav'

time.sleep(tiempoIntercambio)

while True:
    
    os.rename(archivoProcesar, auxArchivo) # cambia de nombre a archivoProcesar    
    os.rename(archivoIntercambio,archivoProcesar) # archivoIntercambio toma el nombre de archivoProcesar
    os.rename(auxArchivo,archivoIntercambio) # archivoIntercambio toma el nombre de archivoProcesar
    time.sleep(tiempoIntercambio)


