
namespace WR3223
{
  WR3223Connector *connector;
  WR3223StatusValueHolder *statusHolder;
  WR3223ErrorValueDecoder *errorHolder;
  WR3223ModusValueDecoder *modeHolder;
  WR3223RelaisValueDecoder *relaisDecoder;

  bool publish_to_statusHolder_ta(const char* data)  {
    return statusHolder->setTaStatus(data);
  }

  bool publish_to_statusHolder_sw(const char* data)  {
    return statusHolder->setSWStatus(data);
  }

  bool publish_to_errorHolder(const char* data)  {
    return errorHolder->publishValue(data);
  }

  bool publish_to_modeHolder(const char* data)  {
    return modeHolder->publishValue(data);
  }

  bool publish_to_relaisDecoder(const char* data)  {    
    return relaisDecoder->publish_relais(data);
  }

  class WR3223Controller : public PollingComponent, public CustomAPIDevice {
  public:
    WR3223Controller(int update_intervall, UARTComponent *parent) : PollingComponent(update_intervall)
    { 
      connector = new WR3223Connector(parent);
      statusHolder = new WR3223StatusValueHolder();
      errorHolder = new WR3223ErrorValueDecoder();
      modeHolder = new WR3223ModusValueDecoder();
      relaisDecoder = new WR3223RelaisValueDecoder();
    }     

    // diese kommandos muss bei fehlendem Bedienteil mindesten alle 20 sekunden den Status schreiben
    const CommandPair StatusWriteCmd = CommandPair(WR3223Commands::SW, publish_to_statusHolder_sw);
    const CommandPair RelaisCmd =  CommandPair(WR3223Commands::RL, publish_to_relaisDecoder);
    const CommandPair StatusUpdateCmd = CommandPair(WR3223Commands::Ta, publish_to_statusHolder_ta);
    const CommandPair ModusCmd = CommandPair(WR3223Commands::MD, publish_to_modeHolder);
    const CommandPair ErrorCmd = CommandPair(WR3223Commands::ER, publish_to_errorHolder);

    const CommandPair LufstufeCmd = CommandPair(WR3223Commands::LS, ECommandResultType::Number);    
    const CommandPair ZusatzheizungFreiCmd = CommandPair(WR3223Commands::ZH, ECommandResultType::Bool);
    const CommandPair ZusatzheizungEinCmd = CommandPair(WR3223Commands::ZE, ECommandResultType::Bool);

    const CommandPair VentLv1Cmd = CommandPair(WR3223Commands::L1, ECommandResultType::Number);
    const CommandPair VentLv2Cmd = CommandPair(WR3223Commands::L2, ECommandResultType::Number);
    const CommandPair VentLv3Cmd = CommandPair(WR3223Commands::L3, ECommandResultType::Number);

    //const CommandPair ZuluftSoll = CommandPair(WR3223Commands::SP, ECommandResultType::Number);
    //const CommandPair ZuluftSollRe = CommandPair(WR3223Commands::Re, ECommandResultType::Number);
    //const CommandPair RaumsollRd = CommandPair(WR3223Commands::Rd, ECommandResultType::Number);

    const CommandPair ETSommer = CommandPair(WR3223Commands::ES, ECommandResultType::Number);
	const CommandPair ETWinter = CommandPair(WR3223Commands::EW, ECommandResultType::Number);
	const CommandPair StartEWT = CommandPair(WR3223Commands::EE, ECommandResultType::Number);
	const CommandPair StopETW = CommandPair(WR3223Commands::EA, ECommandResultType::Number);
	const CommandPair Sommerstop = CommandPair(WR3223Commands::Es, ECommandResultType::Number);
    // const CommandPair AbtauStartCmd = CommandPair(WR3223Commands::AE, ECommandResultType::Number);
    // const CommandPair AbtauEndeCmd = CommandPair(WR3223Commands::AA, ECommandResultType::Number);

