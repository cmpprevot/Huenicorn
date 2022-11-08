const BoundaryType =
{
  Min: 0,
  Max: 1,
  Center: 2
};


class Handle
{
  constructor(owner, x, y, boundaryType)
  {
    owner.handles.push(this);
    this.x = x;
    this.y = y;
    this.boundaryType = boundaryType;

    this.radius = 10;
    this._hovered = false;
    this.xSibling = null;
    this.ySibling = null;
    this.equidistantSiblings = [];
    this.middleSibling = null;
  }

  setPosition(x, y)
  {
    this.x = x;
    this.y = y;

    this._affectSiblings();
  }


  hovered(x, y)
  {
    let distX = Math.abs(x - this.x);
    let distY = Math.abs(y - this.y);

    this._hovered = (distX * distX + distY * distY) < this.radius * this.radius;
    return this._hovered;
  }


  draw(context)
  {
    context.fillStyle = this._hovered ? "#0088ff" : "#446688";
    context.strokeStyle = "#0088ffff";

    context.beginPath();
    context.arc(this.x, this.y, this.radius, 0, 2 * Math.PI);
    context.fill();
    context.stroke();
  }


  _affectSiblings()
  {
    function average(a, b){
      return (a + b) / 2;
    }

    if(this.xSibling){
      this.xSibling.x = this.x;
      
      if(this.boundaryType[1] == BoundaryType.Min){
        if(this.y > this.xSibling.y){
          this.xSibling.setPosition(this.xSibling.x, this.y);
        }
      }
      else if(this.boundaryType[1] == BoundaryType.Max){
        if(this.y < this.xSibling.y){
          this.xSibling.setPosition(this.xSibling.x, this.y);
        }
      }
    }

    if(this.ySibling){
      this.ySibling.y = this.y;

      if(this.boundaryType[0] == BoundaryType.Min){
        if(this.x > this.ySibling.x){
          this.ySibling.setPosition(this.x, this.ySibling.y);
        }
      }
      else if(this.boundaryType[0] == BoundaryType.Max){
        if(this.x < this.ySibling.x){
          this.ySibling.setPosition(this.x, this.ySibling.y);
        }
      }
    }

    if( this.boundaryType[0] == BoundaryType.Center &&
        this.boundaryType[0] == BoundaryType.Center){

      let xMin = null;
      let yMin = null;
      let xMax = null;
      let yMax = null;

      for(let sibling of this.equidistantSiblings){
        if(xMin == null && sibling.boundaryType[0] == BoundaryType.Min){
          xMin = sibling.x;
        }
        if(xMax == null && sibling.boundaryType[0] == BoundaryType.Max){
          xMax = sibling.x;
        }
        if(yMin == null && sibling.boundaryType[1] == BoundaryType.Min){
          yMin = sibling.y;
        }
        if(yMax == null && sibling.boundaryType[1] == BoundaryType.Max){
          yMax = sibling.y;
        }
      }

      let xDelta = this.x - average(xMin, xMax);
      let yDelta = this.y - average(yMin, yMax);

      // Todo : check boundaries
      for(let sibling of this.equidistantSiblings){
        continue;
      }

      // Apply
      for(let sibling of this.equidistantSiblings){
        sibling.x += xDelta;
        sibling.y += yDelta;
      }
    }

    if(this.middleSibling){
      this.middleSibling.x = average(this.x, this.ySibling.x);
      this.middleSibling.y = average(this.y, this.xSibling.y);
    }
  }
}


class Rectangle
{
  constructor(vertexA, vertexB)
  {
    this.vertexA = vertexA;
    this.vertexB = vertexB;
  }


  draw(context)
  {
    context.fillStyle = "#ffffff80";
    context.strokeStyle = "#ffffffff";

    let width = this.vertexB.x - this.vertexA.x;
    let height = this.vertexB.y - this.vertexA.y;

    context.strokeRect(this.vertexA.x, this.vertexA.y, width, height);
    context.fillRect(this.vertexA.x, this.vertexA.y, width, height);
  }
}


