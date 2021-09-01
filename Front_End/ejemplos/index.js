const express = require("express");
const app = express();
const fs = require('fs');
const Influx = require('influx');
const cors = require('cors');


app.use(cors())

app.listen(5000, () => {
    console.log(`Server is up and running on 5000 ...`);
});

app.get("/", (req, res) => {
    const client = new Influx.InfluxDB({
        host: '192.186.1.86',
        port: '8086',
        database: 'embebidos',
        username: 'admin',
        password: 'admin'
    })

    let rawdata = fs.readFileSync('ult_lect.json');
    let ult_lect = JSON.parse(rawdata);
    res.send(ult_lect);
});