class WebUI
{
  constructor()
  {
    this.syncedLightsNode = document.getElementById("syncedLightsList");
    this.syncedLightsNode.addEventListener("drop", (event) => {
      this._syncLight(JSON.parse(event.dataTransfer.getData("lightData")).id);
    });

    this.availableLightsNode = document.getElementById("availableLightsList");
    this.availableLightsNode.addEventListener("drop", (event) => {
      this._unsyncLight(JSON.parse(event.dataTransfer.getData("lightData")).id);
    });

    document.addEventListener("dragover", (event) => {event.preventDefault();});

    this.syncedLights = {};

    this.screenWidget = new ScreenWidget("svgArea", this);

    this.saveProfileButton = document.getElementById("saveProfileButton");
    this.saveProfileButton.addEventListener("click", () => {this._saveProfile()});
  }


  initUI()
  {
    RequestUtils.get("/allLights", (data) => this._refreshLightLists(JSON.parse(data)));
    RequestUtils.get("/screen", (data) => this._screenPreviewCallback(data));
  }


  notifyUV(uvData)
  {
    RequestUtils.put("/setLightUV/" + this.screenWidget.currentLight.id, JSON.stringify(uvData), (jsonCheckedUVs) => {this.screenWidget.uvCallback(JSON.parse(jsonCheckedUVs));});
  }


  _syncLight(lightId)
  {
    RequestUtils.post("/syncLight", JSON.stringify({id : lightId}), (jsonData) => {
      let data = JSON.parse(jsonData);
      this._refreshLightLists(data.lights);
      
      if("newSyncedLightId" in data){
        this._manageLight(data["newSyncedLightId"]);
      }
    });
  }


  _unsyncLight(lightId)
  {
    RequestUtils.post("/unsyncLight", JSON.stringify({id : lightId}), (jsonData) => {
      let data = JSON.parse(jsonData);
      this._refreshLightLists(data.lights);
      this.screenWidget.showWidgets(false);
    });
  }


  _refreshLightLists(lights)
  {
    this.syncedLights = {};
    this._refreshSyncedLights(lights.synced);
    this._refreshAvailableLights(lights.available);

    if(lights.available.length == 0){
      this.screenArea.setLegend(ScreenWidget.legends.noLight);
    }
    else if(Object.keys(this.syncedLights).length == 0){
      this.screenWidget.setLegend(ScreenWidget.Legends.pleaseDrag);
      this.screenWidget.showWidgets(false);
    }
    else{
      this.screenWidget.setLegend(ScreenWidget.Legends.pleaseSelect);
    }
  }


  _refreshSyncedLights(syncedLights)
  {
    this.syncedLightsNode.innerHTML = "";
    for(let lightData of syncedLights){
      let newLightEntryNode = document.createElement("p");
      newLightEntryNode.draggable = true;
      newLightEntryNode.selected = false;

      let newLight = new Light(lightData, newLightEntryNode)
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


  _setItemSelected(lightNode, selected)
  {
    lightNode.selected = selected;

    if(lightNode.selected){
      lightNode.classList.add("selected");
    }
    else{
      lightNode.classList.remove("selected");
    }
  }


  _toggleClicked(lightNode)
  {
    this._setItemSelected(lightNode, !lightNode.selected);
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
    for(let loopLightId of Object.keys(this.syncedLights)){
      let lightNode = this.syncedLights[loopLightId].node;
      if(loopLightId != lightId){
        this._setItemSelected(lightNode, false);
      }
      else{
        this._toggleClicked(lightNode);
      }
    }

    if(!this.syncedLights[lightId].node.selected){
      this.screenWidget.setLegend(ScreenWidget.Legends.pleaseSelect);
      this.screenWidget.showWidgets(false);
      return;
    }

    this.screenWidget.setLegend(ScreenWidget.Legends.none);

    RequestUtils.get(`/syncedLight/${lightId}`, (jsonSyncedLight) => {
      let syncedLightData = JSON.parse(jsonSyncedLight);
      this.syncedLights[lightId].uvs = syncedLightData.uvs;
      this.screenWidget.initLightRegion(this.syncedLights[lightId]);
    });
  }


  _saveProfile()
  {
    RequestUtils.post("/saveProfile", JSON.stringify(null), (data) => {log("Saved profile");});
  }
}
