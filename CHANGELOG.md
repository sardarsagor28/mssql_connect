## 0.0.3

* Added SQL Server Change Tracking support.
* New methods in `MsSqlConnection`:
    * `getChangeTrackingVersion()`: Get current database sync version.
    * `hasTableChanges()`: Quickly check if a table has any changes.
    * `getMinValidVersion()`: Verify if last sync version is still valid.
    * `getTableChanges()`: Get detailed list of modified primary keys and operations (Insert, Update, Delete).
* Improved performance for multi-counter POS systems and real-time data synchronization.

## 0.0.2

* Fix: Image bug fix.

## 0.0.1

* Initial release of `mssql_connect`.
* Basic support for MS SQL Server connection and query execution.
* Support for Android and Windows platforms.
