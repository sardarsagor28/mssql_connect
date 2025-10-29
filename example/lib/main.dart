import 'dart:developer';
import 'package:flutter/material.dart';
import 'dart:async';
import 'package:mssql_connect/mssql_connect.dart'; //

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'MSSQL Connection Demo',
      theme: ThemeData(primarySwatch: Colors.blue, useMaterial3: true),
      home: const HomePage(),
    );
  }
}

class HomePage extends StatefulWidget {
  const HomePage({Key? key}) : super(key: key);

  @override
  State<HomePage> createState() => _HomePageState();
}

class _HomePageState extends State<HomePage> {
  final _serverController = TextEditingController(
    text: '192.168.1.18\\SQL2014',
  );
  final _databaseController = TextEditingController(text: 'tPOS_BOSTON_BITES');
  final _usernameController = TextEditingController(text: 'sa');
  final _passwordController = TextEditingController(text: 'data');
  final _queryController = TextEditingController(
    text: 'SELECT * FROM TableInfo',
  );

  MsSqlConnection? _connection;
  String _statusMessage = 'Not connected';
  bool _isConnected = false;
  bool _isLoading = false;
  List<Map<String, dynamic>> _queryResults = [];
  List<String> _columnNames = [];

  String _platformVersion = 'Unknown';

  @override
  void initState() {
    super.initState();
    _getPlatformVersion();
  }

  Future<void> _getPlatformVersion() async {
    String platformVersion;
    try {
      platformVersion =
          await MssqlConnect().getPlatformVersion() ??
          'Unknown platform version';
    } catch (e) {
      platformVersion = 'Failed to get platform version.';
    }

    if (!mounted) return;

    setState(() {
      _platformVersion = platformVersion;
    });
  }

  @override
  void dispose() {
    _serverController.dispose();
    _databaseController.dispose();
    _usernameController.dispose();
    _passwordController.dispose();
    _queryController.dispose();
    _connection?.dispose();
    super.dispose();
  }

