// Huenicorn headers (pulling in Crow) must come before Qt headers because of
// the Qt "signals" macro clashing with Crow's signals() method
#include <Huenicorn/HuenicornCore.hpp>
#include <Huenicorn/Interpolation.hpp>
#include <Huenicorn/UV.hpp>

#include <Huenicorn/Gui/QtController.hpp>
#include <Huenicorn/Gui/ChannelListModel.hpp>
#include <Huenicorn/Gui/EntertainmentConfigurationModel.hpp>

#include <QCoreApplication>
#include <QStringList>
#include <QThreadPool>
#include <QVariant>

#include <algorithm>
#include <cmath>


namespace
{
  QVariant jsonToVariant(const nlohmann::json& json)
  {
    if(json.is_boolean()){
      return json.get<bool>();
    }
    if(json.is_number_integer()){
      return static_cast<qlonglong>(json.get<long long>());
    }
    if(json.is_number_unsigned()){
      return static_cast<qulonglong>(json.get<unsigned long long>());
    }
    if(json.is_number_float()){
      return json.get<double>();
    }
    if(json.is_string()){
      return QString::fromStdString(json.get<std::string>());
    }
    if(json.is_array()){
      QVariantList list;
      for(const auto& entry : json){
        list.append(jsonToVariant(entry));
      }
      return list;
    }
    if(json.is_object()){
      QVariantMap map;
      for(auto it = json.begin(); it != json.end(); ++it){
        map[QString::fromStdString(it.key())] = jsonToVariant(it.value());
      }
      return map;
    }
    return QVariant();
  }


  // Approximate Kelvin -> RGB conversion (Tanner Helland), used to preview
  // the white mode color in the GUI before it is applied to the bridge.
  QColor kelvinToColor(int kelvin)
  {
    double t = std::clamp(kelvin, 2000, 6500) / 100.0;
    double r, g, b;
    if(t <= 66){
      r = 255.0;
      g = 99.4708025861 * std::log(t) - 161.1195681661;
    }
    else{
      r = 329.698727446 * std::pow(t - 60, -0.1332047592);
      g = 288.1221695283 * std::pow(t - 60, -0.0755148492);
    }
    if(t >= 66){
      b = 255.0;
    }
    else if(t <= 19){
      b = 0.0;
    }
    else{
      b = 138.5177312231 * std::log(t - 10) - 305.0447927307;
    }
    auto clampChannel = [](double v){ return static_cast<int>(std::clamp(std::round(v), 0.0, 255.0)); };
    return QColor(clampChannel(r), clampChannel(g), clampChannel(b));
  }


  // Bridge requests are blocking (up to 1s timeout each): run them off the GUI
  // thread so slider drags don't freeze the UI
  void applyManualStateAsync(Huenicorn::HuenicornCore* core, nlohmann::json jsonState)
  {
    QThreadPool::globalInstance()->start([core, jsonState](){ core->setManualLightState(jsonState); });
  }
}


namespace Huenicorn
{
  QtController::QtController(HuenicornCore* core, QObject* parent):
  QObject(parent),
  m_core(core),
  m_channelsModel(new ChannelListModel(core, this)),
  m_entertainmentConfigurationsModel(new EntertainmentConfigurationModel(core, this))
  {
    m_pollTimer.setInterval(500);
    connect(&m_pollTimer, &QTimer::timeout, this, &QtController::_poll);
    m_pollTimer.start();
  }


  QString QtController::version() const
  {
    return QString::fromStdString(m_core->version());
  }


  QString QtController::configFilePath() const
  {
    return QString::fromStdString(m_core->configFilePath().string());
  }


  bool QtController::setupRequired() const
  {
    return !m_core->initialSetupOk();
  }


  bool QtController::running() const
  {
    return m_core->running();
  }


  bool QtController::streamingActive() const
  {
    return m_core->streamingActive();
  }


  int QtController::selectedChannelId() const
  {
    return m_selectedChannelId;
  }


  void QtController::setSelectedChannelId(int channelId)
  {
    if(m_selectedChannelId == channelId){
      return;
    }

    m_selectedChannelId = channelId;
    emit selectedChannelIdChanged();
  }


  ChannelListModel* QtController::channelsModel() const
  {
    return m_channelsModel;
  }