    // const CommandPair AbtauLuftstufeCmd = CommandPair(WR3223Commands::Az, ECommandResultType::Number);
    // const CommandPair AbtauPauseCmd = CommandPair(WR3223Commands::AP, ECommandResultType::Number);

    // const CommandPair AbtauNachlaufzeitCmd = CommandPair(WR3223Commands::AN, ECommandResultType::Number);
    // const CommandPair AbtauRueckwaermezahlCmd = CommandPair(WR3223Commands::AR, ECommandResultType::Number);

    const CommandPair ZuluftCmd = CommandPair(WR3223Commands::LD, ECommandResultType::Number);
	const CommandPair AbluftCmd = CommandPair(WR3223Commands::Ld, ECommandResultType::Number);

    const static int cmdPairCount = 13;
    const CommandPair Commands[cmdPairCount] = 
    {  

      ModusCmd,

      CommandPair(WR3223Commands::T1, TempSensors::verdampfertemperatur_sensor),
      CommandPair(WR3223Commands::T2, TempSensors::kondensatortemperatur_sensor),
      CommandPair(WR3223Commands::T3, TempSensors::aussentemperatur_sensor),      
      CommandPair(WR3223Commands::T5, TempSensors::fortlufttemperatur_sensor),
      CommandPair(WR3223Commands::T6, TempSensors::zulufttemperatur_sensor),     
      CommandPair(WR3223Commands::T7, TempSensors::ewttemperatur_sensor),	  
      CommandPair(WR3223Commands::T8, TempSensors::nachvorheizregistertemperatur_sensor),
      
      StatusUpdateCmd,
      
      RelaisCmd,

      ErrorCmd,      

      CommandPair(WR3223Commands::NA, LueftungSensor::drehzahl_abluft_sensor),
      CommandPair(WR3223Commands::NZ, LueftungSensor::drehzahl_zuluft_sensor),
      
      //ZuluftSoll,
      //ZuluftSollRe,
      //RaumsollRd,

      //StartEWT,
	  //StopETW,
	  //Sommerstop,
      //StatusWriteCmd,
      //LufstufeCmd 
    };

    /// @brief Liefert den Ventilatorstellwert der Übergebenen Luftstufe (40-100%)
    int get_vent_level_speed(int pLevel)
    {
      switch (pLevel)
      {
        case 1:
          return std::stoi(readLine(VentLv1Cmd));
        case 2:
          return std::stoi(readLine(VentLv2Cmd));
        case 3:
          return std::stoi(readLine(VentLv3Cmd));
        default:
          return 0;        
      }
    }

    /// @brief Setzt den Ventilatorstellwert der Übergebenen Luftstufe (40-100%)
    bool set_vent_level_speed(int pLevel, int pSpeed)
    {
      if(pSpeed < 40 || pSpeed > 100)
      {
        ESP_LOGE("SET VENT LEVEL", "Ventilatorstellwert für Luftstufe %i konnte nicht gesetzt werden.", pLevel);
        ESP_LOGE("SET VENT LEVEL", "Wert %i liegt außerhalb des zulässigen Bereichs: [40-100]", pSpeed);
        return false;
      }

      char const* data = std::to_string(pSpeed).c_str();
      bool result = false;
      switch (pLevel)
      {
        case 1:
          result = connector->write(VentLv1Cmd, data);          
          break;
        case 2:
          result = connector->write(VentLv2Cmd, data);          
          break;
        case 3:
          result = connector->write(VentLv3Cmd, data);          
          break;
        default:
          break;
      }
      vent_level_1_speed->update();
      vent_level_2_speed->update();
      vent_level_3_speed->update();
      return result;
    }
	
	/// @brief Liefert den Zuluft-Stellwert (-20% - +20%)
	int get_zuluft_speed()
	{
    return std::stoi(readLine(ZuluftCmd));
	}

