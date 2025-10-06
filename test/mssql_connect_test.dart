import 'package:flutter_test/flutter_test.dart';
import 'package:mssql_connect/mssql_connect.dart';
import 'package:mssql_connect/mssql_connect_platform_interface.dart';
import 'package:mssql_connect/mssql_connect_method_channel.dart';
import 'package:plugin_platform_interface/plugin_platform_interface.dart';

class MockMssqlConnectPlatform
    with MockPlatformInterfaceMixin
    implements MssqlConnectPlatform {

  @override
  Future<String?> getPlatformVersion() => Future.value('42');
}

void main() {
  final MssqlConnectPlatform initialPlatform = MssqlConnectPlatform.instance;

  test('$MethodChannelMssqlConnect is the default instance', () {
    expect(initialPlatform, isInstanceOf<MethodChannelMssqlConnect>());
  });

  test('getPlatformVersion', () async {
    MssqlConnect mssqlConnectPlugin = MssqlConnect();
    MockMssqlConnectPlatform fakePlatform = MockMssqlConnectPlatform();
    MssqlConnectPlatform.instance = fakePlatform;

    expect(await mssqlConnectPlugin.getPlatformVersion(), '42');
  });
}
