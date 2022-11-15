class Light
{
  constructor(data)
  {
    this.lightId = data.lightId;
    this.name = data.name;
    this.productName = data.productName;

    this.uvs = null;
    if(data.uvs){
      this.uvs = data.uvs;
    }
  }
}
