const express = require("express");
const app = express();
const fs = require('fs');
const cors = require('cors');


app.use(cors())

app.listen(1500, () => {
    console.log(`Server is up and running on 5000 ...`);
});


app.get("/", (req, res) => {
    let rawdata = fs.readFileSync('ult_lect.json');
    let ult_lect = JSON.parse(rawdata);
    res.send(ult_lect);
});