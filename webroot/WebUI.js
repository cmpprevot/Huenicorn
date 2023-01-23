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

    this.screenWidget = new ScreenWidget(this);

    this.advancedSettingsCheckbox = document.getElementById("advancedSettingsCheckbox");
    this.advancedSettingsCheckbox.addEventListener("click", (e) => {this._toggleAdvancedDisplay(e.target.checked);});
    this.advancedSettingsNode = document.getElementById("advancedSettings");
    this.availableSubsamplesNode = document.getElementById("availableSubsampleWidths");
    this.availableSubsamplesNode.addEventListener("change", (event) => {this._setSubsampleWidth(parseInt(event.target.value));});
    this.refreshRateInputNode = document.getElementById("refreshRate");
    this.refreshRateInputNode.addEventListener("change", (event) => {this._setRefreshRate(event.target.valueAsNumber);});
    this.transitionTimeInputNode = document.getElementById("transitionTime");
    this.transitionTimeInputNode.addEventListener("change", (event) => {this._setTransitionTime(event.target.valueAsNumber);});

    this.saveProfileButton = document.getElementById("saveProfileButton");
    this.saveProfileButton.addEventListener("click", () => {this._saveProfile()});
  }


  initUI()
  {
    RequestUtils.get("/allLights", (data) => this._refreshLightLists(JSON.parse(data)));
    RequestUtils.get("/displayInfo", (data) => this._displayInfoCallback(JSON.parse(data)));
    RequestUtils.get("/transitionTime_c", (data) => this._setTransitionTimeCallback(JSON.parse(data).transitionTime));
  }


  notifyUV(uvData)
  {
    RequestUtils.put(`/setLightUV/${this.screenWidget.currentLight.id}`, JSON.stringify(uvData), (jsonCheckedUVs) => {
      let checkedUVs = JSON.parse(jsonCheckedUVs);
      this.syncedLights[this.screenWidget.currentLight.id].uvs = checkedUVs;
      this.screenWidget.uvCallback(checkedUVs);
    });
  }


  updateGammaFactor(gammaFactor)
  {
    if(!this.screenWidget.currentLight){
      return;
    }

    if(gammaFactor == this.screenWidget.currentLight.gammaFactor){
      return;
    }

    this.screenWidget.currentLight.gammaFactor = gammaFactor;

    RequestUtils.put(`/setLightGammaFactor/${this.screenWidget.currentLight.id}`, JSON.stringify({gammaFactor : gammaFactor}), (jsonGammaFactorData) => {
      let gammaFactorData = JSON.parse(jsonGammaFactorData);
      let gammaFactor = gammaFactorData.gammaFactor;
      this.screenWidget.currentLight.gammaFactor = Utils.truncate(gammaFactor, 2);
    });
  }


  _initAdvancedSettings()
  {
    let showAdvancedSettings = false;
    this.advancedSettingsCheckbox.checked = showAdvancedSettings;
    this._toggleAdvancedDisplay(showAdvancedSettings);

    this.availableSubsamplesNode.innerHTML = "";
    for(let subsampleCandidate of this.screenWidget.subsampleResolutionCandidates.reverse()){
      let newOption = document.createElement("option");
      let width = subsampleCandidate.x;
      let height = subsampleCandidate.y;
      let percentage = width / this.screenWidget.width * 100;
      percentage = MathUtils.roundPrecision(percentage, 2);
      newOption.innerHTML = width + "x" + height;
      newOption.innerHTML += ` (${percentage}%)`;
      newOption.value = width;
      this.availableSubsamplesNode.appendChild(newOption);
    }
    
    let proportion = parseInt(this.availableSubsamplesNode.value) / this.screenWidget.width;
    let gradientColor = StyleUtils.greenRedGradient(proportion);
    this.availableSubsamplesNode.style.color = gradientColor;
  }


  _toggleAdvancedDisplay(checked)
  {
    if(checked){
      this.advancedSettingsNode.style.display = "block";
    }
    else{
      this.advancedSettingsNode.style.display = "none";
    }
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
      this.screenWidget.showPreview();
    });
  }


  _refreshLightLists(lights)
  {
    this.syncedLights = {};
    this._refreshSyncedLights(lights.synced);
    this._refreshAvailableLights(lights.available);

    if(lights.available.length == 0){
      this.screenWidget.setLegend(ScreenWidget.Legends.noLight);
    }
    else if(Object.keys(this.syncedLights).length == 0){
      this.screenWidget.setLegend(ScreenWidget.Legends.pleaseDrag);
      this.screenWidget.showWidgets(false);
    }
    else{
      this.screenWidget.setLegend(ScreenWidget.Legends.pleaseSelect);
    }
    
    this.screenWidget.showPreview();
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

      //newLightEntryNode.innerHTML = `${newLight.name} - ${newLight.productName}`;
      newLightEntryNode.innerHTML = newLight.name;

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

      //newLightEntryNode.innerHTML = `${newLight.name} - ${newLight.productName}`;
      newLightEntryNode.innerHTML = newLight.name;

      this.availableLightsNode.appendChild(newLightEntryNode);
    }
  }


  _displayInfoCallback(displayInfo)
  {
    let x = displayInfo.x;
    let y = displayInfo.y;
    let subsampleWidth = displayInfo.subsampleWidth;

    this.screenWidget.setDimensions(x, y, subsampleWidth);
    this.screenWidget.setSubsampleCandidates(displayInfo.subsampleResolutionCandidates);

    this._initAdvancedSettings();
    this.availableSubsamplesNode.value = subsampleWidth.toString();
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
      this.screenWidget.showPreview();
      this.screenWidget.currentLight = null;

      return;
    }
    else{
      this.screenWidget.showPreview(lightId);
    }

    this.screenWidget.setLegend(ScreenWidget.Legends.none);

    RequestUtils.get(`/syncedLight/${lightId}`, (jsonSyncedLight) => {
      let syncedLightData = JSON.parse(jsonSyncedLight);
      this.syncedLights[lightId].uvs = syncedLightData.uvs;
      this.screenWidget.initLightRegion(this.syncedLights[lightId]);
    });
  }


  _setSubsampleWidth(subsampleWidth)
  {
    RequestUtils.put("/setSubsampleWidth", JSON.stringify(subsampleWidth), (jsonDisplayInfo) => {
      let displayInfo = JSON.parse(jsonDisplayInfo);
      this.screenWidget.setDimensions(displayInfo.x, displayInfo.y, displayInfo.subsampleWidth);

      let proportion = subsampleWidth / this.screenWidget.width;

      let gradientColor = StyleUtils.greenRedGradient(proportion);
      this.availableSubsamplesNode.style.color = gradientColor;
    });
  }


  _setRefreshRate(refreshRate){
    RequestUtils.put("/setRefreshRate", JSON.stringify(refreshRate), (data) => {this._setRefreshRateCallback(JSON.parse(data).refreshRate);});
  }


  _setTransitionTime(transitionTime)
  {
    RequestUtils.put("/setTransitionTime_c", JSON.stringify(transitionTime), (data) => {this._setTransitionTimeCallback(JSON.parse(data).transitionTime_c);});
  }


  _setRefreshRateCallback(refreshRate)
  {
    this.refreshRateInputNode.value = refreshRate;
    if(refreshRate.value > 10){
      this.refreshRateInputNode.style.color = "#ff0000";
    }
  }


  _setTransitionTimeCallback(transitionTime)
  {
    this.transitionTimeInputNode.value = transitionTime;
  }


  _saveProfile()
  {
    RequestUtils.post("/saveProfile", JSON.stringify(null), (data) => {log("Saved profile");});
  }
}


if(new URLSearchParams(window.location.search).has("unleashTheUnicorn")){
  var swagIncrement = 180;
  document.documentElement.style.setProperty("--shadowRadius", "60px");
  setInterval(() => {
    document.documentElement.style.setProperty("--shadowColor", `${swagIncrement}, 100%, 50%`);
    swagIncrement = (this.swagIncrement + 1) % 360;
  }, 50);
}
