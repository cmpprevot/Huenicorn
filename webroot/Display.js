class Utils
{
  static clamp(value, min, max)
  {
    return Math.max(min, Math.min(value, max));
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


  constructor(handleId, controller)
  {
    this.handleNode = document.getElementById(handleId);
    this.controller = controller;
    this.handleNode.addEventListener("mousedown", () => {this.drag();});
    this.type = Handle.MapHandleTypeId[handleId];
  }


  drag()
  {
    this.controller.draggedHandle = this;
  }


  drop()
  {
    log("Dropped");
  }


  setPosition(position, notify = false)
  {
    let screenDimensions = this.controller.screenDimensions();
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
      this.controller._updateUVs(uvData);
    }
  }
}


class Controller
{
  constructor(svgAreaId, webApp)
  {
    this.svgAreaNode = document.getElementById(svgAreaId);
    this.screenAreaNode = document.getElementById("screenArea");
    this.uvAreaNode = document.getElementById("uvArea");

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
    this.showWidgets();
  }


  showWidgets()
  {
    this.uvAreaNode.style.display = "block";
    for(let [key, handle] of Object.entries(this.handles)){
      log(handle)
      handle.handleNode.style.display = "block";
    }
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