	/// @brief Setzt den Zuluft-Stellwert (-20% - +20%)
	bool set_zuluft_speed(int zuluftValue)
	{
    // Überprüfen, ob der Zuluftwert im zulässigen Bereich liegt
    if (zuluftValue < -20 || zuluftValue > 20)
    {
        ESP_LOGE("SET ZULUFT", "Zuluftstellwert konnte nicht gesetzt werden.");
        ESP_LOGE("SET ZULUFT", "Wert %i liegt außerhalb des zulässigen Bereichs: [-20 - +20]", zuluftValue);
        return false;
    }

    // Konvertiere den Zuluftwert in einen String
    std::string valueStr = std::to_string(zuluftValue);
    const char* data = valueStr.c_str();
    
    // Setze die Zuluft-Geschwindigkeit
    bool result = connector->write(ZuluftCmd, data);
    
    if (result)
    {
        zuluft_speed->update();
    }
    
    return result;
	}
	
	/// @brief Liefert den Abluft-Stellwert (-20% - +20%)
	int get_abluft_speed()
	{
    return std::stoi(readLine(AbluftCmd));
	}

	/// @brief Setzt den Abluft-Stellwert (-20% - +20%)
	bool set_abluft_speed(int abluftValue)
	{
    // Überprüfen, ob der Zuluftwert im zulässigen Bereich liegt
    if (abluftValue < -20 || abluftValue > 20)
    {
        ESP_LOGE("SET ABLUFT", "Abluftstellwert konnte nicht gesetzt werden.");
        ESP_LOGE("SET ABLUFT", "Wert %i liegt außerhalb des zulässigen Bereichs: [-20 - +20]", abluftValue);
        return false;
    }

    // Konvertiere den Abluftwert in einen String
    std::string valueStr = std::to_string(abluftValue);
    const char* data = valueStr.c_str();
    
    // Setze die Abluft-Geschwindigkeit
    bool result = connector->write(AbluftCmd, data);
    
    if (result)
    {
        abluft_speed->update();
    }
    
    return result;
	}
	
	/// @brief Liefert den ETSommer (+15°C - +40°C)
	int get_ETSommer_speed()
	{
    return std::stoi(readLine(ETSommer));
	}

	/// @brief Setzt den ETSommer (+15°C - +40°C)
	bool set_ETSommer_speed(int ETSommerValue)
	{
    // Überprüfen, ob der Zuluftwert im zulässigen Bereich liegt
    if (ETSommerValue < 15 || ETSommerValue > 40)
    {
        ESP_LOGE("SET ETSommer", "ETSommer konnte nicht gesetzt werden.");
        ESP_LOGE("SET ETSommer", "Wert %i liegt außerhalb des zulässigen Bereichs: [15 - 40]", ETSommerValue);
        return false;
    }

    // Konvertiere den ETSommer in einen String
    std::string valueStr = std::to_string(ETSommerValue);
    const char* data = valueStr.c_str();
    
    // Setze die ETSommer
    bool result = connector->write(ETSommer, data);
    
    if (result)
    {
        ETSommer_speed->update();
    }
    
    return result;
	}

	/// @brief Liefert den ETWinter (+15°C - +40°C)
	int get_ETWinter_speed()
	{
    return std::stoi(readLine(ETWinter));
	}

	/// @brief Setzt den ETWinter (-20°C - +10°C)
	bool set_ETWinter_speed(int ETWinterValue)
	{
    // Überprüfen, ob der Zuluftwert im zulässigen Bereich liegt
    if (ETWinterValue < -20 || ETWinterValue > 10)
    {
        ESP_LOGE("SET ETWinter", "ETWinter konnte nicht gesetzt werden.");
        ESP_LOGE("SET ETWinter", "Wert %i liegt außerhalb des zulässigen Bereichs: [-20 - 10]", ETWinterValue);
        return false;
    }

    // Konvertiere den ETWinter in einen String
    std::string valueStr = std::to_string(ETWinterValue);
    const char* data = valueStr.c_str();
    
    // Setze die ETWinter
    bool result = connector->write(ETWinter, data);
    
    if (result)
    {
        ETWinter_speed->update();
    }
    
    return result;
	}
	
