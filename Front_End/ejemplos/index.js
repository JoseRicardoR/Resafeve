const express = require("express");
const app = express();
const fs = require('fs');
const cors = require('cors');
const PORT = 8080;
const HOST = '0.0.0.0';

app.use(cors())

app.listen(PORT, HOST); //Para docker
/*app.listen(1500, () => {
    console.log(`Server is up and running on 5000 ...`);
});
*/

app.get("/", (req, res) => {
    let rawdata = fs.readFileSync('ult_lect.json');
    let ult_lect = JSON.parse(rawdata);
    res.send(ult_lect);
});
