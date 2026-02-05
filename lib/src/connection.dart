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
  final bool encrypt;

  bool _isConnected = false;
  int? _connectionId;

  MsSqlConnection({
    required this.server,
    required this.database,
    this.username,
    this.password,
    this.port = 1433,
    this.trustedConnection = false,
    this.encrypt = true,
  });

  /// Connect to the database
  Future<bool> connect() async {
    // If already connected, try to disconnect first to clean up resources
    if (_isConnected) {
      try {
        await disconnect();
      } catch (e) {
        // Ignore disconnect errors during re-connect
      }
    }

    try {
      final result = await _channel.invokeMethod('connect', {
        'server': server,
        'database': database,
        'username': username ?? '',
        'password': password ?? '',
        'port': port,
        'trustedConnection': trustedConnection,
        'encrypt': encrypt,
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
      throw QueryException(
        'Query execution failed',
        details: e.details as String?,
      );
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
      throw QueryException(
        'Execute command failed',
        details: e.details as String?,
      );
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
      throw ConnectionException(
        'Failed to disconnect',
        details: e.details as String?,
      );
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
        'encrypt': encrypt,
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

  // ========== Change Tracking Methods ==========

  /// Get current change tracking version for the database
  /// Returns null if change tracking is not enabled
  ///
  /// Example:
  /// ```dart
  /// final version = await connection.getChangeTrackingVersion();
  /// if (version != null) {
  ///   print('Current version: $version');
  /// }
  /// ```
  Future<int?> getChangeTrackingVersion() async {
    _ensureConnected();

    try {
      final result = await query(
        'SELECT CHANGE_TRACKING_CURRENT_VERSION() as CurrentVersion',
      );

      if (result.rows.isEmpty) {
        return null;
      }

      final version = result.rows.first['CurrentVersion'];

      // Handle different return types (int, String, null)
      if (version == null) {
        return null;
      } else if (version is int) {
        return version;
      } else if (version is String) {
        return int.tryParse(version);
      }

      return null;
    } catch (e) {
      // Change tracking not enabled or error
      return null;
    }
  }

  /// Check if a specific table has changes since last version
  /// Returns true if changes detected, false otherwise
  ///
  /// Parameters:
  /// - [tableName]: Name of the table to check
  /// - [lastVersion]: Last known change tracking version
  ///
  /// Example:
  /// ```dart
  /// final hasChanges = await connection.hasTableChanges(
  ///   tableName: 'Orders',
  ///   lastVersion: 100,
  /// );
  /// if (hasChanges) {
  ///   // Fetch updated data
  /// }
  /// ```
  Future<bool> hasTableChanges({
    required String tableName,
    required int lastVersion,
  }) async {
    _ensureConnected();

    try {
      // Direct injection for lastVersion as it is an integer (safe)
      // Using parameterized query for table name is not supported in T-SQL for CHANGETABLE
      // But table name comes from our internal logic usually.
      // NOTE: CHANGETABLE requires the table name to be a literal or valid identifier, not a string parameter.
      // So we must inject table name directly.

      final result = await query(
        'SELECT COUNT(*) as ChangeCount FROM CHANGETABLE(CHANGES $tableName, $lastVersion) AS CT',
      );

      if (result.rows.isEmpty) {
        return false;
      }

      final count = result.rows.first['ChangeCount'];

      // Handle different return types
      if (count is int) {
        return count > 0;
      } else if (count is String) {
        return (int.tryParse(count) ?? 0) > 0;
      }

      return false;
    } catch (e) {
      // Table doesn't have change tracking or error
      return false;
    }
  }

  /// Get minimum valid change tracking version for a table
  /// Useful to check if your stored version is still valid
  /// Returns null if change tracking is not enabled on the table
  ///
  /// Example:
  /// ```dart
  /// final minVersion = await connection.getMinValidVersion(
  ///   tableName: 'Orders',
  /// );
  /// if (minVersion != null && lastStoredVersion < minVersion) {
  ///   // Stored version is too old, need full refresh
  /// }
  /// ```
  Future<int?> getMinValidVersion({required String tableName}) async {
    _ensureConnected();

    try {
      // Using string injection for table name object_id check
      final result = await query(
        "SELECT CHANGE_TRACKING_MIN_VALID_VERSION(OBJECT_ID('$tableName')) as MinVersion",
      );

      if (result.rows.isEmpty) {
        return null;
      }

      final version = result.rows.first['MinVersion'];

      if (version == null) {
        return null;
      } else if (version is int) {
        return version;
      } else if (version is String) {
        return int.tryParse(version);
      }

      return null;
    } catch (e) {
      return null;
    }
  }

  /// Get detailed change information for a table
  /// Returns a list of changes with operation type and primary key
  ///
  /// Parameters:
  /// - [tableName]: Name of the table to check
  /// - [lastVersion]: Last known change tracking version
  /// - [primaryKeyColumn]: Name of the primary key column (default: 'Id')
  ///
  /// Returns a list of maps containing:
  /// - Primary key value
  /// - Operation type: 'I' (Insert), 'U' (Update), 'D' (Delete)
  /// - Change version
  ///
  /// Example:
  /// ```dart
  /// final changes = await connection.getTableChanges(
  ///   tableName: 'Orders',
  ///   lastVersion: 100,
  ///   primaryKeyColumn: 'OrderNo',
  /// );
  /// for (var change in changes) {
  ///   print('${change['operation']}: ${change['primaryKey']}');
  /// }
  /// ```
  Future<List<Map<String, dynamic>>> getTableChanges({
    required String tableName,
    required int lastVersion,
    String primaryKeyColumn = 'Id',
  }) async {
    _ensureConnected();

    try {
      final result = await query('''
        SELECT 
          CT.$primaryKeyColumn as PrimaryKey,
          CT.SYS_CHANGE_OPERATION as Operation,
          CT.SYS_CHANGE_VERSION as ChangeVersion
        FROM CHANGETABLE(CHANGES $tableName, $lastVersion) AS CT
        ORDER BY CT.SYS_CHANGE_VERSION
        ''');

      return result.rows.map((row) {
        return {
          'primaryKey': row['PrimaryKey'],
          'operation': row['Operation'], // 'I', 'U', or 'D'
          'changeVersion': row['ChangeVersion'],
        };
      }).toList();
    } catch (e) {
      return [];
    }
  }

  // ========== End Change Tracking Methods ==========

  /// Close connection on dispose
  Future<void> dispose() async {
    if (_isConnected) {
      await disconnect();
    }
  }
}
