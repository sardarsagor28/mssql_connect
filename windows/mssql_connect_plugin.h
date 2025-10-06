#ifndef FLUTTER_PLUGIN_MSSQL_CONNECT_PLUGIN_H_
#define FLUTTER_PLUGIN_MSSQL_CONNECT_PLUGIN_H_

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>
#include <flutter/encodable_value.h>

#include <memory>
#include <string>
#include <unordered_map>

namespace mssql_connect {

class MssqlConnectPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

  MssqlConnectPlugin();

  virtual ~MssqlConnectPlugin();

 private:
  // Called when a method is called on this plugin's channel from Dart.
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

  // Helper methods
  static std::string GetStringFromMap(const flutter::EncodableMap& map, const char* key);
  static int GetIntFromMap(const flutter::EncodableMap& map, const char* key, int default_value);
  static bool GetBoolFromMap(const flutter::EncodableMap& map, const char* key, bool default_value);
  
  // Connection management
  static std::wstring StringToWString(const std::string& str);
  static std::string WStringToString(const std::wstring& wstr);

  // Method implementations
  void Connect(const flutter::MethodCall<flutter::EncodableValue>& method_call,
               std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  void Disconnect(const flutter::MethodCall<flutter::EncodableValue>& method_call,
                  std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  void Query(const flutter::MethodCall<flutter::EncodableValue>& method_call,
             std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  void Execute(const flutter::MethodCall<flutter::EncodableValue>& method_call,
               std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  void TestConnection(const flutter::MethodCall<flutter::EncodableValue>& method_call,
                      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

  // Connection tracking
  static int next_connection_id_;
  static std::unordered_map<int, void*> connections_;
};

}  // namespace mssql_connect

#endif  // FLUTTER_PLUGIN_MSSQL_CONNECT_PLUGIN_H_