#include "mssql_connect_plugin.h"
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>

namespace mssql_connect {

// Static member definitions
int MssqlConnectPlugin::next_connection_id_ = 0;
std::unordered_map<int, void*> MssqlConnectPlugin::connections_;

// Helper function to convert string to wide string
std::wstring MssqlConnectPlugin::StringToWString(const std::string& str) {
    if (str.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

// Helper function to convert wide string to string
std::string MssqlConnectPlugin::WStringToString(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

// Helper function to get string from map
std::string MssqlConnectPlugin::GetStringFromMap(const flutter::EncodableMap& map, const char* key) {
    std::string key_str(key);
    flutter::EncodableValue key_value(key_str);
    
    auto it = map.find(key_value);
    if (it != map.end() && std::holds_alternative<std::string>(it->second)) {
        return std::get<std::string>(it->second);
    }
    return "";
}

// Helper function to get int from map
int MssqlConnectPlugin::GetIntFromMap(const flutter::EncodableMap& map, const char* key, int default_value) {
    std::string key_str(key);
    flutter::EncodableValue key_value(key_str);
    
    auto it = map.find(key_value);
    if (it != map.end() && std::holds_alternative<int>(it->second)) {
        return std::get<int>(it->second);
    }
    return default_value;
}

// Helper function to get bool from map
bool MssqlConnectPlugin::GetBoolFromMap(const flutter::EncodableMap& map, const char* key, bool default_value) {
    std::string key_str(key);
    flutter::EncodableValue key_value(key_str);
    
    auto it = map.find(key_value);
    if (it != map.end() && std::holds_alternative<bool>(it->second)) {
        return std::get<bool>(it->second);
    }
    return default_value;
}

// Constructor
MssqlConnectPlugin::MssqlConnectPlugin() {}

// Destructor
MssqlConnectPlugin::~MssqlConnectPlugin() {}

// Static method to register the plugin
void MssqlConnectPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows *registrar) {
  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), "mssql_connect",  // Make sure this matches
          &flutter::StandardMethodCodec::GetInstance());

  auto plugin = std::make_unique<MssqlConnectPlugin>();

  channel->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto &call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  registrar->AddPlugin(std::move(plugin));
}

// Method call handler
void MssqlConnectPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue> &method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    
  const std::string &method_name = method_call.method_name();

  if (method_name == "getPlatformVersion") {
    // Handle the platform version request
    result->Success(flutter::EncodableValue("Windows"));
  } else if (method_name == "connect") {
    Connect(method_call, std::move(result));
  } else if (method_name == "disconnect") {
    Disconnect(method_call, std::move(result));
  } else if (method_name == "query") {
    Query(method_call, std::move(result));
  } else if (method_name == "execute") {
    Execute(method_call, std::move(result));
  } else if (method_name == "testConnection") {
    TestConnection(method_call, std::move(result));
  } else {
    result->NotImplemented();
  }
}

// Connect method implementation
void MssqlConnectPlugin::Connect(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    
  if (!method_call.arguments() || !std::holds_alternative<flutter::EncodableMap>(*method_call.arguments())) {
    result->Error("InvalidArguments", "Arguments must be a map");
    return;
  }

  const flutter::EncodableMap& args = std::get<flutter::EncodableMap>(*method_call.arguments());
  
  std::string server = GetStringFromMap(args, "server");
  std::string database = GetStringFromMap(args, "database");
  std::string username = GetStringFromMap(args, "username");
  std::string password = GetStringFromMap(args, "password");

  // Here you would implement the actual SQL Server connection logic
  // For now, we'll just simulate a successful connection
  int connection_id = ++next_connection_id_;
  
  // In a real implementation, you would store the actual connection object
  connections_[connection_id] = nullptr; // Placeholder for actual connection
  
  flutter::EncodableMap response;
  response[flutter::EncodableValue("connectionId")] = flutter::EncodableValue(connection_id);
  response[flutter::EncodableValue("success")] = flutter::EncodableValue(true);
  
  result->Success(flutter::EncodableValue(response));
}