	/// @brief Liefert den StartEWT (-3°C - +5°C)
	int get_StartEWT_speed()
	{
    return std::stoi(readLine(StartEWT));
	}

	/// @brief Setzt den StartEWT (-3°C - +5°C)
	bool set_StartEWT_speed(int StartEWTValue)
	{
    // Überprüfen, ob der Zuluftwert im zulässigen Bereich liegt
    if (StartEWTValue < -3 || StartEWTValue > 5)
    {
        ESP_LOGE("SET StartEWT", "StartEWT konnte nicht gesetzt werden.");
        ESP_LOGE("SET StartEWT", "Wert %i liegt außerhalb des zulässigen Bereichs: [-3 - 5]", StartEWTValue);
        return false;
    }

    // Konvertiere den StartEWT in einen String
    std::string valueStr = std::to_string(StartEWTValue);
    const char* data = valueStr.c_str();
    
    // Setze die StartEWT
    bool result = connector->write(StartEWT, data);
    
    if (result)
    {
        StartEWT_speed->update();
    }
    
    return result;
	}

	/// @brief Liefert den StopETW (-5°C - 15°C)
	int get_StopETW_speed()
	{
    return std::stoi(readLine(StopETW));
	}

	/// @brief Setzt den StopETW (-5°C - +15°C)
	bool set_StopETW_speed(int StopETWValue)
	{
    // Überprüfen, ob der Zuluftwert im zulässigen Bereich liegt
    if (StopETWValue < -5 || StopETWValue > 15)
    {
        ESP_LOGE("SET StopETW", "StopETW konnte nicht gesetzt werden.");
        ESP_LOGE("SET StopETW", "Wert %i liegt außerhalb des zulässigen Bereichs: [-5 - 15]", StopETWValue);
        return false;
    }

    // Konvertiere den StopETW in einen String
    std::string valueStr = std::to_string(StopETWValue);
    const char* data = valueStr.c_str();
    
    // Setze die StopETW
    bool result = connector->write(StopETW, data);
    
    if (result)
    {
        StopETW_speed->update();
    }
    
    return result;
	}
	
	/// @brief Liefert den Sommerstop (+5°C - +30°C)
	int get_Sommerstop_speed()
	{
    return std::stoi(readLine(Sommerstop));
	}

