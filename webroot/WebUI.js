class WebUI
{
  constructor()
  {
    this.availableLightSelectoNode = document.getElementById("availableLightSelector");
    //this.availableLightSelectoNode.onchange = (data) => {this._manageLight(data.target.value);};
    this.emptyAvailableOption = document.getElementById("emptyAvailableLightOption");

    this.syncedLightSelectoNode = document.getElementById("syncedLightSelector");
    this.syncedLightSelectoNode.onchange = (data) => {this._manageLight(data.target.value);};

    this.lights = {};
    this.screenPreview = new ScreenPreview(this);

    this.syncButton = document.getElementById("syncButton");
    this.syncButton.addEventListener("click", () => {this._syncLight()});
  }

  initUI()
  {
    RequestUtils.get("http://127.0.0.1:8080/availableLights", (data) => this._availableLightsCallback(data));
    RequestUtils.get("http://127.0.0.1:8080/syncedLights", (data) => this._syncedLightsCallback(data));
    RequestUtils.get("http://127.0.0.1:8080/screen", (data) => this._screenPreviewCallback(data));
  }


  notifyUVs(uvs)
  {
    RequestUtils.put("http://127.0.0.1:8080/setLightUVs/" + this.screenPreview.currentLight.id, JSON.stringify(uvs), (data) => {log("Set uv");});

  }


  _syncLight()
  {
    let lightId = this.availableLightSelectoNode.value;
    log(lightId);

    RequestUtils.post("http://127.0.0.1:8080/syncLight", JSON.stringify({lightId : lightId}), (data) => {log(`syncing ${data}`);});
  }


  _availableLightsCallback(jsonLights)
  {
    let lights = JSON.parse(jsonLights);
    if(lights.length > 0){
      this.availableLightSelectoNode.disabled = false;
      this.emptyAvailableOption.innerHTML = "Select a light to sync...";
    }

    for(let lightData of lights){
      let newLight = new Light(lightData)
      this.lights[newLight.id] = newLight;

      let newLightOption = document.createElement("option");
      newLightOption.value = newLight.id;
      newLightOption.innerHTML = `${newLight.name} - ${newLight.productName}`;
      this.availableLightSelectoNode.appendChild(newLightOption);
    }
  }


  _syncedLightsCallback(jsonLights)
  {
    let lights = JSON.parse(jsonLights);
    if(lights.length > 0){
      this.syncedLightSelectoNode.disabled = false;
      this.emptySyncedOption.innerHTML = "Select a light to manage...";
    }

    for(let lightData of lights){
      let newLight = new Light(lightData)
      this.lights[newLight.id] = newLight;

      let newLightOption = document.createElement("option");
      newLightOption.value = newLight.id;
      newLightOption.innerHTML = `${newLight.name} - ${newLight.productName}`;
      this.syncedLightSelectoNode.appendChild(newLightOption);
    }

    //this.screenPreview.initLightRegion(this.lights["3"]); // ToDO : Remove
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
    this.screenPreview.initLightRegion(this.lights[lightId]);
  }
}
