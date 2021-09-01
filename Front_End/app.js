(function() {
    var actualizarHora = function() {
        var fecha = new Date(),
            horas = fecha.getHours(),
            minutos = fecha.getMinutes(),
            segundos = fecha.getSeconds(),
            dia = fecha.getDate(),
            mes = fecha.getMonth(),
            year = fecha.getFullYear(),
            pHoras = document.getElementById('hora'),
            pMinutos = document.getElementById('minutos'),
            pSegundos = document.getElementById('segundos'),
            pDia = document.getElementById('dia'),
            pMes = document.getElementById('mes'),
            pYear = document.getElementById('years');

        pHoras.textContent = horas;
        pMinutos.textContent = minutos;
        pSegundos.textContent = segundos;


        meses = ['Enero', 'Febrero', 'Marzo', 'Abril', 'Junio', 'Julio', 'Agosto', 'Septiembre', 'Octubre', 'Noviembre', 'Diciembre'];
        pMes.textContent = meses[mes - 1];
        pYear.textContent = year;
        pDia.textContent = dia;


    };

    actualizarHora();
    setInterval(actualizarHora, 1000);
}())