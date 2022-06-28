// Decode decodes an array of bytes into an object.
//  - fPort contains the LoRaWAN fPort number
//  - bytes is an array of bytes, e.g. [225, 230, 255, 0]
//  - variables contains the device variables e.g. {"calibration": "3.5"} (both the key / value are of type string)
// The function must return an object, e.g. {"temperature": 22.5}
// Decode decodes an array of bytes into an object.
//  - fPort contains the LoRaWAN fPort number
//  - bytes is an array of bytes, e.g. [225, 230, 255, 0]
//  - variables contains the device variables e.g. {"calibration": "3.5"} (both the key / value are of type string)
// The function must return an object, e.g. {"temperature": 22.5}


function Decode(fPort, bytes) {

    //Changer la valeur de n_irq_pins en fonction du nombre de pin(s) d'interruption(s) activé(s), 0, 1 ou 2 pins (activé(s) si IRQ_PIN2 et/ou IRQ_PIN3 défini(s) dans le programme de l'Arduino).
    var n_irq_pins = 1;

    var decoded = {};
    decoded.irq_pin2 = false;
    decoded.irq_pin3 = false;

    decoded.humidity = null;
    decoded.temperature = null;
  	decoded.voltage = null;


    var charCode = '';
    for (var i=0; i<n_irq_pins; i++){
        charCode = String.fromCharCode(bytes[i]);
        switch(charCode){
            case 'I':
                decoded.irq_pin2 = true;
                break;
            case 'J':
                decoded.irq_pin3 = true;
                break;
        }
    }
    for (var j=n_irq_pins; j<bytes.length; j+=5){
        charCode = String.fromCharCode(bytes[j]);

        var bytes_val = [bytes[j+4], bytes[j+3], bytes[j+2], bytes[j+1]];
        var f_val = decodeFloat(bytes_val);

        //Modifier ici le code pour ajouter/supprimer des données de capteurs à récupérer
        switch(charCode){
            case 'T'://Nomenclature identifiant la donnée du capteur venant de l'Arduino
                decoded.temperature = f_val;//Nomenclature identifiant la donnée dans l'objet json généré
                break;
            case 'H':
                decoded.humidity = f_val;
                break;
            case 'V':
                decoded.voltage = f_val;
                break;
        }
    }

    return decoded;
}



function dec2binString(dec) {
  res_str = (dec >>> 0).toString(2);
  while(res_str.length<8)
    res_str = '0' + res_str;

  return res_str;
}

function bytesArray2binString(array) {
  var result = "";
  for (var i = 0; i < array.length; i++) {
    result += dec2binString(array[i]);
  }
  return result;
}

function decodeFloat(data) {
    var binary = bytesArray2binString(data);
    if (binary.length < 32) 
        binary = ('00000000000000000000000000000000'+binary).substr(binary.length);
    var sign = (binary.charAt(0) == '1')?-1:1;
    var exponent = parseInt(binary.substr(1, 8), 2) - 127;
    var significandBase = binary.substr(9);
    var significandBin = '1'+significandBase;
    var i = 0;
    var val = 1;
    var significand = 0;

    if (exponent == -127) {
        if (significandBase.indexOf('1') == -1)
            return 0;
        else {
            exponent = -126;
            significandBin = '0'+significandBase;
        }
    }

    while (i < significandBin.length) {
        significand += val * parseInt(significandBin.charAt(i));
        val = val / 2;
        i++;
    }

    return sign * significand * Math.pow(2, exponent);
}



