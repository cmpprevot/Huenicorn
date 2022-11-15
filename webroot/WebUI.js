class WebUI
{
  constructor()
  {
    this.availableLightSelectorNode = document.getElementById("availableLightSelector");
    this.emptyAvailableOption = document.getElementById("emptyAvailableLightOption");
    this.emptySyncedOption = document.getElementById("emptySyncedLightOption");

    this.syncedLightSelectorNode = document.getElementById("syncedLightSelector");
    this.syncedLightSelectorNode.onchange = (data) => {this._manageLight(data.target.value);};

    this.availableLights = {};
    this.screenPreview = new ScreenPreview(this);

    this.syncButton = document.getElementById("syncButton");
    this.syncButton.addEventListener("click", () => {this._syncLight();});

    this.saveProfileButton = document.getElementById("saveProfileButton");
    this.saveProfileButton.addEventListener("click", () => {this._saveProfile()})
  }

  initUI()
  {
    RequestUtils.get("http://127.0.0.1:8080/allLights", (data) => this._refreshLightLists(data));
    RequestUtils.get("http://127.0.0.1:8080/screen", (data) => this._screenPreviewCallback(data));
  }


  notifyUVs(uvs)
  {
    RequestUtils.put("http://127.0.0.1:8080/setLightUVs/" + this.screenPreview.currentLight.lightId, JSON.stringify(uvs), (data) => {log("Set uv");});
  }


  _syncLight()
  {
    let lightId = this.availableLightSelectorNode.value;

    RequestUtils.post("http://127.0.0.1:8080/syncLight", JSON.stringify({lightId : lightId}), (jsonData) => {
      let data = JSON.parse(jsonData);
      this._refreshSyncedLights(data["syncedLights"]);
      if("newSyncedLightId" in data){
        this._manageLight(data["newSyncedLightId"]);
      }
    });
  }


  _refreshLightLists(jsonData)
  {
    let data = JSON.parse(jsonData);
    this._refreshAvailableLights(data["available"]);
    this._refreshSyncedLights(data["synced"]);
  }

  _refreshSyncedLights(syncedLights)
  {
    // Disable in available list
    for(let syncedLight of syncedLights){
      this.syncedLightSelectorNode.disabled = false;
      for(let option of this.availableLightSelectorNode.options){
        if(option.value == syncedLight.lightId){
          option.disabled = true;
        }
      }

      let newLight = new Light(syncedLight)
      this.availableLights[newLight.lightId] = newLight;
      
      let duplicate = [...this.syncedLightSelectorNode.options].map(o => o.value).includes(syncedLight.lightId);

      if(!duplicate){
        let newLightOption = document.createElement("option");
        newLightOption.value = newLight.lightId;
        newLightOption.innerHTML = `${newLight.name} - ${newLight.productName}`;
        this.syncedLightSelectorNode.appendChild(newLightOption);

        this.syncedLightSelectorNode.value = newLight.lightId;
      }
    }
  }


  _refreshAvailableLights(availableLights)
  {
    if(availableLights.length > 0){
      this.availableLightSelectorNode.disabled = false;
      this.emptyAvailableOption.innerHTML = "Select a light to sync...";
    }

    for(let lightData of availableLights){
      let newLight = new Light(lightData)
      this.availableLights[newLight.lightId] = newLight;

      let newLightOption = document.createElement("option");
      newLightOption.value = newLight.lightId;
      newLightOption.innerHTML = `${newLight.name} - ${newLight.productName}`;
      this.availableLightSelectorNode.appendChild(newLightOption);
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
    let light = this.availableLights[lightId];
    this.screenPreview.initLightRegion(light);
  }


  _saveProfile()
  {
    RequestUtils.post("http://127.0.0.1:8080/saveProfile", JSON.stringify(null), (data) => {log("Saved profile");});
  }
}
