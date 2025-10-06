#ifndef FLUTTER_PLUGIN_MSSQL_CONNECT_PLUGIN_C_H_
#define FLUTTER_PLUGIN_MSSQL_CONNECT_PLUGIN_C_H_

#include <flutter_plugin_registrar.h>

// Fix: Properly define the export macro
#ifdef FLUTTER_PLUGIN_IMPL
#define FLUTTER_PLUGIN_EXPORT __declspec(dllexport)
#else
#define FLUTTER_PLUGIN_EXPORT __declspec(dllimport)
#endif

#if defined(__cplusplus)
extern "C" {
#endif

// Fix: Correct syntax for function declaration
FLUTTER_PLUGIN_EXPORT void MssqlConnectPluginCApiRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar);

#if defined(__cplusplus)
}  // extern "C"
#endif

#endif  // FLUTTER_PLUGIN_MSSQL_CONNECT_PLUGIN_C_H_