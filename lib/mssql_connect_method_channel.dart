import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

import 'mssql_connect_platform_interface.dart';

/// An implementation of [MssqlConnectPlatform] that uses method channels.
class MethodChannelMssqlConnect extends MssqlConnectPlatform {
  /// The method channel used to interact with the native platform.
  @visibleForTesting
  final methodChannel = const MethodChannel('mssql_connect');

  @override
  Future<String?> getPlatformVersion() async {
    final version = await methodChannel.invokeMethod<String>('getPlatformVersion');
    return version;
  }
}
