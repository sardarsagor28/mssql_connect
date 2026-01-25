# mssql_connect

A Flutter plugin for Microsoft SQL Server (MSSQL) connectivity. This plugin allows you to connect to MS SQL Server and execute queries directly from your Flutter application.

## Features

- Connect to Microsoft SQL Server.
- Execute SQL queries (SELECT, INSERT, UPDATE, DELETE).
- Support for multiple platforms (Android, iOS, Windows, macOS, Linux, Web).
- Easy to use API.

## Installation

Add `mssql_connect` to your `pubspec.yaml` file:

```yaml
dependencies:
  mssql_connect: ^0.0.1
```

Then run:

```bash
flutter pub get
```

## Usage

### Direct Connection

```dart
import 'package:mssql_connect/mssql_connect.dart';

void main() async {
  final connection = MssqlConnect();
  
  bool connected = await connection.connect(
    ip: '192.168.1.100',
    port: '1433',
    database: 'your_db',
    username: 'sa',
    password: 'your_password',
  );

  if (connected) {
    var results = await connection.executeQuery("SELECT * FROM Users");
    print(results);
  }
}
```

## Platform Support

| Android | Windows |
| :---: | :---: |
| ✅ | ✅ |

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
