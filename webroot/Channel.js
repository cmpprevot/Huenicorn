class Channel
{
  constructor(channelData, node = null)
  {
    this.channelId = channelData.channelId;
    //this.name = channelData.name;
    //this.productName = channelData.productName;
    this.gammaFactor = channelData.gammaFactor;
    this.node = node;

    this.uvs = null;
    if(channelData.uvs){
      this.uvs = channelData.uvs;
    }
  }
}
