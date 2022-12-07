class Utils
{
  static clamp(value, min, max)
  {
    return Math.max(min, Math.min(value, max));
  }


  static truncate(value, decimals)
  {
    let exp = Math.pow(10, decimals);
    return Math.round(value * exp) / exp;
  }
}


class Handle
{
  static Type =
  {
    TopLeft : 0,
    TopRight : 1,
    BottomLeft : 2,
    BottomRight : 3
  };


  static MapHandleTypeId =
  {
    "tl" : Handle.Type.TopLeft,
    "tr" : Handle.Type.TopRight,
    "bl" : Handle.Type.BottomLeft,
    "br" : Handle.Type.BottomRight
  };


  constructor(handleId, screenWidget)
  {
    this.handleNode = document.getElementById(handleId);
    this.screenWidget = screenWidget;
    this.handleNode.addEventListener("mousedown", () => {this.drag();});
    this.type = Handle.MapHandleTypeId[handleId];
  }


  drag()
  {
    this.screenWidget.draggedHandle = this;
  }


  drop()
  {
    log("Dropped");
  }


  setPosition(position, notify = false)
  {
    let screenDimensions = this.screenWidget.screenDimensions();
    let maxX = screenDimensions.x;
    let maxY = screenDimensions.y;
    let minX = 0;
    let minY = 0;

    let x = Utils.clamp(position.x, minX, maxX);
    let y = Utils.clamp(position.y, minY, maxY);

    this.handleNode.setAttribute("cx", x);
    this.handleNode.setAttribute("cy", y);

    let xRange = (maxX - minX);
    let xNorm = x / xRange;
    let yRange = (maxY - minY);
    let yNorm = y / yRange;

    let uvData = {
      x : xNorm,
      y : yNorm,
      type : this.type
    };

    if(notify){
      this.screenWidget._updateUVs(uvData);
    }
  }
}


class ScreenWidget
{
  static Legends = {
    none : "",
    noLight : "There are currently no available lights. Please register them through official application.",
    pleaseDrag : "Drag and and drop light from 'available' to 'synced' box to manage it.",
    pleaseSelect : "Select a synced light to manage"
  };

  constructor(svgAreaId, webApp)
  {
    this.svgAreaNode = document.getElementById(svgAreaId);
    this.screenAreaNode = document.getElementById("screenArea");
    this.uvAreaNode = document.getElementById("uvArea");
    this.svgLightNameNode = document.getElementById("svgLightName");
    this.svgLightUVSizeNode = document.getElementById("svgLightUVSize");

    this.legendText = document.getElementById("legendText");

    this.webApp = webApp;

    let handleIds = ["tl", "tr", "bl", "br"];
    this.handles = {}

    for(let handleId of handleIds){
      this.handles[handleId] = new Handle(handleId, this);
    };

    document.addEventListener("mouseup", () => {this.drop();})
    this.svgAreaNode.addEventListener("mousemove", (event) => {this._updateMousePosition(event);});
    this.svgAreaNode.addEventListener("mouseup", () => {this.drop();});

    this.draggedHandle = null;
    this.currentLight = null;
  }


  screenDimensions()
  {
    let bbox = this.screenAreaNode.getBoundingClientRect();
    return {x : bbox.width, y : bbox.height};
  }


  initLightRegion(light)
  {
    this.currentLight = light;
    this._updateShape(light.uvs);
    this.showWidgets(true);
    this.svgLightNameNode.innerHTML = light.name;
  }


  setLegend(legendFlag){
    this.legendText.innerHTML = legendFlag;
  }

  showWidgets(show)
  {
    let display = show ? "block" : "none";
    this.uvAreaNode.style.display = display;
    for(let [key, handle] of Object.entries(this.handles)){
      handle.handleNode.style.display = display;
    }
  }


  showPreview(show)
  {

  }


  drop()
  {
    if(this.draggedHandle){
      this.draggedHandle.drop();
      this.draggedHandle = null;
    }
  }


  uvCallback(uvs)
  {
    this._updateShape(uvs);
  }


  _updateShape(uvs)
  {
    let screenDimensions = this.screenDimensions();

    let ax = uvs.uvA.x * screenDimensions.x;
    let ay = uvs.uvA.y * screenDimensions.y;
    let bx = uvs.uvB.x * screenDimensions.x;
    let by = uvs.uvB.y * screenDimensions.y;

    this.handles["tl"].setPosition({x : ax, y : ay});
    this.handles["tr"].setPosition({x : bx, y : ay});
    this.handles["bl"].setPosition({x : ax, y : by});
    this.handles["br"].setPosition({x : bx, y : by});

    this.uvAreaNode.setAttribute("x", ax);
    this.uvAreaNode.setAttribute("y", ay);
    this.uvAreaNode.setAttribute("width", bx - ax);
    this.uvAreaNode.setAttribute("height", by - ay);
    

    let propWidth = Utils.truncate((uvs.uvB.x - uvs.uvA.x) * 100, 2);
    let propHeight = Utils.truncate((uvs.uvB.y - uvs.uvA.y) * 100, 2);


    this.svgLightUVSizeNode.innerHTML = `${propWidth}% x ${propHeight}%`;
  }


  _updateMousePosition(event)
  {
    if(this.draggedHandle){
      var ctm = this.screenAreaNode.getScreenCTM();

      let point = {
        x: (event.clientX - ctm.e) / ctm.a,
        y: (event.clientY - ctm.f) / ctm.d
      }

      this.draggedHandle.setPosition(point, true);
    }
  }


  _updateUVs(uvData)
  {
    this.webApp.notifyUV(uvData);
  }
}
