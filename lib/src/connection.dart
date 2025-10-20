import 'dart:async';
import 'package:flutter/services.dart';
import 'query_result.dart';
import 'exceptions.dart';

/// Main class for managing MS SQL Server connections
class MsSqlConnection {
  // Fix: Use consistent method channel name
  static const MethodChannel _channel = MethodChannel('mssql_connect');

  final String server;
  final String database;
  final String? username;
  final String? password;
  final int port;
  final bool trustedConnection;

  bool _isConnected = false;
  int? _connectionId;

  MsSqlConnection({
    required this.server,
    required this.database,
    this.username,
    this.password,
    this.port = 1433,
    this.trustedConnection = false,
  });

  /// Connect to the database
  Future<bool> connect() async {
    if (_isConnected) {
      throw ConnectionException('Already connected to database');
    }

    try {
      final result = await _channel.invokeMethod('connect', {
        'server': server,
        'database': database,
        'username': username ?? '',
        'password': password ?? '',
        'port': port,
        'trustedConnection': trustedConnection,
      });

      if (result is Map) {
        _isConnected = result['success'] == true;
        _connectionId = result['connectionId'];
        return _isConnected;
      }

      return false;
    } on PlatformException catch (e) {
      throw ConnectionException(
        'Failed to connect to database',
        details: e.details as String?,
      );
    }
  }

  /// Execute a SELECT query
  Future<QueryResult> query(String sql, [List<dynamic>? parameters]) async {
    _ensureConnected();

    try {
      final result = await _channel.invokeMethod('query', {
        'connectionId': _connectionId,
        'sql': sql,
        'parameters': parameters ?? [],
      });

      if (result is Map) {
        return QueryResult.fromJson(result);
      }

      throw QueryException('Invalid query result format');
    } on PlatformException catch (e) {
      throw QueryException('Query execution failed', details: e.details as String?);
    }
  }

  /// Execute INSERT, UPDATE, DELETE commands
  Future<int> execute(String sql, [List<dynamic>? parameters]) async {
    _ensureConnected();

    try {
      final result = await _channel.invokeMethod('execute', {
        'connectionId': _connectionId,
        'sql': sql,
        'parameters': parameters ?? [],
      });

      return result as int? ?? 0;
    } on PlatformException catch (e) {
      throw QueryException('Execute command failed', details: e.details as String?);
    }
  }

  /// Execute a stored procedure
  Future<QueryResult> executeStoredProcedure(
    String procedureName,
    Map<String, dynamic> parameters,
  ) async {
    _ensureConnected();

    try {
      final result = await _channel.invokeMethod('executeStoredProcedure', {
        'connectionId': _connectionId,
        'procedureName': procedureName,
        'parameters': parameters,
      });

      if (result is Map) {
        return QueryResult.fromJson(result);
      }

      throw QueryException('Invalid stored procedure result format');
    } on PlatformException catch (e) {
      throw QueryException(
        'Stored procedure execution failed',
        details: e.details as String?,
      );
    }
  }

  /// Disconnect from the database
  Future<void> disconnect() async {
    if (!_isConnected) {
      return;
    }

    try {
      await _channel.invokeMethod('disconnect', {
        'connectionId': _connectionId,
      });
      _isConnected = false;
      _connectionId = null;
    } on PlatformException catch (e) {
      throw ConnectionException('Failed to disconnect', details: e.details as String?);
    }
  }

  /// Test the connection
  Future<bool> testConnection() async {
    try {
      final result = await _channel.invokeMethod('testConnection', {
        'server': server,
        'database': database,
        'username': username ?? '',
        'password': password ?? '',
        'port': port,
        'trustedConnection': trustedConnection,
      });

      return result == true;
    } on PlatformException catch (e) {
      throw ConnectionException(
        'Connection test failed',
        details: e.details as String?,
      );
    }
  }

  /// Check if connected
  bool get isConnected => _isConnected;

  /// Ensure connection is active
  void _ensureConnected() {
    if (!_isConnected) {
      throw ConnectionException(
        'Not connected to database. Call connect() first.',
      );
    }
  }

  /// Close connection on dispose
  Future<void> dispose() async {
    if (_isConnected) {
      await disconnect();
    }
  }
}
