class Light
{
  constructor(lightData)
  {
    this.id = lightData.id;
    this.name = lightData.name;
    this.productName = lightData.productName;

    this.uvs = null;
    if(lightData.uvs){
      this.uvs = lightData.uvs;
    }
  }
}
