log = console.log;

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


class WebUI
{
  constructor()
  {
    this.lightSelectoNode = document.getElementById("lightSelector");
    this.lightSelectoNode.onchange = (data) => {log(data.target.selectedIndex)};
    this.emptyOption = document.getElementById("emptyLightOption");
  }

  initUI()
  {
    RequestUtils.get("http://127.0.0.1:8080/lights", (data) => this._lightsCallback(data));
  }


  _lightsCallback(jsonLights)
  {
    let lights = JSON.parse(jsonLights);
    if(lights.length > 0){
      this.lightSelectoNode.disabled = false;
      this.emptyOption.innerHTML = "Select a light to manage...";
    }

    for(let light of lights){
      log(light);
      let newLightOption = document.createElement("option");
      newLightOption.value = light.id;
      newLightOption.innerHTML = `${light.name} - ${light.productName}`
      this.lightSelectoNode.appendChild(newLightOption);
    }

  }
}

let webUI = new WebUI();

webUI.initUI();

