[¿Cómo obtener la dirección IP de un contenedor Docker desde el host?](https://www.it-swarm-es.com/es/docker/como-obtener-la-direccion-ip-de-un-contenedor-docker-desde-el-host/1073314901/)

docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' container_name_or_id

[DOCKER 2021 - De NOVATO a PRO! (CURSO COMPLETO)](https://www.youtube.com/watch?v=CV_Uf3Dq-EU)
