#pragma once

#include <Huenicorn/IRestServer.hpp>

#include <filesystem>


namespace Huenicorn
{
  class HuenicornCore;

  class SetupBackend : public IRestServer
  {
  public:
    // Constructor / destructor
    /**
     * @brief SetupBackend constructor
     * 
     * @param huenicornCore Pointer to Huenicorn core
     */
    SetupBackend(HuenicornCore* huenicornCore);


    /**
     * @brief SetupBackend destructor
     * 
     */
    ~SetupBackend();


  // Getters
    /**
     * @brief Returns wether the setup was aborted or not
     * 
     * @return true Setup was aborted
     * @return false Setup was completed
     */
    bool aborted() const;


  protected:
    /**
     * @brief Overriden method to call _spawnBrowser
     * 
     */
    void _onStart() override;


  private:
    /**
     * @brief Opens user default browser on Huenicorn setup page
     * 
     */
    void _spawnBrowser();


  protected:
    // Handlers
    /**
     * @brief Returns the version of the backend project
     * 
     * @param session Pending HTTP connection
     */
    void _getVersion(const SharedSession& session) const override;

  private:
    /**
     * @brief Completes Huenicorn setup and stops setup backend server
     * 
     * @param session Pending HTTP connection
     */
    void _finish(const SharedSession& session);


    /**
     * @brief Aborts Huenicorn setup and stops setup backend server
     * 
     * @param session Pending HTTP connection
     */
    void _abort(const SharedSession& session);


    /**
     * @brief Handles call for Hue bridge auto-detection
     * 
     * @param session Pending HTTP connection
     */
    void _autoDetectBridge(const SharedSession& session);


    /**
     * @brief Returns the path to the config file
     * 
     * @param session Pending HTTP connection
     */
    void _configFilePath(const SharedSession& session);


    /**
     * @brief Handles a requests for a Hue bridge address validation
     * 
     * @param session Pending HTTP connection
     */
    void _validateBridgeAddress(const SharedSession& session);


    /**
     * @brief Handles a requests for Hue bridge credentials validation
     * 
     * @param session 
     */
    void _validateCredentials(const SharedSession& session);


    /**
     * @brief Handles a request for a new Hue bridge user registration
     * 
     * @param session 
     */
    void _registerNewUser(const SharedSession& session);


    // Attributes
    HuenicornCore* m_huenicornCore;
    const std::filesystem::path m_webroot;
    std::unordered_map<std::string, std::string> m_contentTypes;
    bool m_aborted{false};
  };
}
