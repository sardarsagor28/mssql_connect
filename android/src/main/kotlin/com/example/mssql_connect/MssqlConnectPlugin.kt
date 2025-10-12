package com.example.mssql_connect

import io.flutter.embedding.engine.plugins.FlutterPlugin
import io.flutter.plugin.common.MethodCall
import io.flutter.plugin.common.MethodChannel
import io.flutter.plugin.common.MethodChannel.MethodCallHandler
import io.flutter.plugin.common.MethodChannel.Result
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import java.sql.Connection
import java.sql.DriverManager
import java.sql.SQLException

/** MssqlConnectPlugin */
class MssqlConnectPlugin : FlutterPlugin, MethodCallHandler {
    private lateinit var channel: MethodChannel
    private val scope = CoroutineScope(Dispatchers.IO)
    private val connections = mutableMapOf<Int, Connection>()
    private var nextConnectionId = 0

    override fun onAttachedToEngine(flutterPluginBinding: FlutterPlugin.FlutterPluginBinding) {
        channel = MethodChannel(flutterPluginBinding.binaryMessenger, "mssql_connect")
        channel.setMethodCallHandler(this)
    }

    override fun onMethodCall(call: MethodCall, result: Result) {
        when (call.method) {
            "getPlatformVersion" -> {
                result.success("Android ${android.os.Build.VERSION.RELEASE}")
            }
            "connect" -> {
                scope.launch {
                    try {
                        val args = call.arguments as Map<String, Any>
                        val server = args["server"] as String
                        val database = args["database"] as String
                        val username = args["username"] as String
                        val password = args["password"] as String
                        val connectionUrl = "jdbc:sqlserver://$server;databaseName=$database;user=$username;password=$password;encrypt=true;trustServerCertificate=true;"
                        Class.forName("com.microsoft.sqlserver.jdbc.SQLServerDriver")
                        val connection = DriverManager.getConnection(connectionUrl)
                        val connectionId = nextConnectionId++
                        connections[connectionId] = connection
                        val response = mapOf("connectionId" to connectionId, "success" to true)
                        result.success(response)
                    } catch (e: Exception) {
                        result.error("ConnectionError", "Failed to connect to database", e.message)
                    }
                }
            }
            "disconnect" -> {
                scope.launch {
                    try {
                        val args = call.arguments as Map<String, Any>
                        val connectionId = args["connectionId"] as Int
                        val connection = connections[connectionId]
                        if (connection != null) {
                            connection.close()
                            connections.remove(connectionId)
                            result.success(true)
                        } else {
                            result.error("InvalidConnection", "Invalid connection ID", null)
                        }
                    } catch (e: Exception) {
                        result.error("DisconnectError", "Failed to disconnect", e.message)
                    }
                }
            }
            "query" -> {
                scope.launch {
                    try {
                        val args = call.arguments as Map<String, Any>
                        val connectionId = args["connectionId"] as Int
                        val sql = args["sql"] as String
                        val connection = connections[connectionId]
                        if (connection != null) {
                            val statement = connection.createStatement()
                            val resultSet = statement.executeQuery(sql)
                            val metaData = resultSet.metaData
                            val columnCount = metaData.columnCount
                            val columnNames = (1..columnCount).map { metaData.getColumnName(it) }
                            val rows = mutableListOf<Map<String, Any?>>()
                            while (resultSet.next()) {
                                val row = mutableMapOf<String, Any?>()
                                for (i in 1..columnCount) {
                                    row[columnNames[i - 1]] = resultSet.getObject(i)
                                }
                                rows.add(row)
                            }
                            val response = mapOf("rows" to rows, "rowCount" to rows.size, "columns" to columnNames)
                            result.success(response)
                        } else {
                            result.error("InvalidConnection", "Invalid connection ID", null)
                        }
                    } catch (e: Exception) {
                        result.error("QueryError", "Query execution failed", e.message)
                    }
                }
            }
            "execute" -> {
                scope.launch {
                    try {
                        val args = call.arguments as Map<String, Any>
                        val connectionId = args["connectionId"] as Int
                        val sql = args["sql"] as String
                        val connection = connections[connectionId]
                        if (connection != null) {
                            val statement = connection.createStatement()
                            val affectedRows = statement.executeUpdate(sql)
                            result.success(affectedRows)
                        } else {
                            result.error("InvalidConnection", "Invalid connection ID", null)
                        }
                    } catch (e: Exception) {
                        result.error("ExecuteError", "Command execution failed", e.message)
                    }
                }
            }
            "testConnection" -> {
                scope.launch {
                    try {
                        val args = call.arguments as Map<String, Any>
                        val server = args["server"] as String
                        val database = args["database"] as String
                        val username = args["username"] as String
                        val password = args["password"] as String
                        val connectionUrl = "jdbc:sqlserver://$server;databaseName=$database;user=$username;password=$password;encrypt=true;trustServerCertificate=true;"
                        Class.forName("com.microsoft.sqlserver.jdbc.SQLServerDriver")
                        val connection = DriverManager.getConnection(connectionUrl)
                        connection.close()
                        result.success(true)
                    } catch (e: Exception) {
                        result.error("ConnectionError", "Connection test failed", e.message)
                    }
                }
            }
            else -> {
                result.notImplemented()
            }
        }
    }

    override fun onDetachedFromEngine(binding: FlutterPlugin.FlutterPluginBinding) {
        channel.setMethodCallHandler(null)
        connections.values.forEach { it.close() }
        connections.clear()
    }
}