class PortionWidget
{
  constructor(uvs, canvas)
  {
    this.handles = [];
    this.boundaries = {
      xMin : 0,
      yMin : 0,
      xMax : canvas.width,
      yMax : canvas.height
    };

    let x1 = uvs.uvA.x * canvas.width;
    let y1 = uvs.uvA.y * canvas.height;

    let x2 = uvs.uvB.x * canvas.width;
    let y2 = uvs.uvB.y * canvas.height;

    this.tl = new Handle(this, x1, y1, [BoundaryType.Min, BoundaryType.Min]);
    this.bl = new Handle(this, x1, y2, [BoundaryType.Min, BoundaryType.Max]);
    this.tr = new Handle(this, x2, y1, [BoundaryType.Max, BoundaryType.Min]);
    this.br = new Handle(this, x2, y2, [BoundaryType.Max, BoundaryType.Max]);

    this.tl.xSibling = this.bl;
    this.bl.xSibling = this.tl;

    this.tr.xSibling = this.br;
    this.br.xSibling = this.tr;

    this.tl.ySibling = this.tr;
    this.tr.ySibling = this.tl;

    this.bl.ySibling = this.br;
    this.br.ySibling = this.bl;


    this.middle = new Handle(this, (x2 - x1) / 2, (y2 - y1) / 2, [BoundaryType.Center, BoundaryType.Center]);
    this.middle.equidistantSiblings.push(this.tl, this.tr, this.bl, this.br);
    this.tl.middleSibling = this.middle;
    this.tr.middleSibling = this.middle;
    this.bl.middleSibling = this.middle;
    this.br.middleSibling = this.middle;

    this.rectangle = new Rectangle(this.tl, this.br);
  }


  getHoveredHandle(x, y)
  {
    for(let handle of this.handles){
      if(handle.hovered(x, y)){
        return handle;
      }
    }

    return null;
  }


  draw(context)
  {
    this.rectangle.draw(context);

    for(let handle of this.handles){
      handle.draw(context);
    }
  }
}

class ScreenPreview
{
  constructor(webApp){
    this.webApp = webApp;
    this.width = 512;
    this.canvas = document.getElementById("screenPreview");
    this.canvas.style.backgroundColor = "#000000";
    this.canvas.style.width = this.width + "px";
    this.dpi = window.devicePixelRatio;
    this.context = this.canvas.getContext("2d");

    this.mouseDown = false;

    this.canvas.onmousemove = (data) => {this._updateCursorPosition(data)};
    this.canvas.onmousedown = (data) => {this._checkMouse(data)};
    this.canvas.onmouseup = (data) => {this._checkMouse(data)};
    this.canvas.onmouseout = (data) => {this._leaveMouse(data)};
  }


  setDimensions(screenX, screenY)
  {
    let ratio = screenY / screenX;

    this.canvas.width = this.width;
    this.canvas.height = this.width * ratio;

    this.canvas.style.width = this.width + "px";
    this.canvas.style.height = this.width * ratio + "px";

    this.context.scale(this.dpi, this.dpi);
  }

  _drawRectangle(x1, y1, x2, y2)
  {
    this.context.fillStyle = "#ffffff80";
    this.context.strokeStyle = "#ffffffff";

    let width = x2 - x1;
    let height = y2 - y1;

    this.context.strokeRect(x1, y1, width, height);
    this.context.fillRect(x1, y1, width, height);
  }


  _updateCursorPosition(data){
    var rect = this.canvas.getBoundingClientRect();
    let x = data.clientX - rect.left;
    let y = data.clientY - rect.top;

    if(!this.portionWidget){
      return;
    }

    this.hoveredHandle = this.hoveredHandle || this.portionWidget.getHoveredHandle(x, y);

    if(this.hoveredHandle){
      if(this.mouseDown){
        var rect = this.canvas.getBoundingClientRect();
        let x = data.layerX - rect.x;
        let y = data.layerY - rect.y;
        this.hoveredHandle.setPosition(x, y);

        this._updateUVs();
      }
    }

    this.draw();
  }


  _checkMouse(data)
  {
    if(data.buttons == 1){
      this.mouseDown = true;
    }
    else if(data.buttons == 0){
      this.mouseDown = false;
      this.hoveredHandle = undefined;
    }
  }

  _leaveMouse(data)
  {
    this.mouseDown = false;
  }


  _updateUVs()
  {
    let uvAx = this.portionWidget.tl.x / this.canvas.width;
    let uvAy = this.portionWidget.tl.y / this.canvas.height;

    let uvBx = this.portionWidget.br.x / this.canvas.width;
    let uvBy = this.portionWidget.br.y / this.canvas.height;

    let newUV = {
      uvA : {
        x : uvAx,
        y : uvAy
      },
      uvB : {
        x : uvBx,
        y : uvBy
      }
    };

    this.webApp.notifyUVs(newUV);
  }

  initLightRegion(light)
  {
    this.currentLight = light;
    this.portionWidget = new PortionWidget(this.currentLight.uv, this.canvas);
  }


  draw()
  {
    this.context.clearRect(0, 0, this.canvas.width, this.canvas.height)
    this.portionWidget.draw(this.context);
  }
}