  EntertainmentConfigurationModel* QtController::entertainmentConfigurationsModel() const
  {
    return m_entertainmentConfigurationsModel;
  }


  QVariantList QtController::subsampleCandidates() const
  {
    return m_subsampleCandidates;
  }


  int QtController::subsampleWidth() const
  {
    return m_subsampleWidth;
  }


  int QtController::refreshRate() const
  {
    return m_refreshRate;
  }


  int QtController::maxRefreshRate() const
  {
    return m_maxRefreshRate;
  }


  int QtController::transitionSmoothing() const
  {
    return m_transitionSmoothing;
  }


  int QtController::displayWidth() const
  {
    return m_displayWidth;
  }


  int QtController::displayHeight() const
  {
    return m_displayHeight;
  }


  QVariantList QtController::interpolations() const
  {
    return m_interpolations;
  }


  int QtController::interpolation() const
  {
    return m_interpolation;
  }


  QString QtController::currentEntertainmentConfigurationId() const
  {
    return m_currentEntertainmentConfigurationId;
  }


  int QtController::channelsVersion() const
  {
    return m_channelsVersion;
  }


  QVariantMap QtController::autodetectBridges()
  {
    nlohmann::json result = m_core->autodetectedBridge();

    QVariantMap map;
    map["succeeded"] = result.value("succeeded", false);

    if(result.contains("error")){
      map["error"] = QString::fromStdString(result.at("error").get<std::string>());
    }

    if(result.contains("bridges")){
      QVariantList bridges;
      for(const auto& bridge : result.at("bridges")){
        QVariantMap bridgeMap;
        bridgeMap["id"] = QString::fromStdString(bridge.value("id", ""));
        bridgeMap["internalipaddress"] = QString::fromStdString(bridge.value("internalipaddress", ""));
        bridges.append(bridgeMap);
      }
      map["bridges"] = bridges;
    }

    return map;
  }


  bool QtController::validateBridgeAddress(const QString& bridgeAddress)
  {
    QString sanitized = bridgeAddress.trimmed();
    while(sanitized.endsWith('/')){
      sanitized.chop(1);
    }

    if(sanitized.isEmpty()){
      return false;
    }

    return m_core->validateBridgeAddress(sanitized.toStdString());
  }


  QVariantMap QtController::registerNewUser()
  {
    nlohmann::json result = m_core->registerNewUser();

    QVariantMap map;
    map["succeeded"] = result.value("succeeded", false);

    if(result.contains("error")){
      map["error"] = QString::fromStdString(result.at("error").get<std::string>());
    }
    if(result.contains("username")){
      map["username"] = QString::fromStdString(result.at("username").get<std::string>());
    }
    if(result.contains("clientkey")){
      map["clientkey"] = QString::fromStdString(result.at("clientkey").get<std::string>());
    }

    if(map["succeeded"].toBool()){
      // The core start() loop polls the config, so the streaming will begin on its own
      emit setupRequiredChanged();
    }

    return map;
  }


  bool QtController::selectEntertainmentConfiguration(const QString& entertainmentConfigurationId)
  {
    bool succeeded = m_core->setEntertainmentConfiguration(entertainmentConfigurationId.toStdString());

    if(succeeded){
      _reloadChannels();
      _reloadEntertainmentConfigurations();
      setSelectedChannelId(-1);
    }

    return succeeded;
  }


  bool QtController::setChannelActivity(int channelId, bool active)
  {
    bool succeeded = m_core->setChannelActivity(static_cast<uint8_t>(channelId), active);

    if(succeeded){
      _reloadChannels();
    }

    return succeeded;
  }


  void QtController::setStreamingActive(bool active)
  {
    m_core->setStreamingActive(active);

    // Emit immediately so the QML property is up to date even if the user
    // toggles again before the next poll
    if(m_wasStreamingActive != active){
      m_wasStreamingActive = active;
      emit streamingActiveChanged();
    }
  }


