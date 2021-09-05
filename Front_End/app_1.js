var resp = 1;

resp = fetch('http://resafeve.hopto.org:8082') 
    .then(response => response.json())
    .then(data => printIt(data));

let printIt = (data) => {
    
    elemt_2 = document.getElementById("men11");
    elemt_3 = document.getElementById("men12");
    elemt_4 = document.getElementById("men21");
    elemt_5 = document.getElementById("men22");
    elemt_6 = document.getElementById("men31");
    elemt_7 = document.getElementById("men32");
    elemt_8 = document.getElementById("men41");
    elemt_9 = document.getElementById("men42");
    elemt_10 = document.getElementById("men51");
    elemt_11 = document.getElementById("men52");
    

    elemt_2.textContent = data.predictions_0_label;
    elemt_3.textContent = parseFloat(data.predictions_0_probability).toFixed(2);

    elemt_4.textContent = data.predictions_1_label;
    elemt_5.textContent = parseFloat(data.predictions_1_probability).toFixed(2);

    elemt_6.textContent = data.predictions_2_label;
    elemt_7.textContent = parseFloat(data.predictions_2_probability).toFixed(2);

    elemt_8.textContent = data.predictions_3_label;
    elemt_9.textContent = parseFloat(data.predictions_3_probability).toFixed(2);

    elemt_10.textContent = data.predictions_4_label;
    elemt_11.textContent = parseFloat(data.predictions_4_probability).toFixed(2);


    if ((data.predictions_0_label == "Chainsaw" || data.predictions_0_label == "Engine"||data.predictions_0_label == "Medium engine (mid frequency)" ||data.predictions_0_label == "Vehicle" ||data.predictions_0_label == "Pump (liquid)" ||data.predictions_0_label == "Motorcycle" ||data.predictions_0_label == "Electric toothbrush")&&(data.predictions_0_probability >= 0.1)) {
        elemt = document.getElementById("main_container_1");
        elemt_1 = document.getElementById("kk");
        if (elemt.style.backgroundColor != "red") {
            elemt.style.backgroundColor = "red";
            elemt_1.textContent = "Estado de Alarma: Activado";
        }
    }
    else{
        elemt = document.getElementById("main_container_1");
        elemt_1 = document.getElementById("kk");
        elemt.style.backgroundColor = "#00b627bd";
        elemt_1.textContent = "Estado de Alarma: Apagado";
    }
}
