#pragma once

#include <QObject>
#include <QColor>
#include <QString>
#include <QTimer>
#include <QVariantList>
#include <QVariantMap>

namespace Huenicorn
{
  class HuenicornCore;
  class ChannelListModel;
  class EntertainmentConfigurationModel;

  /**
   * @brief Bridge between the Qt Quick GUI and HuenicornCore.
   *
   * Exposes the core state as QML properties and the core actions as invokable
   * methods. Instanciated once in main() and exposed to QML as the "huenicorn"
   * context property.
   */
  class QtController : public QObject
  {
    Q_OBJECT

    Q_PROPERTY(QString version READ version CONSTANT)
    Q_PROPERTY(QString configFilePath READ configFilePath CONSTANT)
    Q_PROPERTY(bool setupRequired READ setupRequired NOTIFY setupRequiredChanged)
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(bool streamingActive READ streamingActive NOTIFY streamingActiveChanged)
    Q_PROPERTY(bool manualMode READ manualMode NOTIFY manualModeChanged)
    Q_PROPERTY(int selectedChannelId READ selectedChannelId WRITE setSelectedChannelId NOTIFY selectedChannelIdChanged)
    Q_PROPERTY(ChannelListModel* channels READ channelsModel CONSTANT)
    Q_PROPERTY(EntertainmentConfigurationModel* entertainmentConfigurations READ entertainmentConfigurationsModel CONSTANT)
    Q_PROPERTY(QVariantList subsampleCandidates READ subsampleCandidates NOTIFY displayInfoChanged)
    Q_PROPERTY(int subsampleWidth READ subsampleWidth NOTIFY displayInfoChanged)
    Q_PROPERTY(int refreshRate READ refreshRate NOTIFY displayInfoChanged)
    Q_PROPERTY(int maxRefreshRate READ maxRefreshRate NOTIFY displayInfoChanged)
    Q_PROPERTY(int transitionSmoothing READ transitionSmoothing NOTIFY displayInfoChanged)
    Q_PROPERTY(int displayWidth READ displayWidth NOTIFY displayInfoChanged)
    Q_PROPERTY(int displayHeight READ displayHeight NOTIFY displayInfoChanged)
    Q_PROPERTY(QVariantList interpolations READ interpolations NOTIFY interpolationInfoChanged)
    Q_PROPERTY(int interpolation READ interpolation NOTIFY interpolationInfoChanged)
    Q_PROPERTY(QString currentEntertainmentConfigurationId READ currentEntertainmentConfigurationId NOTIFY entertainmentConfigurationsChanged)
    Q_PROPERTY(int channelsVersion READ channelsVersion NOTIFY channelsChanged)
    Q_PROPERTY(int manualTargetChannelId READ manualTargetChannelId WRITE setManualTargetChannelId NOTIFY manualTargetChannelIdChanged)
    Q_PROPERTY(QString manualTargetLightId READ manualTargetLightId WRITE setManualTargetLightId NOTIFY manualTargetLightIdChanged)
    Q_PROPERTY(QVariantList manualLights READ manualLights NOTIFY channelsChanged)

  public:
    /**
     * @brief QtController constructor
     * 
     * @param core Pointer to Huenicorn core
     * @param parent Parent QObject
     */
    explicit QtController(HuenicornCore* core, QObject* parent = nullptr);


    // Getters
    QString version() const;
    QString configFilePath() const;
    bool setupRequired() const;
    bool running() const;
    bool streamingActive() const;

    /**
     * @brief Returns whether manual light control currently owns the stream
     * (ambilight following suspended)
     *
     * @return true Manual mode is active
     */
    bool manualMode() const;
    int selectedChannelId() const;
    ChannelListModel* channelsModel() const;
    EntertainmentConfigurationModel* entertainmentConfigurationsModel() const;
    QVariantList subsampleCandidates() const;
    int subsampleWidth() const;
    int refreshRate() const;
    int maxRefreshRate() const;
    int transitionSmoothing() const;
    int displayWidth() const;
    int displayHeight() const;
    QVariantList interpolations() const;
    int interpolation() const;
    QString currentEntertainmentConfigurationId() const;
    int channelsVersion() const;

