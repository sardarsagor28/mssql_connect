/// Custom exception for database operations
class DatabaseException implements Exception {
  final String message;
  final String? details;

  DatabaseException(this.message, {this.details});

  @override
  String toString() {
    if (details != null) {
      return 'DatabaseException: $message\nDetails: $details';
    }
    return 'DatabaseException: $message';
  }
}

/// Exception for connection errors
class ConnectionException extends DatabaseException {
  ConnectionException(String message, {String? details})
    : super(message, details: details);
}

/// Exception for query errors
class QueryException extends DatabaseException {
  QueryException(String message, {String? details})
    : super(message, details: details);
}
