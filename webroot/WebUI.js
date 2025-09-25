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
    
    this.entertainmentConfigurationsNode = document.getElementById("entertainmentConfigurations");
    this.entertainmentConfigurationsSelectNode = document.getElementById("entertainmentConfigurationsSelect");
    this.entertainmentConfigurationsSelectNode.addEventListener("change", (event) => {this._setEntertainmentConfiguration(event.target.value);});

    this.advancedSettingsCheckbox = document.getElementById("advancedSettingsCheckbox");
    this.advancedSettingsCheckbox.addEventListener("click", (e) => {this._toggleAdvancedDisplay(e.target.checked);});
    this.advancedSettingsNode = document.getElementById("advancedSettings");
    this.availableSubsamplesNode = document.getElementById("availableSubsampleWidths");
    this.availableSubsamplesNode.addEventListener("change", (event) => {this._setSubsampleWidth(parseInt(event.target.value));});
    this.availableInterpolationsNode = document.getElementById("availableInterpolations");
    this.availableInterpolationsNode.addEventListener("change", (event) => {this._setInterpolation(parseInt(event.target.value));});
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
    let channelsPromise = RequestUtils.get("/api/channels");
    channelsPromise.then((data) => {this._refreshChannelsLists(data);});
    channelsPromise.catch((error) => {log(error);});

    let displayInfoPromise = RequestUtils.get("/api/displayInfo");
    displayInfoPromise.then((data) => {this._displayInfoCallback(data);});
    displayInfoPromise.catch((error) => {log(error);});

    let interpolationInfoPromise = RequestUtils.get("/api/interpolationInfo");
    interpolationInfoPromise.then((data) => {this._interpolationInfoCallback(data);});
    interpolationInfoPromise.catch((error) => {log(error);});

    let entertainmentConfigurationsPromise = RequestUtils.get("/api/entertainmentConfigurations");
    entertainmentConfigurationsPromise.then((data) => {this._entertainmentConfigurationsCallback(data);});
    entertainmentConfigurationsPromise.catch((error) => {log(error);});
  }


  notifyUV(uvData)
  {
    let uvPromise = RequestUtils.put(`/api/setChannelUV/${this.screenWidget.currentChannel.channelId}`, JSON.stringify(uvData));
    uvPromise.then((checkedUVs) => {
      this.activeChannels[this.screenWidget.currentChannel.channelId].uvs = checkedUVs;
      this.screenWidget.uvCallback(checkedUVs);
    });
    uvPromise.catch((error) => {log(error);});
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

    let promise = RequestUtils.put(`/api/setChannelGammaFactor/${this.screenWidget.currentChannel.channelId}`, JSON.stringify({gammaFactor : gammaFactor}));
    promise.then((gammaFactorData) => {
      let gammaFactor = gammaFactorData.gammaFactor;
      this.screenWidget.currentChannel.gammaFactor = Utils.truncate(gammaFactor, 2);
    });
    promise.catch((error) => {log(error);});
  }


  _initAdvancedSettings()
  {
    let showAdvancedSettings = false;
    this.advancedSettingsCheckbox.checked = showAdvancedSettings;
    this._toggleAdvancedDisplay(showAdvancedSettings);
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
    let promise = RequestUtils.post(`/api/setChannelActivity/${channelId}`, JSON.stringify({active : active}));
    promise.then((data) => {
      this._refreshChannelsLists(data.channels);
      
      if("newActiveChannelId" in data){
        this._manageChannel(data.newActiveChannelId);
      }
      else{
        this.screenWidget.showWidgets(false);
      }
    });
    promise.catch((error) => {log(error);});
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

      newChannelEntryNode.innerHTML = newChannel.formatMembers();

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

      newChannelEntryNode.innerHTML = newChannel.formatMembers();

      this.inactiveChannelsNode.appendChild(newChannelEntryNode);
    }
  }


  _displayInfoCallback(displayInfo)
  {
    let x = displayInfo.x;
    let y = displayInfo.y;
    let subsampleWidth = displayInfo.subsampleWidth;
    this.refreshRateInputNode.value = displayInfo.selectedRefreshRate;
    this.refreshRateInputNode.max = displayInfo.maxRefreshRate;

    this.screenWidget.setDimensions(x, y, subsampleWidth);
    let subsampleResolutionCandidates = displayInfo.subsampleResolutionCandidates;

    this.availableSubsamplesNode.innerHTML = "";
    for(let subsampleCandidate of subsampleResolutionCandidates.reverse()){
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

    this._initAdvancedSettings();
    this.availableSubsamplesNode.value = subsampleWidth.toString();
  }


  _interpolationInfoCallback(interpolationInfo)
  {
    let availableInterpolations = interpolationInfo.available;

    this.availableInterpolationsNode.innerHTML = "";

    for(let availableInterpolation of availableInterpolations){
      let newOption = document.createElement("option");

      let key = Object.keys(availableInterpolation)[0];
      let value = Object.values(availableInterpolation)[0];

      newOption.innerHTML = key;
      newOption.value = value;

      this.availableInterpolationsNode.appendChild(newOption);
    }

    this.availableInterpolationsNode.value = interpolationInfo.current;
  }

  _entertainmentConfigurationsCallback(entertainmentConfigurationsData)
  {
    let entertainmentConfigurations = entertainmentConfigurationsData.entertainmentConfigurations;
    let currentEntertainmentConfigurationId = entertainmentConfigurationsData.currentEntertainmentConfigurationId;

    if(entertainmentConfigurations.length > 1){
      this.entertainmentConfigurationsNode.style.display = "block";
    }

    for(let entertainmentConfiguration of entertainmentConfigurations){
      let newOption = document.createElement("option");
      newOption.innerHTML = entertainmentConfiguration.name;
      newOption.value = entertainmentConfiguration.entertainmentConfigurationId;
      this.entertainmentConfigurationsSelectNode.appendChild(newOption);
    }

    this.entertainmentConfigurationsSelectNode.value = currentEntertainmentConfigurationId;
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

    let channelPromise = RequestUtils.get(`/api/channel/${channelId}`);
    channelPromise.then((channelData) => {
      this.activeChannels[channelId].uvs = channelData.uvs;
      this.screenWidget.initChannelRegion(this.activeChannels[channelId]);
    });
    channelPromise.catch((error) => {log(error);});
  }


  _setEntertainmentConfiguration(entertainmentConfigurationId)
  {
    this._showLoading(true, "Switching configuration...");
    let promise = RequestUtils.put("/api/setEntertainmentConfiguration", JSON.stringify(entertainmentConfigurationId));

    promise.then((entertainmentConfigurationData) => {
      this._refreshChannelsLists(entertainmentConfigurationData.channels)
      this.screenWidget.showPreview();
      this.screenWidget.showWidgets(false);
      this._showLoading(false);
    });
    promise.catch((error) => {
      log(error);
      this._showLoading(false);
    });
  }


  _setSubsampleWidth(subsampleWidth)
  {
    let promise = RequestUtils.put("/api/setSubsampleWidth", JSON.stringify(subsampleWidth));
    promise.then((displayInfo) => {
      this.screenWidget.setDimensions(displayInfo.x, displayInfo.y, displayInfo.subsampleWidth);

      let proportion = subsampleWidth / this.screenWidget.width;

      let gradientColor = StyleUtils.greenRedGradient(proportion);
      this.availableSubsamplesNode.style.color = gradientColor;
    });
    promise.catch((error) => {log(error);});
  }


  _setInterpolation(interpolation)
  {
    let promise = RequestUtils.put("/api/setInterpolation", JSON.stringify(interpolation));
    promise.catch((error) => {log(error);});
  }


  _setRefreshRate(refreshRate){
    let promise = RequestUtils.put("/api/setRefreshRate", JSON.stringify(refreshRate));
    promise.then((data) => {this._setRefreshRateCallback(data.refreshRate);});
    promise.catch((error) => {log(error);});
  }


  _setRefreshRateCallback(refreshRate)
  {
    this.refreshRateInputNode.value = refreshRate;
  }


  _saveProfile()
  {
    RequestUtils.post("/api/saveProfile", JSON.stringify(null), (data) => {log("Saved profile");});
  }


  _askStopConfirmation()
  {
    this.overlay.style.display = "block";
    document.getElementById("confirmStopSection").style.display = "block";
  }


  _stop()
  {
    let promise = RequestUtils.post("/api/stop", JSON.stringify(null));
    promise.then((data) => {
      if(data.succeeded){
        document.getElementById("confirmStopSection").style.display = "none";
        document.getElementById("stoppedInfoSection").style.display = "block";
      }
    });
  }


  _showLoading(show, loadingText = null)
  {
    let loadingInfoSectionNode = document.getElementById("loadingInfoSection");

    if(show){
      this.overlay.style.display = "block";
      loadingInfoSectionNode.style.display = "block";
      if(loadingText !== null){
        document.getElementById("loadingTextInfo").innerHTML = loadingText;
      }
    }
    else{
      this.overlay.style.display = "none";
      loadingInfoSectionNode.style.display = "none";
    }
  }
}