    /**
     * @brief Returns the channel targeted by manual light commands
     *
     * @return int Channel ID, or -1 when all lights of the entertainment area are targeted
     */
    int manualTargetChannelId() const;

    /**
     * @brief Returns the light targeted by manual light commands
     *
     * @return QString Light service ID, or empty when no single light is targeted
     */
    QString manualTargetLightId() const;

    /**
     * @brief Returns the individual lights (light services) of the current
     * entertainment area, for the "Light" target scope. Cached; refreshed
     * whenever the channels change. Note: a multi-segment gradient strip
     * exposes a single light (its segments are addressed through the
     * "Channel" target scope)
     *
     * @return QVariantList Array of {lightId, name, channelId, channelCount},
     *         where channelId is the lowest channel holding the light and
     *         channelCount the number of channels sharing it
     */
    QVariantList manualLights() const;


    // Setter
    void setSelectedChannelId(int channelId);

    /**
     * @brief Sets the channel targeted by the next manual light commands
     *
     * @param channelId Channel ID, or -1 to target all lights of the entertainment area
     */
    void setManualTargetChannelId(int channelId);

    /**
     * @brief Sets the single light targeted by the next manual light commands.
     * Takes precedence over manualTargetChannelId when not empty.
     *
     * @param lightId Light service ID, or empty to clear the single-light target
     */
    void setManualTargetLightId(const QString& lightId);


    // Setup invokables
    /**
     * @brief Queries the Hue discovery service for bridges on the local network
     * 
     * @return QVariantMap {succeeded, bridges: [{id, internalipaddress}], error?}
     */
    Q_INVOKABLE QVariantMap autodetectBridges();

    /**
     * @brief Validates and registers a Hue bridge address
     * 
     * @param bridgeAddress Bridge address (IP or URL)
     * @return true Bridge address is valid and was registered
     */
    Q_INVOKABLE bool validateBridgeAddress(const QString& bridgeAddress);

    /**
     * @brief Registers a new user on the configured Hue bridge (link button must be pressed)
     * 
     * @return QVariantMap {succeeded, username?, clientkey?, error?}
     */
    Q_INVOKABLE QVariantMap registerNewUser();


    // Runtime invokables
    Q_INVOKABLE bool selectEntertainmentConfiguration(const QString& entertainmentConfigurationId);
    Q_INVOKABLE bool setChannelActivity(int channelId, bool active);
    Q_INVOKABLE void setStreamingActive(bool active);
    Q_INVOKABLE QVariantMap setChannelUV(int channelId, int corner, double x, double y);
    Q_INVOKABLE bool setChannelGammaFactor(int channelId, double gammaFactor);
    Q_INVOKABLE void setSubsampleWidth(int subsampleWidth);
    Q_INVOKABLE void setRefreshRate(int refreshRate);
    Q_INVOKABLE void setTransitionSmoothing(int transitionSmoothing);
    Q_INVOKABLE void setInterpolation(int interpolation);
    Q_INVOKABLE void saveProfile();
    Q_INVOKABLE void quit();

    /**
     * @brief Applies a manual state to the target of the manual panel (all
     * lights, a single channel or a single light). Enters manual mode: the
     * ambilight following is suspended while the manual mode is active, and
     * non-targeted channels freeze at their last color. Resume ambilight from
     * the Ambilight toggle to leave manual mode. The bridge payload (RGB for
     * the color mode, mirek for the white mode) is computed here: the QML layer
     * only forwards raw UI fields.
     *
     * @param on Power state
     * @param brightness Brightness (0-100)
     * @param mode "color" or "white"
     * @param hue Hue (0-360, color mode)
     * @param saturation Saturation (0-100, color mode)
     * @param kelvin Color temperature in Kelvin (white mode)
     * @return true State was accepted for application
     */
    Q_INVOKABLE bool setManualLight(bool on, int brightness, const QString& mode, int hue, int saturation, int kelvin);

