function Decode(fPort, bytes) {

    var decoded = {};
    decoded.i_id = bytes[0];
    decoded.i_burst = bytes[1];

    return decoded;
}
