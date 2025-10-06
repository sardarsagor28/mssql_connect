#include "include/mssql_connect/mssql_connect_plugin_c_api.h"

#include <flutter/plugin_registrar_windows.h>

#include "mssql_connect_plugin.h"

void MssqlConnectPluginCApiRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  mssql_connect::MssqlConnectPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
