class RequestUtils
{
  //*/
  static defaultCallback = (response) => {log(response);};
  /*/
  static defaultCallback = (response) => {};
  //*/

  static get(url, callback = RequestUtils.defaultCallback){
    var xhr = new XMLHttpRequest();
    xhr.open("GET", url, true);
    xhr.onreadystatechange = function(){
      if(this.readyState === XMLHttpRequest.DONE && this.status === 200){
        callback(this.responseText);
      }
    };
    xhr.send();
  }

  static post(url, data = {}, callback = RequestUtils.defaultCallback){
    var xhr = new XMLHttpRequest();
    xhr.open("POST", url);
    xhr.setRequestHeader("Accept", "application/json");
    xhr.setRequestHeader("Content-Type", "application/json");
    xhr.onreadystatechange = function(){
      if(this.readyState === XMLHttpRequest.DONE && this.status === 200){
        callback(this.responseText)
      }
    };
    xhr.send(data);
  }


  static put(url, data = {}, callback = RequestUtils.defaultCallback){
    var xhr = new XMLHttpRequest();
    xhr.open("PUT", url);
    xhr.setRequestHeader("Accept", "application/json");
    xhr.setRequestHeader("Content-Type", "application/json");
    xhr.onreadystatechange = function(){
      if(this.readyState === XMLHttpRequest.DONE && this.status === 200){
        callback(this.responseText)
      }
    };
    xhr.send(data);
  }
}


class MathUtils
{
  static roundPrecision(num, decimals)
  {
    let factor = Math.pow(10, decimals);
    return Math.round(num * factor) / factor;
  }
}


class StyleUtils
{
  static greenRedGradient(proportion)
  {
    let rFactor = proportion;
    let gFactor = 1 - proportion;
    let bFactor = 0;

    let norm = Math.sqrt(rFactor * rFactor + gFactor * gFactor + bFactor * bFactor);

    rFactor /= norm;
    gFactor /= norm;
    bFactor /= norm;

    return `rgb(${rFactor * 255}, ${gFactor * 255}, ${bFactor * 255})`;
  }
}
