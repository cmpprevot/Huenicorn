class Light
{
  constructor(lightData, node = null)
  {
    this.id = lightData.id;
    this.name = lightData.name;
    this.productName = lightData.productName;
    this.node = node;

    this.uvs = null;
    if(lightData.uvs){
      this.uvs = lightData.uvs;
    }
  }
}
