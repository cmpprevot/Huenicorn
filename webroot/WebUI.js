class WebUI
{
  constructor()
  {
    this.lightSelectoNode = document.getElementById("lightSelector");
    this.lightSelectoNode.onchange = (data) => {this._manageLight(data.target.value);};
    this.emptyOption = document.getElementById("emptyLightOption");

    this.lights = {};
    this.screenPreview = new ScreenPreview(this);
  }

  initUI()
  {
    RequestUtils.get("http://127.0.0.1:8080/lights", (data) => this._lightsCallback(data));
    RequestUtils.get("http://127.0.0.1:8080/screen", (data) => this._screenPreviewCallback(data));
  }


  notifyUVs(uvs)
  {
    RequestUtils.put("http://127.0.0.1:8080/setLightUVs/" + this.screenPreview.currentLight.id, JSON.stringify(uvs), (data) => {log("Set uv");});

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

    this.screenPreview.initLightRegion(this.lights["3"]); // ToDO : Remove
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
