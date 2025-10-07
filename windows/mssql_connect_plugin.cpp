#include "mssql_connect_plugin.h"
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>
#include <memory>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <windows.h>
#include <sql.h>
#include <sqlext.h>

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

  SQLHENV hEnv = SQL_NULL_HENV;
  SQLHDBC hDbc = SQL_NULL_HDBC;
  SQLRETURN ret;

  ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
  if (!SQL_SUCCEEDED(ret)) {
      result->Error("ConnectionError", "Failed to allocate environment handle");
      return;
  }

  ret = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
  if (!SQL_SUCCEEDED(ret)) {
      SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
      result->Error("ConnectionError", "Failed to set ODBC version");
      return;
  }

  ret = SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
  if (!SQL_SUCCEEDED(ret)) {
      SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
      result->Error("ConnectionError", "Failed to allocate connection handle");
      return;
  }

  std::wstringstream conn_str_ss;
  conn_str_ss << L"DRIVER={ODBC Driver 18 for SQL Server};SERVER=" << StringToWString(server)
              << L";DATABASE=" << StringToWString(database)
              << L";UID=" << StringToWString(username)
              << L";PWD=" << StringToWString(password)
              << L";TrustServerCertificate=Yes;";
  std::wstring conn_str = conn_str_ss.str();

  wchar_t out_conn_str[1024];
  SQLSMALLINT out_conn_str_len;

  ret = SQLDriverConnect(hDbc, NULL, (SQLWCHAR*)conn_str.c_str(), SQL_NTS,
                         out_conn_str, sizeof(out_conn_str) / sizeof(wchar_t),
                         &out_conn_str_len, SQL_DRIVER_NOPROMPT);

  if (SQL_SUCCEEDED(ret)) {
      int connection_id = ++next_connection_id_;
      connections_[connection_id] = hDbc;

      flutter::EncodableMap response;
      response[flutter::EncodableValue("connectionId")] = flutter::EncodableValue(connection_id);
      response[flutter::EncodableValue("success")] = flutter::EncodableValue(true);
      result->Success(flutter::EncodableValue(response));
  } else {
      SQLWCHAR sqlstate[6];
      SQLINTEGER native_error;
      SQLWCHAR message_text[SQL_MAX_MESSAGE_LENGTH];
      SQLSMALLINT text_length;
      SQLGetDiagRec(SQL_HANDLE_DBC, hDbc, 1, sqlstate, &native_error, message_text, SQL_MAX_MESSAGE_LENGTH, &text_length);
      std::string error_message = WStringToString(message_text);

      SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
      SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
      result->Error("ConnectionError", "Failed to connect to database", flutter::EncodableValue(error_message));
  }
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

  SQLHDBC hDbc = (SQLHDBC)connections_[connectionId];
  SQLDisconnect(hDbc);
  SQLFreeHandle(SQL_HANDLE_DBC, hDbc);

  connections_.erase(connectionId);
  
  flutter::EncodableMap response;
  response[flutter::EncodableValue("success")] = flutter::EncodableValue(true);
  result->Success(flutter::EncodableValue(true));
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

  SQLHDBC hDbc = (SQLHDBC)connections_[connectionId];
  SQLHSTMT hStmt = SQL_NULL_HSTMT;
  SQLRETURN ret;

  ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
  if (!SQL_SUCCEEDED(ret)) {
      result->Error("QueryError", "Failed to allocate statement handle");
      return;
  }

  std::wstring wsql = StringToWString(sql);
  ret = SQLExecDirect(hStmt, (SQLWCHAR*)wsql.c_str(), SQL_NTS);

  if (SQL_SUCCEEDED(ret)) {
      SQLSMALLINT num_cols;
      SQLNumResultCols(hStmt, &num_cols);

      flutter::EncodableList columnNames;
      std::vector<SQLWCHAR> col_name_buffer(256);
      for (SQLSMALLINT i = 1; i <= num_cols; ++i) {
          SQLDescribeCol(hStmt, i, col_name_buffer.data(), (SQLSMALLINT)col_name_buffer.size(), NULL, NULL, NULL, NULL, NULL);
          columnNames.push_back(flutter::EncodableValue(WStringToString(col_name_buffer.data())));
      }

      flutter::EncodableList rows;
      SQLLEN row_count = 0;
      while (SQL_SUCCEEDED(SQLFetch(hStmt))) {
          row_count++;
          flutter::EncodableMap row;
          for (SQLSMALLINT i = 1; i <= num_cols; ++i) {
              SQLWCHAR data[1024];
              SQLLEN indicator;
              ret = SQLGetData(hStmt, i, SQL_C_WCHAR, data, sizeof(data), &indicator);
              if (SQL_SUCCEEDED(ret)) {
                  if (indicator == SQL_NULL_DATA) {
                      row[columnNames[i - 1]] = flutter::EncodableValue();
                  } else {
                      row[columnNames[i - 1]] = flutter::EncodableValue(WStringToString(data));
                  }
              }
          }
          rows.push_back(flutter::EncodableValue(row));
      }

      flutter::EncodableMap response;
      response[flutter::EncodableValue("rows")] = rows;
      response[flutter::EncodableValue("rowCount")] = (int)row_count;
      response[flutter::EncodableValue("columns")] = columnNames;

      result->Success(flutter::EncodableValue(response));
  } else {
      SQLWCHAR sqlstate[6];
      SQLINTEGER native_error;
      SQLWCHAR message_text[SQL_MAX_MESSAGE_LENGTH];
      SQLSMALLINT text_length;
      SQLGetDiagRec(SQL_HANDLE_STMT, hStmt, 1, sqlstate, &native_error, message_text, SQL_MAX_MESSAGE_LENGTH, &text_length);
      std::string error_message = WStringToString(message_text);
      result->Error("QueryError", "Query execution failed", flutter::EncodableValue(error_message));
  }

  SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
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

  SQLHDBC hDbc = (SQLHDBC)connections_[connectionId];
  SQLHSTMT hStmt = SQL_NULL_HSTMT;
  SQLRETURN ret;

  ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
  if (!SQL_SUCCEEDED(ret)) {
      result->Error("ExecuteError", "Failed to allocate statement handle");
      return;
  }

  std::wstring wsql = StringToWString(sql);
  ret = SQLExecDirect(hStmt, (SQLWCHAR*)wsql.c_str(), SQL_NTS);

  if (SQL_SUCCEEDED(ret)) {
      SQLLEN affected_rows;
      SQLRowCount(hStmt, &affected_rows);
      result->Success(flutter::EncodableValue((int)affected_rows));
  } else {
      result->Error("ExecuteError", "Command execution failed");
  }

  SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
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

  SQLHENV hEnv = SQL_NULL_HENV;
  SQLHDBC hDbc = SQL_NULL_HDBC;
  SQLRETURN ret;

  ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
  if (!SQL_SUCCEEDED(ret)) {
      result->Success(flutter::EncodableValue(false));
      return;
  }

  ret = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
  if (!SQL_SUCCEEDED(ret)) {
      SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
      result->Success(flutter::EncodableValue(false));
      return;
  }

  ret = SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
  if (!SQL_SUCCEEDED(ret)) {
      SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
      result->Success(flutter::EncodableValue(false));
      return;
  }

  std::wstringstream conn_str_ss;
  conn_str_ss << L"DRIVER={ODBC Driver 18 for SQL Server};SERVER=" << StringToWString(server)
              << L";DATABASE=" << StringToWString(database)
              << L";UID=" << StringToWString(username)
              << L";PWD=" << StringToWString(password)
              << L";TrustServerCertificate=Yes;";
  std::wstring conn_str = conn_str_ss.str();

  ret = SQLDriverConnect(hDbc, NULL, (SQLWCHAR*)conn_str.c_str(), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

  if (SQL_SUCCEEDED(ret)) {
      SQLDisconnect(hDbc);
      SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
      SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
      result->Success(flutter::EncodableValue(true));
  } else {
      SQLWCHAR sqlstate[6];
      SQLINTEGER native_error;
      SQLWCHAR message_text[SQL_MAX_MESSAGE_LENGTH];
      SQLSMALLINT text_length;
      SQLGetDiagRec(SQL_HANDLE_DBC, hDbc, 1, sqlstate, &native_error, message_text, SQL_MAX_MESSAGE_LENGTH, &text_length);
      std::string error_message = WStringToString(message_text);
      result->Error("ConnectionError", "Connection test failed", flutter::EncodableValue(error_message));
  }
}

}  // namespace mssql_connect

// Function called by Flutter to register the plugin
void MssqlConnectPluginCApiRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  mssql_connect::MssqlConnectPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}