	/// @brief Setzt den Sommerstop (+5°C - +30°C)
	bool set_Sommerstop_speed(int SommerstopValue)
	{
    // Überprüfen, ob der Zuluftwert im zulässigen Bereich liegt
    if (SommerstopValue < 5 || SommerstopValue > 30)
    {
        ESP_LOGE("SET Sommerstop", "Sommerstop konnte nicht gesetzt werden.");
        ESP_LOGE("SET Sommerstop", "Wert %i liegt außerhalb des zulässigen Bereichs: [5 - 30]", SommerstopValue);
        return false;
    }

    // Konvertiere den Sommerstop in einen String
    std::string valueStr = std::to_string(SommerstopValue);
    const char* data = valueStr.c_str();
    
    // Setze die Sommerstop
    bool result = connector->write(Sommerstop, data);
    
    if (result)
    {
        Sommerstop_speed->update();
    }
    
    return result;
	}

/*	/// @brief Liefert den ZuluftSollRe (+15°C - +40°C)
	int get_ZuluftSollRe_speed()
	{
    return std::stoi(readLine(ZuluftSollRe));
	}

	/// @brief Setzt den ZuluftSollRe (0°C - +35°C)
	bool set_ZuluftSollRe_speed(int ZuluftSollReValue)
	{
    // Überprüfen, ob der Zuluftwert im zulässigen Bereich liegt
    if (ZuluftSollReValue < 0 || ZuluftSollReValue > 35)
    {
        ESP_LOGE("SET ZuluftSollRe", "ZuluftSollRe konnte nicht gesetzt werden.");
        ESP_LOGE("SET ZuluftSollRe", "Wert %i liegt außerhalb des zulässigen Bereichs: [0 - 35]", ZuluftSollReValue);
        return false;
    }

    // Konvertiere den ZuluftSollRe in einen String
    std::string valueStr = std::to_string(ZuluftSollReValue);
    const char* data = valueStr.c_str();
    
    // Setze die ZuluftSollRe
    bool result = connector->write(ZuluftSollRe, data);
    
    if (result)
    {
        ZuluftSollRe_speed->update();
    }
    
    return result;
	}

	/// @brief Liefert den RaumsollRd (+15°C - +40°C)
	int get_RaumsollRd_speed()
	{
    return std::stoi(readLine(RaumsollRd));
	}

	/// @brief Setzt den RaumsollRd (0°C - +35°C)
	bool set_RaumsollRd_speed(int RaumsollRdValue)
	{
    // Überprüfen, ob der Zuluftwert im zulässigen Bereich liegt
    if (RaumsollRdValue < 0 || RaumsollRdValue > 35)
    {
        ESP_LOGE("SET RaumsollRd", "RaumsollRd konnte nicht gesetzt werden.");
        ESP_LOGE("SET RaumsollRd", "Wert %i liegt außerhalb des zulässigen Bereichs: [0 - 35]", RaumsollRdValue);
        return false;
    }

    // Konvertiere den RaumsollRd in einen String
    std::string valueStr = std::to_string(RaumsollRdValue);
    const char* data = valueStr.c_str();
    
    // Setze die RaumsollRd
    bool result = connector->write(RaumsollRd, data);
    
    if (result)
    {
        RaumsollRd_speed->update();
    }
    
    return result;
	}

	/// @brief Liefert den ZuluftSoll (+15°C - +40°C)
	int get_ZuluftSoll_speed()
	{
    return std::stoi(readLine(ZuluftSoll));
	}

	/// @brief Setzt den ZuluftSoll (0°C - +35°C)
	bool set_ZuluftSoll_speed(int ZuluftSollValue)
	{
    // Überprüfen, ob der Zuluftwert im zulässigen Bereich liegt
    if (ZuluftSollValue < 0 || ZuluftSollValue > 35)
    {
        ESP_LOGE("SET ZuluftSoll", "ZuluftSoll konnte nicht gesetzt werden.");
        ESP_LOGE("SET ZuluftSoll", "Wert %i liegt außerhalb des zulässigen Bereichs: [0 - 35]", ZuluftSollValue);
        return false;
    }

    // Konvertiere den ZuluftSoll in einen String
    std::string valueStr = std::to_string(ZuluftSollValue);
    const char* data = valueStr.c_str();
    
    // Setze die ZuluftSoll
    bool result = connector->write(ZuluftSoll, data);
    
    if (result)
    {
        ZuluftSoll_speed->update();
    }
    
    return result;
	}*/

    void set_update_interval(uint32_t update_interval) override
    {
      if(update_interval > 20000)
        update_interval = 20000;
      else if (update_interval < 1000)
        update_interval = 1000;
      
      this->update_interval_ = update_interval;
    }

    void setup() override 
    { 
      // Declare a service "hello_world"
      //  - Service will be called "esphome.<NODE_NAME>_hello_world" in Home Assistant.
      //  - The service has no arguments
      //  - The function on_hello_world declared below will attached to the service.
      register_service(&WR3223Controller::Refresh_Relais, "Refresh_Relais");
      register_service(&WR3223Controller::Config_Restore, "Config_Restore");
      register_service(&WR3223Controller::Config_Save, "Config_Save");
    }

    void Refresh_Relais() 
    {
      relaisDecoder->publish_relais(readLine(RelaisCmd).c_str());
    }

