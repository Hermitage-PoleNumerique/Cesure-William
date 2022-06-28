// Decode decodes an array of bytes into an object.
//  - fPort contains the LoRaWAN fPort number
//  - bytes is an array of bytes, e.g. [225, 230, 255, 0]
//  - variables contains the device variables e.g. {"calibration": "3.5"} (both the key / value are of type string)
// The function must return an object, e.g. {"temperature": 22.5}


function Decode(fPort, bytes) {

    var n_irq_pins = 1;

    var decoded = {};
    decoded.irq_pin2 = false;
    //decoded.irq_pin3 = false;
    decoded.distance = null;

    var charCode = '';
    for (var i=0; i<bytes.length; i++){
        if(i<n_irq_pins){
            charCode = String.fromCharCode(bytes[i]);
            switch(charCode){
                case 'I':
                    decoded.irq_pin2 = true;
                    break;
                //case 'J':
                //    decoded.irq_pin3 = true;
                //    break;
            }
        }
        else{
            var j = (i-n_irq_pins)*3+n_irq_pins;
            charCode = String.fromCharCode(bytes[j]);
            var bytes_val = (bytes[j+1]& 0xFF) | (bytes[j+2]& 0xFF) << 8 | (bytes[j+3]& 0xFF) << 16 | (bytes[j+4]& 0xFF) <<24
            var f_val = Bytes2Float32(bytes_val);

            switch(charCode){
                case 'D':
                    decoded.distance = f_val;
                    break;
            }
        }
    }

    return decoded;
}

function Bytes2Float32(bytes) {
    var sign = (bytes & 0x80000000) ? -1 : 1;
    var exponent = ((bytes >> 23) & 0xFF) - 127;
    var significand = (bytes & ~(-1 << 23));

    if (exponent == 128) 
        return sign * ((significand) ? Number.NaN : Number.POSITIVE_INFINITY);

    if (exponent == -127) {
        if (significand == 0) return sign * 0.0;
        exponent = -126;
        significand /= (1 << 22);
    } else significand = (significand | (1 << 23)) / (1 << 23);

    return sign * significand * Math.pow(2, exponent);
}
