/// Represents the result of a SQL query
class QueryResult {
  final List<Map<String, dynamic>> rows;
  final int rowCount;
  final List<String> columnNames;

  QueryResult({
    required this.rows,
    required this.rowCount,
    required this.columnNames,
  });

  /// Create QueryResult from JSON
  factory QueryResult.fromJson(Map<dynamic, dynamic> json) {
    final List<dynamic> rowsData = json['rows'] ?? [];
    final List<String> columns = List<String>.from(json['columns'] ?? []);

    final List<Map<String, dynamic>> parsedRows = rowsData.map((row) {
      if (row is Map) {
        return Map<String, dynamic>.from(row);
      }
      return <String, dynamic>{};
    }).toList();

    return QueryResult(
      rows: parsedRows,
      rowCount: json['rowCount'] ?? parsedRows.length,
      columnNames: columns,
    );
  }

  /// Check if result is empty
  bool get isEmpty => rows.isEmpty;

  /// Check if result has data
  bool get isNotEmpty => rows.isNotEmpty;

  /// Convert to JSON
  Map<String, dynamic> toJson() {
    return {'rows': rows, 'rowCount': rowCount, 'columns': columnNames};
  }

  @override
  String toString() {
    return 'QueryResult(rowCount: $rowCount, columns: $columnNames)';
  }
}