  QVariantMap QtController::setChannelUV(int channelId, int corner, double x, double y)
  {
    QVariantMap map;

    UVCorner uvCorner = static_cast<UVCorner>(corner);
    const UVs& clamped = m_core->setChannelUV(static_cast<uint8_t>(channelId), UV{static_cast<float>(x), static_cast<float>(y)}, uvCorner);

    map["uvMinX"] = clamped.min.x;
    map["uvMinY"] = clamped.min.y;
    map["uvMaxX"] = clamped.max.x;
    map["uvMaxY"] = clamped.max.y;

    _reloadChannels();

    return map;
  }


  bool QtController::setChannelGammaFactor(int channelId, double gammaFactor)
  {
    bool succeeded = m_core->setChannelGammaFactor(static_cast<uint8_t>(channelId), static_cast<float>(gammaFactor));

    if(succeeded){
      _reloadChannels();
    }

    return succeeded;
  }


  void QtController::setSubsampleWidth(int subsampleWidth)
  {
    m_core->setSubsampleWidth(static_cast<unsigned>(subsampleWidth));
    _refreshDisplayInfo();
  }


  void QtController::setRefreshRate(int refreshRate)
  {
    if(!m_core->running()){
      return;
    }

    m_core->setRefreshRate(static_cast<unsigned>(refreshRate));
    _refreshDisplayInfo();
  }


  void QtController::setTransitionSmoothing(int transitionSmoothing)
  {
    m_core->setTransitionSmoothing(static_cast<float>(transitionSmoothing));
    _refreshDisplayInfo();
  }


  void QtController::setInterpolation(int interpolation)
  {
    m_core->setInterpolation(static_cast<unsigned>(interpolation));
    _refreshInterpolations();
  }


  void QtController::saveProfile()
  {
    m_core->saveProfile();
  }


  void QtController::quit()
  {
    m_core->stop();
    QCoreApplication::quit();
  }


  bool QtController::setManualLight(bool on, int brightness, const QString& mode, int hue, int saturation, int kelvin)
  {
    const bool whiteMode = (mode == QStringLiteral("white"));

    // Store the raw UI fields (used to restore the manual page state) and the
    // bridge payload computed from them
    nlohmann::json jsonState{
      {"on", on},
      {"brightness", std::clamp(brightness, 0, 100)},
      {"mode", mode.toStdString()},
      {"hue", hue},
      {"saturation", saturation},
      {"kelvin", kelvin},
    };

    if(whiteMode){
      jsonState["mirek"] = std::clamp(static_cast<int>(std::lround(1000000.0 / std::max(kelvin, 1))), 153, 500);
    }
    else{
      const QColor color = manualColor(mode, hue, saturation, kelvin);
      jsonState["r"] = color.red();
      jsonState["g"] = color.green();
      jsonState["b"] = color.blue();
    }

    applyManualStateAsync(m_core, std::move(jsonState));
    return true;
  }


  bool QtController::setManualLightHex(bool on, int brightness, const QString& hex)
  {
    const QColor color = QColor::fromString(hex.trimmed());
    if(!color.isValid()){
      return false;
    }

    int h = 0, s = 0, l = 0;
    color.getHsl(&h, &s, &l);

    nlohmann::json jsonState{
      {"on", on},
      {"brightness", std::clamp(brightness, 0, 100)},
      {"mode", "color"},
      {"hue", (h < 0) ? 0 : h},
      {"saturation", std::clamp(s * 100 / 255, 0, 100)},
      {"kelvin", 4000},
      {"r", color.red()},
      {"g", color.green()},
      {"b", color.blue()},
    };

    applyManualStateAsync(m_core, std::move(jsonState));
    return true;
  }


  QString QtController::manualColorHex(const QString& mode, int hue, int saturation, int kelvin) const
  {
    return manualColor(mode, hue, saturation, kelvin).name(QColor::HexRgb).toUpper();
  }


  QColor QtController::colorFromHex(const QString& hex) const
  {
    return QColor::fromString(hex.trimmed());
  }


  QVariantMap QtController::hexToHsl(const QString& hex) const
  {
    QVariantMap result;
    result["valid"] = false;

    const QColor color = QColor::fromString(hex.trimmed());
    if(!color.isValid()){
      return result;
    }

    int h = 0, s = 0, l = 0;
    color.getHsl(&h, &s, &l);

    result["valid"] = true;
    result["hue"] = (h < 0) ? 0 : h;
    result["saturation"] = std::clamp(s * 100 / 255, 0, 100);
    result["hex"] = color.name(QColor::HexRgb).toUpper();
    return result;
  }


