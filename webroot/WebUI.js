class WebUI
{
  constructor()
  {
    this.activeChannelsNode = document.getElementById("activeChannelsList");
    this.activeChannelsNode.addEventListener("drop", (event) => {
      this._setChannelActivity(JSON.parse(event.dataTransfer.getData("channelData")).channelId, true);
    });

    this.inactiveChannelsNode = document.getElementById("inactiveChannelsList");
    this.inactiveChannelsNode.addEventListener("drop", (event) => {
      this._setChannelActivity(JSON.parse(event.dataTransfer.getData("channelData")).channelId, false);
    });

    document.addEventListener("dragover", (event) => {event.preventDefault();});

    this.activeChannels = {};

    this.screenWidget = new ScreenWidget(this);

    this.advancedSettingsCheckbox = document.getElementById("advancedSettingsCheckbox");
    this.advancedSettingsCheckbox.addEventListener("click", (e) => {this._toggleAdvancedDisplay(e.target.checked);});
    this.advancedSettingsNode = document.getElementById("advancedSettings");
    this.availableSubsamplesNode = document.getElementById("availableSubsampleWidths");
    this.availableSubsamplesNode.addEventListener("change", (event) => {this._setSubsampleWidth(parseInt(event.target.value));});
    this.refreshRateInputNode = document.getElementById("refreshRate");
    this.refreshRateInputNode.addEventListener("change", (event) => {this._setRefreshRate(event.target.valueAsNumber);});

    this.saveProfileButton = document.getElementById("saveProfileButton");
    this.saveProfileButton.addEventListener("click", () => {this._saveProfile();});

    this.stopButton = document.getElementById("stopButton");
    this.stopButton.addEventListener("click", () => {this._askStopConfirmation()});
    
    this.overlay = document.getElementById("overlay");

    this.confirmStopButton = document.getElementById("confirmStopButton");
    this.confirmStopButton.addEventListener("click", () => {this._stop();});
    
    this.cancelStopButton = document.getElementById("cancelStopButton");
    this.cancelStopButton.addEventListener("click", () => {this.overlay.style.display = "none";});
  }


  initUI()
  {
    RequestUtils.get("/channels", (data) => this._refreshChannelsLists(JSON.parse(data)));
    RequestUtils.get("/displayInfo", (data) => this._displayInfoCallback(JSON.parse(data)));
  }


  notifyUV(uvData)
  {
    RequestUtils.put(`/setChannelUV/${this.screenWidget.currentChannel.channelId}`, JSON.stringify(uvData), (jsonCheckedUVs) => {
      let checkedUVs = JSON.parse(jsonCheckedUVs);
      this.activeChannels[this.screenWidget.currentChannel.channelId].uvs = checkedUVs;
      this.screenWidget.uvCallback(checkedUVs);
    });
  }


  updateGammaFactor(gammaFactor)
  {
    if(!this.screenWidget.currentChannel){
      return;
    }

    if(gammaFactor == this.screenWidget.currentChannel.gammaFactor){
      return;
    }

    this.screenWidget.currentChannel.gammaFactor = gammaFactor;

    RequestUtils.put(`/setChannelGammaFactor/${this.screenWidget.currentChannel.channelId}`, JSON.stringify({gammaFactor : gammaFactor}), (jsonGammaFactorData) => {
      let gammaFactorData = JSON.parse(jsonGammaFactorData);
      let gammaFactor = gammaFactorData.gammaFactor;
      this.screenWidget.currentChannel.gammaFactor = Utils.truncate(gammaFactor, 2);
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


  _setChannelActivity(channelId, active)
  {
    RequestUtils.post("/setChannelActivity", JSON.stringify({channelId : channelId, active : active}), (jsonData) => {
      let data = JSON.parse(jsonData);
      this._refreshChannelsLists(data.channels);
      
      if("newActiveChannelId" in data){
        this._manageChannel(data.newActiveChannelId);
      }
    });
  }


  _refreshChannelsLists(channels)
  {
    const activeChannels = channels.filter(channel => channel.active);
    const inactiveChannels = channels.filter(channel => !channel.active);

    this.activeChannels = {};
    this._refreshActiveChannels(activeChannels);
    this._refreshInactiveChannels(inactiveChannels);

    if(channels.length == 0){
      this.screenWidget.setLegend(ScreenWidget.Legends.noChannel);
    }
    else if(Object.keys(this.activeChannels).length == 0){
      this.screenWidget.setLegend(ScreenWidget.Legends.pleaseDrag);
      this.screenWidget.showWidgets(false);
    }
    else{
      this.screenWidget.setLegend(ScreenWidget.Legends.pleaseSelect);
    }
    
    this.screenWidget.showPreview();
  }


  _refreshActiveChannels(channels)
  {
    this.activeChannelsNode.innerHTML = "";
    for(let channelData of channels){
      let newChannelEntryNode = document.createElement("p");
      newChannelEntryNode.draggable = true;
      newChannelEntryNode.selected = false;

      let newChannel = new Channel(channelData, newChannelEntryNode)
      newChannelEntryNode.addEventListener("dragstart", (event) => {
        event.dataTransfer.setData("channelData", JSON.stringify(newChannel));
      });

      newChannelEntryNode.addEventListener("click", (event) => {
        this._manageChannel(channelData.channelId);
      });

      //newChannelEntryNode.innerHTML = `${newChannel.name} - ${newChannel.productName}`;
      newChannelEntryNode.innerHTML = newChannel.channelId; // Todo : get lights names

      this.activeChannelsNode.appendChild(newChannelEntryNode);
      this.activeChannels[newChannel.channelId] = newChannel;
    }
  }


  _setItemSelected(channelNode, selected)
  {
    channelNode.selected = selected;

    if(channelNode.selected){
      channelNode.classList.add("selected");
    }
    else{
      channelNode.classList.remove("selected");
    }
  }


  _toggleClicked(channelNode)
  {
    this._setItemSelected(channelNode, !channelNode.selected);
  }


  _refreshInactiveChannels(channels)
  {
    this.inactiveChannelsNode.innerHTML = "";

    for(let channelData of channels){
      let newChannelEntryNode = document.createElement("p");
      newChannelEntryNode.draggable = true;

      let newChannel = new Channel(channelData)
      newChannelEntryNode.addEventListener("dragstart", (event) => {
        event.dataTransfer.setData("channelData", JSON.stringify(newChannel));
      });

      //newChannelEntryNode.innerHTML = `${newChannel.name} - ${newChannel.productName}`;
      newChannelEntryNode.innerHTML = newChannel.channelId; // Todo : Get lights names

      this.inactiveChannelsNode.appendChild(newChannelEntryNode);
    }
  }


  _displayInfoCallback(displayInfo)
  {
    let x = displayInfo.x;
    let y = displayInfo.y;
    let subsampleWidth = displayInfo.subsampleWidth;
    this.refreshRateInputNode.value = displayInfo.selectedRefreshRate;

    this.screenWidget.setDimensions(x, y, subsampleWidth);
    this.screenWidget.setSubsampleCandidates(displayInfo.subsampleResolutionCandidates);

    this._initAdvancedSettings();
    this.availableSubsamplesNode.value = subsampleWidth.toString();
  }


  _manageChannel(channelId)
  {
    for(let loopChannelId of Object.keys(this.activeChannels)){
      let channelNode = this.activeChannels[loopChannelId].node;
      if(loopChannelId != channelId){
        this._setItemSelected(channelNode, false);
      }
      else{
        this._toggleClicked(channelNode);
      }
    }

    if(!this.activeChannels[channelId].node.selected){
      this.screenWidget.setLegend(ScreenWidget.Legends.pleaseSelect);
      this.screenWidget.showWidgets(false);
      this.screenWidget.showPreview();
      this.screenWidget.currentChannel = null;

      return;
    }
    else{
      this.screenWidget.showPreview(channelId);
    }

    this.screenWidget.setLegend(ScreenWidget.Legends.none);

    RequestUtils.get(`/channel/${channelId}`, (jsonChannel) => {
      let channelData = JSON.parse(jsonChannel);
      this.activeChannels[channelId].uvs = channelData.uvs;
      this.screenWidget.initChannelRegion(this.activeChannels[channelId]);
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


  _setRefreshRateCallback(refreshRate)
  {
    this.refreshRateInputNode.value = refreshRate;
  }


  _saveProfile()
  {
    RequestUtils.post("/saveProfile", JSON.stringify(null), (data) => {log("Saved profile");});
  }


  _askStopConfirmation()
  {
    this.overlay.style.display = "block";
    document.getElementById("confirmStopSection").style.display = "block";
  }


  _stop()
  {
    RequestUtils.post("/stop", JSON.stringify(null), (jsonData) => {
      let data = JSON.parse(jsonData);
      if(data.succeeded){
        document.getElementById("confirmStopSection").style.display = "none";
        document.getElementById("stoppedInfoSection").style.display = "block";
      }
    });
  }
}