    void Config_Restore()
    {

      ESP_LOGI("Test_Read", "Status SW bit content: %s", readLine(StatusWriteCmd).c_str());
      ESP_LOGI("Test_Read", "Status Ta bit content: %s", readLine(StatusUpdateCmd).c_str());
      statusHolder->restore_state_sw();
      //ESP_LOGD("MODUS", "L1: %s", readLine(TestCmd1));
      //ESP_LOGD("MODUS", "L2: %s", readLine(TestCmd2));
      //ESP_LOGD("MODUS", "L3: %s", readLine(TestCmd3));
    }

    void Config_Save()
    {
      statusHolder->save_state_sw();
      //char const* data = std::to_string(60).c_str();
      //connector->write(TestCmd1, data);
    }

    void Show(std::string pShow)
    {
      ESP_LOGD("MODUS", "SHOW: %s", pShow.c_str());
    }

    /// @brief Liefert den Betriebsmodus
    /// @return 
    int Get_Modus()
    {
      int result = modeHolder->get_active_mode();
      ESP_LOGD("MODUS", "Get_Modus: %i", result);
      return result;
    }

    /// @brief Setzt den Betriebsmodus
    /// @param pMode 
    bool Set_Modus(esphome::optional<unsigned int> pMode)
    { 
      if(pMode.has_value() == false)
      {
        ESP_LOGD("MODUS", "Modus_Set: NO VALUE");
        return false;
      }
      else
      {

        ESP_LOGD("MODUS", "Modus_Set: %i", (int)pMode.value());
        ESP_LOGD("MODUS", "Modus_Set: %s", modeHolder->getModeDisplayValue((int)pMode.value()));

        char const* data = std::to_string((int)pMode.value()).c_str();
        return connector->write(ModusCmd, data); // return write ACK ergebnis compare
      }
    }

    int Get_Luftstufe()
    {
      return std::stoi(readLine(LufstufeCmd));
    }    

#pragma region SW (StatusWrite Commands)

    bool Set_Luftstufe(esphome::optional<unsigned int> pLevel)
    {
      if(pLevel.has_value() == false)
      {
        ESP_LOGD("VENT_LEVEL", "Set_Luftstufe: NO VALUE");
        return false;
      }
      if (pLevel.value() < 0 || pLevel.value() > 3) 
      {
        ESP_LOGD("VENT_LEVEL", "Set_Luftstufe: Level %i außerhalb des gültigen Bereichs [0-3]", pLevel.value());
        return false;
      }      

      if((int)pLevel.value() == Get_Luftstufe())
      {
        ESP_LOGD("VENT_LEVEL", "Set_Luftstufe: Die Aktuelle Luftstufe ist bereits Stufe %i", pLevel.value());
        return true;
      }

      statusHolder->setVentilationLevel((int)pLevel.value());
      return write_current_status_sw();
    }

    bool Get_Zusatzheizung_On()
    {
      return statusHolder->getAdditionalHeatingOnStatus();
    }
    
    bool Set_Zusatzheizung_On(bool pOn)
    {
      // wenn der Status schon stimmt, dann brauchen wir nicht noch einmal zu setzen
      if(statusHolder->getAdditionalHeatingOnStatus() == pOn)
        return true;

      statusHolder->setAdditionalHeatingOn(pOn);
      char const* data = std::to_string((int)statusHolder->getSwStatus()).c_str();
      if(connector->write(StatusWriteCmd, data))
        return true;
      
      statusHolder->setAdditionalHeatingOn(!pOn);
      return write_current_status_sw();
    }

    bool Get_Waermepunpe_On()
    {
      return statusHolder->getHeatPumpOnStatus();
    }
    
    bool Set_Waermepunpe_On(bool pOn)
    {
      // wenn der Status schon stimmt, dann brauchen wir nicht noch einmal zu setzen
      if(statusHolder->getHeatPumpOnStatus() == pOn)
        return true;

      statusHolder->setHeatPumpOn(pOn);
      char const* data = std::to_string((int)statusHolder->getSwStatus()).c_str();
      if(connector->write(StatusWriteCmd, data))
        return true;
      
      statusHolder->setHeatPumpOn(!pOn);
      return write_current_status_sw();
    }

// wo wir den 'echten' status der Kühlung herbekommen ist noch unklar?!?! 
// evtl. aus der Wärmepumpe, wenn sommermodus aktiv ist?!
    bool Get_Kuehlung_On()
    {
      return statusHolder->getCoolingOnStatus();
    }
    
