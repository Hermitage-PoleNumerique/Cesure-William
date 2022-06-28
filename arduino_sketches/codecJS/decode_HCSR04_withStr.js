// Decode decodes an array of bytes into an object.
//  - fPort contains the LoRaWAN fPort number
//  - bytes is an array of bytes, e.g. [225, 230, 255, 0]
//  - variables contains the device variables e.g. {"calibration": "3.5"} (both the key / value are of type string)
// The function must return an object, e.g. {"temperature": 22.5}

function bin2String(array) {
  var result = "";
  for (var i = 0; i < array.length; i++) {
    result += String.fromCharCode(array[i]);
  }
  return result;
}   
function Decode(fPort, bytes) {
  //var bytesString = String.fromCharCode(...bytes);
  var bytesString = bin2String(bytes);
      //String.fromCharCode(...bytes);
  var decoded = {};
  for (var i=0; i<bytesString.length;i++){
  	if(bytesString[i] == 'D'){
      for(var j=i+2; j<bytesString.length; j++){
      	if(bytesString[j] == '/'){
           decoded.distance = parseFloat(bytesString.slice(i+2,j));
           i = j;
           break;
        }
      }
      if(i!=j){
      	decoded.distance = parseFloat(bytesString.slice(i+2));
        break;
      }
    }
    if(bytesString[i] == 'I'){
      for(var j=i+1; j<bytesString.length; j++){
      	if(bytesString[j] == '/'){
           decoded.interruption = parseInt(bytesString.slice(i+1,j));
           i = j;
           break;
        }
      }
      if(i!=j){
      	decoded.interruption = parseInt(bytesString.slice(i+1));
        break;
      }
    }
  }
  return decoded;
}

// \!D/283.25
// plain payload hex
