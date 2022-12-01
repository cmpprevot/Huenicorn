class WebUI
{
  constructor()
  {
    this.syncedLightsNode = document.getElementById("syncedLightsList");
    this.syncedLightsNode.addEventListener("drop", (event) => {
      this._syncLight(JSON.parse(event.dataTransfer.getData("lightData")).id);
    });

    this.legendText = document.getElementById("legendText");

    this.availableLightsNode = document.getElementById("availableLightsList");
    this.availableLightsNode.addEventListener("drop", (event) => {
      this._unsyncLight(JSON.parse(event.dataTransfer.getData("lightData")).id);
    });

    document.addEventListener("dragover", (event) => {event.preventDefault();});

    this.syncedLights = {};

    this.controller = new Controller("svgArea", this);

    this.saveProfileButton = document.getElementById("saveProfileButton");
    this.saveProfileButton.addEventListener("click", () => {this._saveProfile()})
  }

  initUI()
  {
    RequestUtils.get("/allLights", (data) => this._refreshLightLists(JSON.parse(data)));
    RequestUtils.get("/screen", (data) => this._screenPreviewCallback(data));
  }


  notifyUV(uvData)
  {
    RequestUtils.put("/setLightUV/" + this.controller.currentLight.id, JSON.stringify(uvData), (jsonCheckedUVs) => {this.controller.uvCallback(JSON.parse(jsonCheckedUVs));});
  }


  _setLegendText(legendText)
  {
    this.legendText.innerHTML = legendText;
  }


  _syncLight(lightId)
  {
    RequestUtils.post("/syncLight", JSON.stringify({id : lightId}), (jsonData) => {
      let data = JSON.parse(jsonData);
      this._refreshLightLists(data.lights)
      if("newSyncedLightId" in data){
        this._manageLight(data["newSyncedLightId"]);
      }
    });
  }


  _unsyncLight(lightId)
  {
    RequestUtils.post("/unsyncLight", JSON.stringify({id : lightId}), (jsonData) => {
      let data = JSON.parse(jsonData);
      this._refreshLightLists(data.lights)
    });
  }


  _refreshLightLists(lights)
  {
    this.syncedLights = {};
    this._refreshSyncedLights(lights.synced);
    this._refreshAvailableLights(lights.available);

    log(Object.keys(this.syncedLights).length)

    if(lights.available.length == 0){
      this._setLegendText("There are currently no available lights. Please register them through official application.");
    }
    else if(Object.keys(this.syncedLights).length == 0){
      this._setLegendText("Drag and and drop light from 'available' to 'synced' box to manage it.");
    }
    else{
      this._setLegendText("");
    }
  }


  _refreshSyncedLights(syncedLights)
  {
    this.syncedLightsNode.innerHTML = "";
    for(let lightData of syncedLights){
      let newLightEntryNode = document.createElement("p");
      newLightEntryNode.draggable = true;

      let newLight = new Light(lightData)
      newLightEntryNode.addEventListener("dragstart", (event) => {
        event.dataTransfer.setData("lightData", JSON.stringify(newLight));
      });

      newLightEntryNode.addEventListener("click", (event) => {
        this._manageLight(lightData.id);
      });

      newLightEntryNode.innerHTML = `${newLight.name} - ${newLight.productName}`;

      this.syncedLightsNode.appendChild(newLightEntryNode);
      this.syncedLights[newLight.id] = newLight;
    }
  }


  _refreshAvailableLights(availableLights)
  {
    this.availableLightsNode.innerHTML = "";

    for(let lightData of availableLights){
      if(lightData.id in this.syncedLights){
        continue;
      }


      let newLightEntryNode = document.createElement("p");
      newLightEntryNode.draggable = true;

      let newLight = new Light(lightData)
      newLightEntryNode.addEventListener("dragstart", (event) => {
        event.dataTransfer.setData("lightData", JSON.stringify(newLight));
      });

      newLightEntryNode.innerHTML = `${newLight.name} - ${newLight.productName}`;

      this.availableLightsNode.appendChild(newLightEntryNode);
    }
  }


  _screenPreviewCallback(jsonScreen)
  {
    // ToDo : get ratio / resampled res to display / clip the handlers
    /*
    let screen = JSON.parse(jsonScreen);
    let x = screen.x;
    let y = screen.y;

    //this.screenPreview.setDimensions(x, y);
    */
  }


  _manageLight(lightId)
  {
    RequestUtils.get(`/syncedLight/${lightId}`, (data) => {this.controller.initLightRegion(JSON.parse(data));});
  }


  _saveProfile()
  {
    RequestUtils.post("/saveProfile", JSON.stringify(null), (data) => {log("Saved profile");});
  }
}
