class Utils
{
  static clamp(value, min, max)
  {
    return Math.max(min, Math.min(value, max));
  }


  static truncate(value, decimals)
  {
    let exp = Math.pow(10, decimals);
    return Math.round((value) * exp) / exp;
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


  constructor(handleId, screenWidget, parentNode)
  {
    this.handleNode = document.getElementById(handleId);
    this.screenWidget = screenWidget;
    this.parentNode = parentNode;
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

    x = Math.round(x * (this.screenWidget.subsampleWidth / screenDimensions.x));
    y = Math.round(y * (this.screenWidget.subsampleHeight / screenDimensions.y));

    let xNorm = x / (this.screenWidget.subsampleWidth);
    let yNorm = y / (this.screenWidget.subsampleHeight);

    this.handleNode.setAttribute("cx", `${xNorm * 100}%`);
    this.handleNode.setAttribute("cy", `${yNorm * 100}%`);

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


class GammaHandle
{
  constructor(screenWidget, parentNode)
  {
    this.screenWidget = screenWidget;
    this.parentNode = parentNode;
    this.handleNode = document.getElementById("gammaCursor");
    this.handleNode.addEventListener("mousedown", () => {this.drag();});
  }


  drag()
  {
    this.screenWidget.draggedHandle = this;
  }


  affectFactor(factor)
  {
    factor += 1;
    factor /= 2
    factor *= this.screenWidget.gammaAreaDimensions().x;

    this.setPosition({x : factor}, false);
  }


  setPosition(position, notify = false)
  {
    let areaDimensions = this.screenWidget.gammaAreaDimensions();
    let gammaFactor =  Utils.clamp(position.x / areaDimensions.x, 0, 1);

    gammaFactor = Utils.truncate(gammaFactor, 1)

    this.handleNode.setAttribute("cx", `${gammaFactor * 100}%`);

    gammaFactor *= 2;
    gammaFactor -= 1;
    gammaFactor = Utils.truncate(gammaFactor, 1)

    this.screenWidget.webApp.updateGammaFactor(gammaFactor);
  }


  drop()
  {
    log("Dropped");
  }
}


class Rectangle
{
  constructor(rectangleNode)
  {
    this.svgNode = rectangleNode;
  }


  setGeometry(x, y, width, height)
  {
    this.svgNode.setAttribute("x", x);
    this.svgNode.setAttribute("y", y);
    this.svgNode.setAttribute("width", width);
    this.svgNode.setAttribute("height", height);
  }


  show(show = true)
  {
    let display = show ? "block" : "none";
    this.svgNode.style.display = display;
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


  constructor(webApp)
  {
    this.webApp = webApp;
    this.svgAreaNode = document.getElementById("svgArea");
    this.screenAreaNode = document.getElementById("screenArea");
    this.gammaAreaNode = document.getElementById("gammaArea");
    this.uvRectangle = new Rectangle(document.getElementById("uvArea"));
    this.svgLightNameNode = document.getElementById("svgLightName");
    this.svgLightUVSizeNode = document.getElementById("svgLightUVSize");
    this.previewArea = document.getElementById("previewRectangles");
    this.legendText = document.getElementById("legendText");
    this.gridAreaNode = document.getElementById("gridArea");

    this.gammaCursor = new GammaHandle(this, this.gammaAreaNode);

    let handleIds = ["tl", "tr", "bl", "br"];
    this.handles = {}

    for(let handleId of handleIds){
      this.handles[handleId] = new Handle(handleId, this, this.screenAreaNode);
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


  gammaAreaDimensions()
  {
    let bbox = this.gammaAreaNode.getBoundingClientRect();
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


  setDimensions(width, height, subsampleWidth)
  {
    this.subsampleWidth = subsampleWidth;
    this.subsampleHeight = height / width * subsampleWidth;

    for(let i = 0; i < this.subsampleWidth; i++){
      let x = i / this.subsampleWidth;
      var line = document.createElementNS("http://www.w3.org/2000/svg", "line");

      line.setAttribute("x1", `${x * 100}%`);
      line.setAttribute("x2", `${x * 100}%`);
      line.setAttribute("y1", `0%`);
      line.setAttribute("y2", `100%`);
      line.setAttribute("stroke", "var(--gridColor)");
      line.setAttribute("stroke-width", "1");

      this.gridAreaNode.appendChild(line);
    }

    for(let i = 0; i < this.subsampleHeight; i++){
      let y = i / this.subsampleHeight;
      var line = document.createElementNS("http://www.w3.org/2000/svg", "line");

      line.setAttribute("x1", `0%`);
      line.setAttribute("x2", `100%`);
      line.setAttribute("y1", `${y * 100}%`);
      line.setAttribute("y2", `${y * 100}%`);
      line.setAttribute("stroke", "var(--gridColor)");
      line.setAttribute("stroke-width", "1");

      this.gridAreaNode.appendChild(line);
    }
  }


  showWidgets(show)
  {
    let display = show ? "block" : "none";
    this.uvRectangle.show(show);
    for(let [key, handle] of Object.entries(this.handles)){
      handle.handleNode.style.display = display;
    }

    this.gammaAreaNode.style.display = display;
    if(show){
      this.gammaCursor.affectFactor(this.currentLight.gammaFactor);
    }
  }


  showPreview(exceptedLightId = null)
  {
    this.previewArea.innerHTML = "";

    let syncedLights = this.webApp.syncedLights;
    let screenDimensions = this.screenDimensions();

    if(!syncedLights){
      return;
    }

    for(let syncedLightId of Object.keys(syncedLights)){
      if(syncedLightId == exceptedLightId){
        continue;
      }

      let syncedLight = syncedLights[syncedLightId];

      var rectNode = document.createElementNS("http://www.w3.org/2000/svg", "rect");
      var textNode = document.createElementNS("http://www.w3.org/2000/svg", "text");

      textNode.innerHTML = syncedLight.name;
      textNode.setAttribute("fill", "rgba(255, 255, 255, 0.2)");
      textNode.setAttribute("text-anchor", "middle");

      let uvs = syncedLight.uvs;
      let ax = uvs.uvA.x * screenDimensions.x;
      let ay = uvs.uvA.y * screenDimensions.y;
      let bx = uvs.uvB.x * screenDimensions.x;
      let by = uvs.uvB.y * screenDimensions.y;
      let width = bx - ax;
      let height = by - ay;

      textNode.setAttribute("x", width / 2 + ax);
      textNode.setAttribute("y", height / 2 + ay);

      rectNode.style.stroke = "rgba(255, 255, 255, 0.2)";
      rectNode.style.strokeWidth = "1px";

      let previewRectangle = new Rectangle(rectNode);

      previewRectangle.setGeometry(ax, ay, width, height);

      this.previewArea.appendChild(rectNode);
      this.previewArea.appendChild(textNode);
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

    this.uvRectangle.setGeometry(ax, ay, bx - ax, by - ay);

    let propWidth = Utils.truncate((uvs.uvB.x - uvs.uvA.x) * 100, 2);
    let propHeight = Utils.truncate((uvs.uvB.y - uvs.uvA.y) * 100, 2);

    this.svgLightUVSizeNode.innerHTML = `${propWidth}% x ${propHeight}%`;
  }


  _updateMousePosition(event)
  {
    if(this.draggedHandle){
      var ctm = this.draggedHandle.parentNode.getScreenCTM();

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