    /**
     * @brief Preview color of a manual state for the GUI (color orb and swatches).
     * Single source of truth for the color conversions: color mode uses HSL,
     * white mode converts Kelvin to RGB (Tanner Helland approximation).
     *
     * @param mode "color" or "white"
     * @param hue Hue (0-360, color mode)
     * @param saturation Saturation (0-100, color mode)
     * @param kelvin Color temperature in Kelvin (white mode)
     * @return QColor Preview color
     */
    Q_INVOKABLE QColor manualColor(const QString& mode, int hue, int saturation, int kelvin) const;

    /**
     * @brief Same as setManualLight, but the color is given exactly as a hex
     * string ("#RRGGBB"): the bridge payload uses its raw RGB and the stored
     * hue/saturation are derived from it for state restore.
     *
     * @return true The hex string is a valid color and was accepted
     */
    Q_INVOKABLE bool setManualLightHex(bool on, int brightness, const QString& hex);

    /**
     * @brief Hex string ("#RRGGBB") of manualColor(), for display in the GUI
     */
    Q_INVOKABLE QString manualColorHex(const QString& mode, int hue, int saturation, int kelvin) const;

    /**
     * @brief Parses a hex color string ("#RRGGBB", also "#RGB", "#RRGGBBAA", names)
     * @return QColor Invalid color if the string cannot be parsed
     */
    Q_INVOKABLE QColor colorFromHex(const QString& hex) const;

    /**
     * @brief Converts a hex color string to the hue/saturation pair used by the GUI
     *
     * @return QVariantMap {valid: bool, hue: 0-360, saturation: 0-100, hex: normalized "#RRGGBB"}
     */
    Q_INVOKABLE QVariantMap hexToHsl(const QString& hex) const;

    /**
     * @brief Returns the last manual light state applied through setManualLight
     *
     * @return QVariantMap Last applied state, empty map if none was applied yet
     */
    Q_INVOKABLE QVariantMap manualLightState() const;

    /**
     * @brief Returns the data of a channel as a map (empty if unknown or not running)
     * 
     * @param channelId Channel ID
     * @return QVariantMap Channel data
     */
    Q_INVOKABLE QVariantMap channelData(int channelId) const;


  signals:
    void setupRequiredChanged();
    void runningChanged();
    void streamingActiveChanged();
    void manualModeChanged();
    void selectedChannelIdChanged();
    void displayInfoChanged();
    void interpolationInfoChanged();
    void entertainmentConfigurationsChanged();
    void channelsChanged();
    void manualTargetChannelIdChanged();
    void manualTargetLightIdChanged();


  private slots:
    /**
     * @brief Polls the core state and emits change signals when needed
     * 
     */
    void _poll();


  private:
    // Private methods
    void _refreshDisplayInfo();
    void _refreshInterpolations();
    void _reloadChannels();
    void _reloadEntertainmentConfigurations();


    // Attributes
    HuenicornCore* m_core;
    ChannelListModel* m_channelsModel;
    EntertainmentConfigurationModel* m_entertainmentConfigurationsModel;
    QTimer m_pollTimer;

    int m_selectedChannelId{-1};
    int m_manualTargetChannelId{-1};
    QString m_manualTargetLightId;
    bool m_wasSetupRequired{true};
    bool m_wasRunning{false};
    bool m_wasStreamingActive{true};
    bool m_wasManualMode{false};
    QString m_currentEntertainmentConfigurationId;
    int m_channelsVersion{0};
    QVariantList m_manualLights;

    QVariantList m_subsampleCandidates;
    int m_subsampleWidth{0};
    int m_refreshRate{0};
    int m_maxRefreshRate{0};
    int m_transitionSmoothing{0};
    int m_displayWidth{0};
    int m_displayHeight{0};
    QVariantList m_interpolations;
    int m_interpolation{0};
  };
}
