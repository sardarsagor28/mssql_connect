export 'src/connection.dart';
export 'src/query_result.dart';
export 'src/exceptions.dart';
import 'mssql_connect_platform_interface.dart';

class MssqlConnect {
  Future<String?> getPlatformVersion() {
    return MssqlConnectPlatform.instance.getPlatformVersion();
  }
}