  Future<void> _testConnection() async {
    setState(() {
      _isLoading = true;
      _statusMessage = 'Testing connection...';
    });

    try {
      final connection = MsSqlConnection(
        server: _serverController.text,
        database: _databaseController.text,
        username: _usernameController.text,
        password: _passwordController.text,
        trustedConnection: false,
      );

      final success = await connection.testConnection();

      setState(() {
        _statusMessage = success
            ? '✓ Connection test successful!'
            : '✗ Connection test failed';
        _isLoading = false;
      });

      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(
            content: Text(_statusMessage),
            backgroundColor: success ? Colors.green : Colors.red,
          ),
        );
      }
    } catch (e) {
      final errorMessage = e is DatabaseException
          ? 'DB Error: ${e.message}${e.details != null ? '\nDetails: ${e.details}' : ''}'
          : e.toString();
      log('✗ Error: $errorMessage');
      setState(() {
        _statusMessage = '✗ Error: $errorMessage';
        _isLoading = false;
      });

      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(
            content: Text('Connection test failed: $errorMessage'),
            backgroundColor: Colors.red,
          ),
        );
      }
    }
  }

  Future<void> _connect() async {
    if (_isConnected) {
      await _disconnect();
      return;
    }

    setState(() {
      _isLoading = true;
      _statusMessage = 'Connecting...';
    });

    try {
      _connection = MsSqlConnection(
        server: _serverController.text,
        database: _databaseController.text,
        username: _usernameController.text,
        password: _passwordController.text,
        trustedConnection: false,
      );

      final success = await _connection!.connect();

      setState(() {
        _isConnected = success;
        _statusMessage = success
            ? '✓ Connected to ${_databaseController.text}'
            : '✗ Connection failed';
        _isLoading = false;
      });

      if (mounted && success) {
        ScaffoldMessenger.of(context).showSnackBar(
          const SnackBar(
            content: Text('Connected successfully!'),
            backgroundColor: Colors.green,
          ),
        );
      }
    } catch (e) {
      setState(() {
        _isConnected = false;
        _statusMessage = e is DatabaseException
            ? '✗ Error: ${e.message}${e.details != null ? '\nDetails: ${e.details}' : ''}'
            : '✗ Error: ${e.toString()}';
        _isLoading = false;
      });

      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(
            content: Text('Connection failed: $e'),
            backgroundColor: Colors.red,
          ),
        );
      }
    }
  }

  Future<void> _disconnect() async {
    if (_connection == null) return;

    setState(() {
      _isLoading = true;
      _statusMessage = 'Disconnecting...';
    });

    try {
      await _connection!.disconnect();

      setState(() {
        _isConnected = false;
        _statusMessage = 'Disconnected';
        _isLoading = false;
        _queryResults = [];
        _columnNames = [];
      });

      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          const SnackBar(
            content: Text('Disconnected successfully'),
            backgroundColor: Colors.orange,
          ),
        );
      }
    } catch (e) {
      setState(() {
        _statusMessage = 'Disconnect error: ${e.toString()}';
        _isLoading = false;
      });
    }
  }

  Future<void> _executeQuery() async {
    if (!_isConnected || _connection == null) {
      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(
          content: Text('Please connect to database first'),
          backgroundColor: Colors.red,
        ),
      );
      return;
    }

    setState(() {
      _isLoading = true;
      _statusMessage = 'Executing query...';
    });

    try {
      final result = await _connection!.query(_queryController.text);

      setState(() {
        _queryResults = result.rows;
        _columnNames = result.columnNames;
        _statusMessage = '✓ Query executed: ${result.rowCount} rows returned';
        _isLoading = false;
      });

      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(
            content: Text('Query executed: ${result.rowCount} rows'),
            backgroundColor: Colors.green,
          ),
        );
      }
    } catch (e) {
      setState(() {
        _statusMessage = e is DatabaseException
            ? '✗ Query Error: ${e.message}${e.details != null ? '\nDetails: ${e.details}' : ''}'
            : '✗ Query Error: ${e.toString()}';
        _isLoading = false;
        _queryResults = [];
        _columnNames = [];
      });

      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(
            content: Text('Query failed: $e'),
            backgroundColor: Colors.red,
          ),
        );
      }

      log('✗ Error: $e');
    }
  }

  Future<void> _executeCommand() async {
    if (!_isConnected || _connection == null) {
      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(
          content: Text('Please connect to database first'),
          backgroundColor: Colors.red,
        ),
      );
      return;
    }

    setState(() {
      _isLoading = true;
      _statusMessage = 'Executing command...';
    });

    try {
      final affectedRows = await _connection!.execute(_queryController.text);

      setState(() {
        _statusMessage = '✓ Command executed: $affectedRows rows affected';
        _isLoading = false;
      });

      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(
            content: Text('Command executed: $affectedRows rows affected'),
            backgroundColor: Colors.green,
          ),
        );
      }
    } catch (e) {
      setState(() {
        _statusMessage = e is DatabaseException
            ? '✗ Command Error: ${e.message}${e.details != null ? '\nDetails: ${e.details}' : ''}'
            : '✗ Command Error: ${e.toString()}';
        _isLoading = false;
      });

      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(
            content: Text('Command failed: $e'),
            backgroundColor: Colors.red,
          ),
        );
      }

      log('✗ Error: $e');
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('MSSQL Connection Demo'),
        backgroundColor: Theme.of(context).colorScheme.inversePrimary,
      ),
      body: SingleChildScrollView(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            // Connection Settings Card
            Card(
              child: Padding(
                padding: const EdgeInsets.all(16.0),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    const Text(
                      'Connection Settings',
                      style: TextStyle(
                        fontSize: 18,
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                    const SizedBox(height: 16),
                    TextField(
                      controller: _serverController,
                      decoration: const InputDecoration(
                        labelText: 'Server',
                        border: OutlineInputBorder(),
                        prefixIcon: Icon(Icons.dns),
                      ),
                      enabled: !_isConnected,
                    ),
                    const SizedBox(height: 12),
                    TextField(
                      controller: _databaseController,
                      decoration: const InputDecoration(
                        labelText: 'Database',
                        border: OutlineInputBorder(),
                        prefixIcon: Icon(Icons.storage),
                      ),
                      enabled: !_isConnected,
                    ),
                    const SizedBox(height: 12),
                    TextField(
                      controller: _usernameController,
                      decoration: const InputDecoration(
                        labelText: 'Username',
                        border: OutlineInputBorder(),
                        prefixIcon: Icon(Icons.person),
                      ),
                      enabled: !_isConnected,
                    ),
                    const SizedBox(height: 12),
                    TextField(
                      controller: _passwordController,
                      decoration: const InputDecoration(
                        labelText: 'Password',
                        border: OutlineInputBorder(),
                        prefixIcon: Icon(Icons.lock),
                      ),
                      obscureText: true,
                      enabled: !_isConnected,
                    ),
                    const SizedBox(height: 16),
                    Row(
                      children: [
                        Expanded(
                          child: ElevatedButton.icon(
                            onPressed: _isLoading ? null : _testConnection,
                            icon: const Icon(Icons.wifi_tethering),
                            label: const Text('Test Connection'),
                            style: ElevatedButton.styleFrom(
                              padding: const EdgeInsets.all(16),
                            ),
                          ),
                        ),
                        const SizedBox(width: 12),
                        Expanded(
                          child: ElevatedButton.icon(
                            onPressed: _isLoading ? null : _connect,
                            icon: Icon(_isConnected ? Icons.close : Icons.link),
                            label: Text(
                              _isConnected ? 'Disconnect' : 'Connect',
                            ),
                            style: ElevatedButton.styleFrom(
                              padding: const EdgeInsets.all(16),
                              backgroundColor: _isConnected
                                  ? Colors.red
                                  : Colors.blue,
                              foregroundColor: Colors.white,
                            ),
                          ),
                        ),
                      ],
                    ),
                  ],
                ),
              ),
            ),
            const SizedBox(height: 16),

            // Status Card
            Card(
              color: _isConnected ? Colors.green[50] : Colors.grey[100],
              child: Padding(
                padding: const EdgeInsets.all(16.0),
                child: Column(
                  children: [
                    Row(
                      children: [
                        Icon(
                          _isConnected
                              ? Icons.check_circle
                              : Icons.info_outline,
                          color: _isConnected ? Colors.green : Colors.grey,
                        ),
                        const SizedBox(width: 12),
                        Expanded(
                          child: Text(
                            _statusMessage,
                            style: TextStyle(
                              color: _isConnected
                                  ? Colors.green[900]
                                  : Colors.black87,
                            ),
                          ),
                        ),
                        if (_isLoading)
                          const SizedBox(
                            width: 20,
                            height: 20,
                            child: CircularProgressIndicator(strokeWidth: 2),
                          ),
                      ],
                    ),
                    const SizedBox(height: 10),
                    Text('Platform Version: $_platformVersion'),
                  ],
                ),
              ),
            ),
            const SizedBox(height: 16),

            // Query Card
            Card(
              child: Padding(
                padding: const EdgeInsets.all(16.0),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    const Text(
                      'SQL Query',
                      style: TextStyle(
                        fontSize: 18,
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                    const SizedBox(height: 16),
                    TextField(
                      controller: _queryController,
                      decoration: const InputDecoration(
                        labelText:
                            'SQL Query (SELECT) or Command (INSERT/UPDATE/DELETE)',
                        border: OutlineInputBorder(),
                        prefixIcon: Icon(Icons.code),
                      ),
                      maxLines: 4,
                      enabled: _isConnected,
                    ),
                    const SizedBox(height: 16),
                    Row(
                      children: [
                        Expanded(
                          child: ElevatedButton.icon(
                            onPressed: _isConnected && !_isLoading
                                ? _executeQuery
                                : null,
                            icon: const Icon(Icons.play_arrow),
                            label: const Text('Execute Query'),
                            style: ElevatedButton.styleFrom(
                              padding: const EdgeInsets.all(16),
                            ),
                          ),
                        ),
                        const SizedBox(width: 12),
                        Expanded(
                          child: ElevatedButton.icon(
                            onPressed: _isConnected && !_isLoading
                                ? _executeCommand
                                : null,
                            icon: const Icon(Icons.edit),
                            label: const Text('Execute Command'),
                            style: ElevatedButton.styleFrom(
                              padding: const EdgeInsets.all(16),
                            ),
                          ),
                        ),
                      ],
                    ),
                  ],
                ),
              ),
            ),
            const SizedBox(height: 16),

            // Results Card
            if (_queryResults.isNotEmpty)
              Card(
                child: Padding(
                  padding: const EdgeInsets.all(16.0),
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      Text(
                        'Results (${_queryResults.length} rows)',
                        style: const TextStyle(
                          fontSize: 18,
                          fontWeight: FontWeight.bold,
                        ),
                      ),
                      const SizedBox(height: 16),
                      SingleChildScrollView(
                        scrollDirection: Axis.horizontal,
                        child: DataTable(
                          columns: _columnNames
                              .map(
                                (col) => DataColumn(
                                  label: Text(
                                    col,
                                    style: const TextStyle(
                                      fontWeight: FontWeight.bold,
                                    ),
                                  ),
                                ),
                              )
                              .toList(),
                          rows: _queryResults.map((row) {
                            return DataRow(
                              cells: _columnNames.map((col) {
                                return DataCell(
                                  Text(row[col]?.toString() ?? 'NULL'),
                                );
                              }).toList(),
                            );
                          }).toList(),
                        ),
                      ),
                    ],
                  ),
                ),
              ),
          ],
        ),
      ),
    );
  }
}
