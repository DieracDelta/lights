const express = require('express')
const app = express()
const PORT = 3000

fieldToCode = {
    kbFarRight: 0x1,
    kbMidRight: 0x2,
    kbMidLeft: 0x4,
    kbFarLeft: 0x8,
    alienHead: 0x20,
    alienwareName: 0x40,
    trackpad: 0x80,
    powerButton: 0x100,
    botLeftBurner: 0x400,
    botRightBurner: 0x800,
    topLeftBurner: 0x1000,
    topRightBurner: 0x2000,
    kbSpecial: 0x4000,
    allTheThings: 0xffff
};

///////// p2p communication //////

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
            rgbVal: param,
            location: fieldToCode['allTheThings']
        }));
    } else {
        res.write("laptop disconnected");
    }
    res.end();
});


app.get('/:location/:rgbparam([a-fA-F0-9]{6})/', function (req, res, next) {
    const loc = req.params['location'];
    const rgbparam = req.params['rgbparam'];
    if (loc in fieldToCode) {
        res.write(`successfully sent request of ${rgbparam} to laptop in region ${loc}`);
        curConn.send(JSON.stringify({
            rgbVal: rgbparam,
            location: loc
        }));


    } else {
        res.write("invalid location!");
    }
    res.end();
});

app.listen(PORT, () => console.log(`Example app listening on port ${PORT}`));