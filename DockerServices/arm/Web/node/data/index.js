const express = require("express");
const app = express();
const fs = require('fs');
const Influx = require('influx');
const cors = require('cors');
const PORT = 8080;
const HOST = '0.0.0.0';

app.use(cors())

app.listen(PORT, HOST);

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