import 'package:plugin_platform_interface/plugin_platform_interface.dart';

import 'mssql_connect_method_channel.dart';

abstract class MssqlConnectPlatform extends PlatformInterface {
  /// Constructs a MssqlConnectPlatform.
  MssqlConnectPlatform() : super(token: _token);

  static final Object _token = Object();

  static MssqlConnectPlatform _instance = MethodChannelMssqlConnect();

  /// The default instance of [MssqlConnectPlatform] to use.
  ///
  /// Defaults to [MethodChannelMssqlConnect].
  static MssqlConnectPlatform get instance => _instance;

  /// Platform-specific implementations should set this with their own
  /// platform-specific class that extends [MssqlConnectPlatform] when
  /// they register themselves.
  static set instance(MssqlConnectPlatform instance) {
    PlatformInterface.verifyToken(instance, _token);
    _instance = instance;
  }

  Future<String?> getPlatformVersion() {
    throw UnimplementedError('platformVersion() has not been implemented.');
  }
}
