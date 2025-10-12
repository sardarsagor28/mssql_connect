//
//  Generated file. Do not edit.
//

// clang-format off

#include "generated_plugin_registrant.h"

#include <mssql_connect/mssql_connect_plugin.h>

void fl_register_plugins(FlPluginRegistry* registry) {
  g_autoptr(FlPluginRegistrar) mssql_connect_registrar =
      fl_plugin_registry_get_registrar_for_plugin(registry, "MssqlConnectPlugin");
  mssql_connect_plugin_register_with_registrar(mssql_connect_registrar);
}
