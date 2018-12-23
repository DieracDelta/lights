const express = require('express')
const app = express()
const PORT = 3000

///////// p2p communication //////

const P2PPORT = 4000;
const PEERID = '420694204200';
const peerLib = require('peerjs-nodejs');
var curConn = null;

var peer = new peerLib(
    PEERID, { /*options go here */ }
);

peer.on('open', function (id) {
    console.log('My peer ID is: ' + id);
});

peer.on('connection', function (conn) {
    // TODO do I need this?
    conn.serialization = 'json';
    console.log(`connected to ${conn.peer}`);
    conn.on('close', () => curConn = null);
    curConn = conn;
});

app.get('/:rgbparam([a-fA-F0-9]{6})/', function (req, res, next) {
    // pretty redundant at this point ...
    // rgbRegex = /^[a-fA-F0-9]{6}$/;
    param = req.params['rgbparam'];
    if (curConn !== null) {
        // send
        res.write(`successfully sent request of ${param} to laptop`);
        curConn.send(JSON.stringify({
            rgbVal: param
        }));
    } else {
        res.write("laptop disconnected");
    }
    res.end();
});

app.listen(PORT, () => console.log(`Example app listening on port ${PORT}`));