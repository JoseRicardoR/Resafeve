var resp = 1;

resp = fetch('http://localhost:5000')
    .then(response => response.json())
    .then(data => printIt(data));

let printIt = (data) => {
    if (data.predictions_0_label == "Chainsaw" && data.predictions_0_probability >= 0.5) {
        elemt = document.getElementById("main_container_1");
        elemt_1 = document.getElementById("kk");
        if (elemt.style.backgroundColor != "red") {
            elemt.style.backgroundColor = "red";
            elemt_1.textContent = "Estado de Alarma: Activado";
        }
    }
}