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
};
