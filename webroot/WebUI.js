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

    this.controller = new Controller("svgArea", this);

    this.syncButton = document.getElementById("syncButton");
    this.syncButton.addEventListener("click", () => {this._syncLight();});

    this.saveProfileButton = document.getElementById("saveProfileButton");
    this.saveProfileButton.addEventListener("click", () => {this._saveProfile()})
  }

  initUI()
  {
    RequestUtils.get("/allLights", (data) => this._refreshLightLists(data));
    RequestUtils.get("/screen", (data) => this._screenPreviewCallback(data));
  }


  notifyUV(uvData)
  {
    RequestUtils.put("/setLightUV/" + this.controller.currentLight.id, JSON.stringify(uvData), (jsonCheckedUVs) => {this.controller.uvCallback(JSON.parse(jsonCheckedUVs));});
  }


  _syncLight()
  {
    let lightId = this.availableLightSelectorNode.value;

    RequestUtils.post("/syncLight", JSON.stringify({id : lightId}), (jsonData) => {
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
        if(option.value == syncedLight.id){
          option.disabled = true;
        }
      }

      let newLight = new Light(syncedLight)
      this.availableLights[newLight.id] = newLight;
      
      let duplicate = [...this.syncedLightSelectorNode.options].map(o => o.value).includes(syncedLight.id);

      if(!duplicate){
        let newLightOption = document.createElement("option");
        newLightOption.value = newLight.id;
        newLightOption.innerHTML = `${newLight.name} - ${newLight.productName}`;
        this.syncedLightSelectorNode.appendChild(newLightOption);

        this.syncedLightSelectorNode.value = newLight.id;
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
      this.availableLights[newLight.id] = newLight;

      let newLightOption = document.createElement("option");
      newLightOption.value = newLight.id;
      newLightOption.innerHTML = `${newLight.name} - ${newLight.productName}`;
      this.availableLightSelectorNode.appendChild(newLightOption);
    }
  }


  _screenPreviewCallback(jsonScreen)
  {
    let screen = JSON.parse(jsonScreen);
    let x = screen.x;
    let y = screen.y;

    //this.screenPreview.setDimensions(x, y);
  }


  _manageLight(lightId)
  {
    let light = this.availableLights[lightId];
    this.controller.initLightRegion(light);
  }


  _saveProfile()
  {
    RequestUtils.post("/saveProfile", JSON.stringify(null), (data) => {log("Saved profile");});
  }
}
