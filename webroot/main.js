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


class Light
{
  constructor(data)
  {
    this.id = data.id;
    this.name = data.name;
    this.productName = data.productName;
    //this.position = data.position;
  }
}


class ScreenPreview
{
  constructor(){
    this.screenPreviewNode = document.getElementById("screenPreview");
    this.screenPreviewNode.style.backgroundColor = "#000000";
    this.width = 512;
    this.screenPreviewNode.style.width = this.width + "px";
  }


  setDimensions(screenX, screenY)
  {
    let ratio = screenY / screenX;
    this.screenPreviewNode.style.height = this.width * ratio + "px";
  }


  showLightRegion(light)
  {
    log(light);
  }
}


class WebUI
{
  constructor()
  {
    this.lightSelectoNode = document.getElementById("lightSelector");
    this.lightSelectoNode.onchange = (data) => {this._manageLight(data.target.value);};
    this.emptyOption = document.getElementById("emptyLightOption");

    this.lights = {};
    this.screenPreview = new ScreenPreview();
  }

  initUI()
  {
    RequestUtils.get("http://127.0.0.1:8080/lights", (data) => this._lightsCallback(data));
    RequestUtils.get("http://127.0.0.1:8080/screen", (data) => this._screenPreviewCallback(data));
  }


  _lightsCallback(jsonLights)
  {
    let lights = JSON.parse(jsonLights);
    if(lights.length > 0){
      this.lightSelectoNode.disabled = false;
      this.emptyOption.innerHTML = "Select a light to manage...";
    }

    for(let lightData of lights){
      let newLight = new Light(lightData)
      this.lights[newLight.id] = newLight;

      let newLightOption = document.createElement("option");
      newLightOption.value = newLight.id;
      newLightOption.innerHTML = `${newLight.name} - ${newLight.productName}`;
      this.lightSelectoNode.appendChild(newLightOption);
    }
  }


  _screenPreviewCallback(jsonScreen)
  {
    let screen = JSON.parse(jsonScreen);
    let x = screen.x;
    let y = screen.y;

    this.screenPreview.setDimensions(x, y);
  }


  _manageLight(lightId)
  {
    log(lightId);

    this.screenPreview.showLightRegion(this.lights[lightId]);
  }
}

let webUI = new WebUI();

webUI.initUI();

let data = {uvA : {x : 0, y : 1}, uvB : {x : 2, y : 3}};
RequestUtils.put("http://127.0.0.1:8080/setLightUVs/4", JSON.stringify(data));

