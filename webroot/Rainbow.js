class Rainbow
{
  constructor()
  {
    this.hueIncrement = 180;
  }


  start()
  {
    document.documentElement.style.setProperty("--shadowRadius", "60px");
    this.rainbowInterval = setInterval(() => {
      document.documentElement.style.setProperty("--rainbowColor", `${this.hueIncrement}, 100%, 50%`);
      this.hueIncrement = (this.hueIncrement + 1) % 360;
    }, 50);
  }


  stop()
  {
    this.rainbowInterval.stop();
  }
}


var rainbow = new Rainbow();
rainbow.start();