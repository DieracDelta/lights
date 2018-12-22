const peerLib = require('peerjs-nodejs');
const PEERID = '42069420420';

var peer = new peerLib();

peer.on('open', (id) => console.log(`peer is id ${id}`));

var conn = peer.connect(PEERID);
conn.serialization = 'json'
conn.on('data', (data) => {
    // TODO regex assertion
    console.log(data);
});