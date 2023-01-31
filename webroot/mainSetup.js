log = console.log;


class SetupUI
{
  constructor()
  {
    this.setupSection = document.getElementById("setupSection");
    this.stepNodes = document.getElementsByClassName("step");

    this.stepId = 0;
    this.start();
  }


  start()
  {
    this._loadStep(0);
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
  }


  validateAddress()
  {
    let bridgeAddress = document.getElementById("bridgeAddress").value;

    if(bridgeAddress != ""){
      RequestUtils.put("/validateBridgeAddress", JSON.stringify({bridgeAddress : bridgeAddress}), (data) => {this.validateBridgeAddressCallback(data);});
    }
  }


  validateBridgeAddressCallback(jsonData)
  {
    let data = JSON.parse(jsonData);

    if(data.succeeded){
      this.incStep();
    }
  }


  // API key step
  validateApiKey()
  {
    let apiKey = document.getElementById("apiKey").value;

    if(apiKey != ""){
      RequestUtils.put("/validateApiKey", JSON.stringify({apiKey: apiKey}), (data) => {this.validateApiKeyCallback(data);});
    }
  }


  validateApiKeyCallback(jsonData)
  {
    let data = JSON.parse(jsonData);

    if(data.succeeded){
      this._finish();
    }
  }


  // API key generation step
  requestNewApiKey()
  {
    RequestUtils.put("/requestNewApiKey", null, (data) => {this.requestNewApiKeyCallback(data);});
  }


  requestNewApiKeyCallback(jsonData)
  {
    let data = JSON.parse(jsonData);

    if(data.succeeded){
      this._finish();
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
    });
  }
}

let setupUI = new SetupUI();