    bool Set_Kuehlung_On(bool pOn)
    {
      // den Status können wir derzeit nicht ermitteln
      // // wenn der Status schon stimmt, dann brauchen wir nicht noch einmal zu setzen
      // if(statusHolder->getCoolingOnStatus() == pOn)
      //   return true;


      statusHolder->setCoolingOn(pOn);
      char const* data = std::to_string((int)statusHolder->getSwStatus()).c_str();
      if(connector->write(StatusWriteCmd, data))
        return true;
      
      statusHolder->setCoolingOn(!pOn);
      return false;
    }
    
#pragma endregion
  
    bool write_current_status_sw()
    {
      // try 
      // {
      //   float test = 18;
      //   char const* sollTemp = std::to_string(test).substr(0,4).c_str();
      //   connector->write(ZuluftSoll, sollTemp);
      //   connector->write(ZuluftSollRe, sollTemp);
      //   connector->write(RaumsollRd, sollTemp);
      //   connector->write(SommerstopEs, sollTemp);
      // }
      // catch(const std::exception& e)
      // {
      //   ESP_LOGE("SOLLTEMP", "Fehler beim schreiben der Daten: %s", e.what()); 
      // }    
      
      char const* data = std::to_string((int)statusHolder->getSwStatus()).c_str();
      return connector->write(StatusWriteCmd, data);
    }

    void update() override 
    {

      if(freshStart == true)
      {
        // das machen wir nur einmal zu beginn
        freshStart = false;
        Refresh_Relais();
        statusHolder->restore_state_sw();
        write_current_status_sw();

        vent_level_1_speed->update();
        vent_level_2_speed->update();
        vent_level_3_speed->update();
		zuluft_speed->update();
		abluft_speed->update();
		ETSommer_speed->update();
		ETWinter_speed->update();
		StartEWT_speed->update();
		StopETW_speed->update();
		Sommerstop_speed->update();
      }
      else
        write_current_status_sw();      

      // ESP_LOGI("INFO", "AbtauStartCmd: %s", readLine(AbtauStartCmd).c_str());
      // ESP_LOGI("INFO", "AbtauEndeCmd: %s", readLine(AbtauEndeCmd).c_str());
      // ESP_LOGI("INFO", "AbtauLuftstufeCmd: %s", readLine(AbtauLuftstufeCmd).c_str());
      // ESP_LOGI("INFO", "AbtauPauseCmd: %s", readLine(AbtauPauseCmd).c_str());
      // ESP_LOGI("INFO", "AbtauNachlauzeitCmd: %s", readLine(AbtauNachlauzeitCmd).c_str());
      // ESP_LOGI("INFO", "AbtauRueckwaermezahlCmd: %s", readLine(AbtauRueckwaermezahlCmd).c_str());

      for (int i = 0; i < cmdPairCount; i++)
      {
        CommandPair cmd = Commands[i];
        std::string resdata = readLine(cmd);
        if(cmd.publish_data(resdata) == false)
          ESP_LOGE("FINAL", "FAILED Command: %s Data: %s", cmd.cmd, resdata.c_str());  
      }
      // currentCommand++;
    }

    std::string readLine(const CommandPair cmd)
    {
      const int max_line_length = 15;
      static char answer[max_line_length];
      
      ESP_LOGI("START", "Command: %s", cmd.cmd);

      int dataLength = connector->readLine(answer, max_line_length, cmd);

      if(dataLength < 1)
        return "";

      char contentData[dataLength];
      std::copy(answer + data_start, answer + data_start + dataLength, contentData);
      contentData[dataLength] = 0;

      return std::string(contentData);
    }
  };
}