  QColor QtController::manualColor(const QString& mode, int hue, int saturation, int kelvin) const
  {
    if(mode == QStringLiteral("white")){
      return kelvinToColor(kelvin);
    }
    return QColor::fromHslF(
      std::clamp(hue, 0, 360) / 360.0,
      std::clamp(saturation, 0, 100) / 100.0,
      0.5
    );
  }


  QVariantMap QtController::manualLightState() const
  {
    return jsonToVariant(m_core->manualLightState()).toMap();
  }


  QVariantMap QtController::channelData(int channelId) const
  {
    QVariantMap map;

    if(!m_core->running()){
      return map;
    }

    const Channels& channels = m_core->channels();
    auto it = channels.find(static_cast<uint8_t>(channelId));
    if(it == channels.end()){
      return map;
    }

    const Channel& channel = it->second;
    const UVs& uvs = channel.uvs();

    QStringList devices;
    for(const Device& device : channel.devices()){
      devices << QString::fromStdString(device.name);
    }

    map["channelId"] = static_cast<int>(it->first);
    map["active"] = channel.state() == Channel::State::Active;
    map["gammaFactor"] = channel.gammaFactor();
    map["uvMinX"] = uvs.min.x;
    map["uvMinY"] = uvs.min.y;
    map["uvMaxX"] = uvs.max.x;
    map["uvMaxY"] = uvs.max.y;
    map["devices"] = devices;
    map["label"] = QString("Channel %1").arg(it->first + 1);

    return map;
  }


  void QtController::_poll()
  {
    bool setupReq = setupRequired();
    bool isRunning = running();
    bool isStreamingActive = streamingActive();

    if(setupReq != m_wasSetupRequired){
      m_wasSetupRequired = setupReq;
      emit setupRequiredChanged();
    }

    if(isRunning != m_wasRunning){
      m_wasRunning = isRunning;
      emit runningChanged();

      if(isRunning){
        _reloadEntertainmentConfigurations();
        _reloadChannels();
        _refreshDisplayInfo();
        _refreshInterpolations();
      }
    }

    if(isStreamingActive != m_wasStreamingActive){
      m_wasStreamingActive = isStreamingActive;
      emit streamingActiveChanged();
    }
  }


  void QtController::_refreshDisplayInfo()
  {
    if(!m_core->running()){
      return;
    }

    glm::ivec2 resolution = m_core->displayResolution();
    m_displayWidth = resolution.x;
    m_displayHeight = resolution.y;

    m_subsampleCandidates.clear();
    for(const auto& candidate : m_core->subsampleResolutionCandidates()){
      QVariantMap candidateMap;
      candidateMap["x"] = candidate.x;
      candidateMap["y"] = candidate.y;
      m_subsampleCandidates.append(candidateMap);
    }

    m_subsampleWidth = static_cast<int>(m_core->subsampleWidth());
    m_refreshRate = static_cast<int>(m_core->refreshRate());
    m_maxRefreshRate = static_cast<int>(m_core->maxRefreshRate());
    m_transitionSmoothing = static_cast<int>(m_core->transitionSmoothing());

    emit displayInfoChanged();
  }


  void QtController::_reloadChannels()
  {
    m_channelsModel->reload();
    ++m_channelsVersion;
    emit channelsChanged();
  }


  void QtController::_reloadEntertainmentConfigurations()
  {
    m_entertainmentConfigurationsModel->reload();

    auto currentId = m_core->currentEntertainmentConfigurationId();
    m_currentEntertainmentConfigurationId = currentId.has_value() ? QString::fromStdString(currentId.value()) : QString();

    emit entertainmentConfigurationsChanged();
  }


  void QtController::_refreshInterpolations()
  {
    m_interpolations.clear();

    for(const auto& [name, type] : m_core->availableInterpolations()){
      QVariantMap interpolationMap;
      interpolationMap["name"] = QString::fromStdString(name);
      interpolationMap["value"] = static_cast<int>(type);
      m_interpolations.append(interpolationMap);
    }

    m_interpolation = static_cast<int>(m_core->interpolation());

    emit interpolationInfoChanged();
  }
}
