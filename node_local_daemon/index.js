//////// binding stuff ////////

const ffi = require('ffi');
const ref = require('ref');
var uint8Type = ref.types.uint8;
var uint16Type = ref.types.uint16;
var voidType = ref.types.void;

var mathlibLoc = '../src/library/build/Release/alienfx';
var lib = ffi.Library(mathlibLoc, {
    "set_colors": [voidType, [uint8Type, uint16Type, uint8Type, uint8Type, uint8Type, uint8Type]],
    "increment_colors": [voidType, [uint8Type, uint16Type, uint8Type, uint8Type, uint8Type, uint8Type]],
    "decrement_colors": [voidType, [uint8Type, uint16Type, uint8Type, uint8Type, uint8Type, uint8Type]],
    "set_freq": [voidType, [uint8Type, uint16Type, uint16Type]],
    "increment_freq": [voidType, [uint8Type, uint16Type, uint16Type]],
    "decrement_freq": [voidType, [uint8Type, uint16Type, uint16Type]],
    "toggle_pause": [voidType, [uint8Type, uint16Type]],
    "set_profile": [voidType, [uint8Type]]
});

//////// p2p stuff ////////

const peerLib = require('peerjs-nodejs');
const PEERID = '42069';

var peer = new peerLib();

peer.on('open', (id) => console.log(`peer is id ${id}`));

var conn = peer.connect(PEERID);
conn.serialization = 'json'
conn.on('data', (data) => {
    const rgbRegex = /^[a-fA-F0-9]{6}$/;
    var rgbval = JSON.parse(data)['rgbVal'];
    console.log(data + rgbval + Object.keys(JSON.parse(data)));
    if (rgbRegex.test(rgbval)) {
        // TODO
        const red = parseInt(rgbval.substring(0, 2), 16);
        const green = parseInt(rgbval.substring(2, 4));
        const blue = parseInt(rgbval.substring(4, 6));
        if (0 <= red && red <= 0xff && 0 <= green && green <= 0xff && 0 <= blue && blue <= 0xff) {
            console.log("yeet")
            // doing the default one ...
            lib.set_profile(0x0);
            lib.set_colors(0x0, 0xffff, 0xff, red, green, blue);
        }
        console.log("success!");
    } else {
        console.log("incorrect formatting ... ?");
    }
    console.log(data);
});