log = console.log;


class SetupUI
{
  constructor()
  {
    this.setupSection = document.getElementById("setupSection");
    this.stepNodes = document.getElementsByClassName("step");
    this.errorMessages = document.getElementsByClassName("errorMessages");

    this.setConfigFilePath();

    this.start();
  }


  setConfigFilePath()
  {
    RequestUtils.get("/configFilePath", (jsonData) => {
      let data = JSON.parse(jsonData);
      let configFileLink = document.getElementById("configFilePath");
      configFileLink.href = `file://${data.configFilePath}`;
      configFileLink.innerHTML = data.configFilePath;
    })
  }


  start()
  {
    this.stepId = 0;
    this._loadStep(this.stepId);
  }


  incStep()
  {
    this._loadStep(this.stepId + 1);
  }


  abort()
  {
    this._exit();
  }


  // Bridge address step
  autoDetectBridge()
  {
    RequestUtils.get("/autoDetectBridge", this.autoDetectBridgeCallback);
  }


  autoDetectBridgeCallback(jsonData)
  {
    let data = JSON.parse(jsonData);

    if(data.succeeded){
      document.getElementById("bridgeAddress").value = data.bridgeAddress;
    }
    else{
      let errorMessageNode = document.getElementById("bridgeAddressErrorMessage");
      errorMessageNode.style.visibility = "visible";
      errorMessageNode.innerHTML = data.error;
    }
  }


  validateAddress()
  {
    let bridgeAddress = document.getElementById("bridgeAddress").value;

    if(bridgeAddress != ""){
      RequestUtils.put("/validateBridgeAddress", JSON.stringify({bridgeAddress : bridgeAddress}), (data) => {this.validateBridgeAddressCallback(data);});
    }
    else{
      let errorMessageNode = document.getElementById("bridgeAddressErrorMessage");
      errorMessageNode.style.visibility = "visible";
      errorMessageNode.innerHTML = "Please provide a valid address";
    }
  }


  validateBridgeAddressCallback(jsonData)
  {
    let data = JSON.parse(jsonData);

    if(data.succeeded){
      this.incStep();
    }
    else{
      let errorMessageNode = document.getElementById("bridgeAddressErrorMessage");
      errorMessageNode.style.visibility = "visible";
      errorMessageNode.innerHTML = "Could not resolve Hue bridge";
    }
  }


  // Credentials validation step
  validateCredentials()
  {
    let username = document.getElementById("username").value;
    let clientkey = document.getElementById("clientkey").value;

    if(username != "" && clientkey != ""){
      RequestUtils.put("/validateCredentials", JSON.stringify({username : username, clientkey : clientkey}), (data) => {this.validateCredentialsCallback(data);});
    }
    else{
      let errorMessageNode = document.getElementById("userErrorMessage");
      errorMessageNode.style.visibility = "visible";
      errorMessageNode.innerHTML = "Please provide valid credentials or generate a new one";
    }
  }


  validateCredentialsCallback(jsonData)
  {
    let data = JSON.parse(jsonData);

    if(data.succeeded){
      this._finish();
    }
    else{
      let errorMessageNode = document.getElementById("credentialsErrorMessage");
      errorMessageNode.style.visibility = "visible";
      errorMessageNode.innerHTML = "The provided key was denied";
    }
  }


  // Credentials generation step
  registerNewUser()
  {
    RequestUtils.put("/registerNewUser", null, (data) => {this.registerNewUserCallback(data);});
  }


  registerNewUserCallback(jsonData)
  {
    let data = JSON.parse(jsonData);

    if(data.succeeded){
      this._finish();
    }
    else{
      let errorMessageNode = document.getElementById("newUserErrorMessage");
      errorMessageNode.style.visibility = "visible";
      errorMessageNode.innerHTML = "Please press the central button on the bridge and try again";
    }
  }


  // Private methods
  _loadStep(stepId)
  {
    this.stepId = stepId;
    let i = 0;
    for(let stepNode of this.stepNodes){
      let display = (i++ == this.stepId) ? "block" : "none";
      stepNode.style.display = display;
    }

    for(let errorMessageNode of this.errorMessages){
      errorMessageNode.style.visibility = "hidden";
      errorMessageNode.innerHTML = "";
    }
  }

  _finish()
  {
    RequestUtils.post("/finishSetup", JSON.stringify(null), (data) => {log("Finished");});

    for(let stepNode of this.stepNodes){
      stepNode.style.display = "none";
    }

    document.getElementById("finishSection").style.display = "block";

    this.pingFunctionInterval = setInterval(
      this._ping
      , 500);
  }


  _exit()
  {
    RequestUtils.post("/abort", JSON.stringify(null), (data) => {});

    for(let stepNode of this.stepNodes){
      stepNode.style.display = "none";
    }

    document.getElementById("abortSection").style.display = "block";
  }


  _ping()
  {
    RequestUtils.get("/", () => {
      document.getElementById("refreshSection").style.display = "block";
      clearInterval(setupUI.pingFunctionInterval);
    });
  }
}

let setupUI = new SetupUI();