// Disconnect method implementation
void MssqlConnectPlugin::Disconnect(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    
  if (!method_call.arguments() || !std::holds_alternative<flutter::EncodableMap>(*method_call.arguments())) {
    result->Error("InvalidArguments", "Arguments must be a map");
    return;
  }

  const flutter::EncodableMap& args = std::get<flutter::EncodableMap>(*method_call.arguments());
  int connectionId = GetIntFromMap(args, "connectionId", -1);

  if (connectionId < 0 || connections_.find(connectionId) == connections_.end()) {
    result->Error("InvalidConnection", "Invalid connection ID");
    return;
  }

  // Here you would implement the actual disconnection logic
  connections_.erase(connectionId);
  
  flutter::EncodableMap response;
  response[flutter::EncodableValue("success")] = flutter::EncodableValue(true);
  
  result->Success(flutter::EncodableValue(response));
}

// Query method implementation
void MssqlConnectPlugin::Query(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    
  if (!method_call.arguments() || !std::holds_alternative<flutter::EncodableMap>(*method_call.arguments())) {
    result->Error("InvalidArguments", "Arguments must be a map");
    return;
  }

  const flutter::EncodableMap& args = std::get<flutter::EncodableMap>(*method_call.arguments());
  int connectionId = GetIntFromMap(args, "connectionId", -1);
  std::string sql = GetStringFromMap(args, "sql");

  if (connectionId < 0 || connections_.find(connectionId) == connections_.end()) {
    result->Error("InvalidConnection", "Invalid connection ID");
    return;
  }

  if (sql.empty()) {
    result->Error("InvalidQuery", "SQL query cannot be empty");
    return;
  }

  // Here you would implement the actual query execution logic
  // For now, we'll return mock data
  
  flutter::EncodableMap response;
  response[flutter::EncodableValue("success")] = flutter::EncodableValue(true);
  response[flutter::EncodableValue("rowCount")] = flutter::EncodableValue(0);
  
  flutter::EncodableList rows;
  response[flutter::EncodableValue("rows")] = rows;
  
  flutter::EncodableList columnNames;
  response[flutter::EncodableValue("columnNames")] = columnNames;
  
  result->Success(flutter::EncodableValue(response));
}

// Execute method implementation
void MssqlConnectPlugin::Execute(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    
  if (!method_call.arguments() || !std::holds_alternative<flutter::EncodableMap>(*method_call.arguments())) {
    result->Error("InvalidArguments", "Arguments must be a map");
    return;
  }

  const flutter::EncodableMap& args = std::get<flutter::EncodableMap>(*method_call.arguments());
  int connectionId = GetIntFromMap(args, "connectionId", -1);
  std::string sql = GetStringFromMap(args, "sql");

  if (connectionId < 0 || connections_.find(connectionId) == connections_.end()) {
    result->Error("InvalidConnection", "Invalid connection ID");
    return;
  }

  if (sql.empty()) {
    result->Error("InvalidCommand", "SQL command cannot be empty");
    return;
  }

  // Here you would implement the actual command execution logic
  // For now, we'll return mock data
  
  flutter::EncodableMap response;
  response[flutter::EncodableValue("success")] = flutter::EncodableValue(true);
  response[flutter::EncodableValue("affectedRows")] = flutter::EncodableValue(0);
  
  result->Success(flutter::EncodableValue(response));
}

// Test connection method implementation
void MssqlConnectPlugin::TestConnection(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    
  if (!method_call.arguments() || !std::holds_alternative<flutter::EncodableMap>(*method_call.arguments())) {
    result->Error("InvalidArguments", "Arguments must be a map");
    return;
  }

  const flutter::EncodableMap& args = std::get<flutter::EncodableMap>(*method_call.arguments());
  
  std::string server = GetStringFromMap(args, "server");
  std::string database = GetStringFromMap(args, "database");
  std::string username = GetStringFromMap(args, "username");
  std::string password = GetStringFromMap(args, "password");

  // Here you would implement the actual connection test logic
  // For now, we'll just simulate a successful test
  
  result->Success(flutter::EncodableValue(true));
}

}  // namespace mssql_connect

// Function called by Flutter to register the plugin
void MssqlConnectPluginCApiRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  mssql_connect::MssqlConnectPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}