
"use strict";

var assert           = require("assert"),
    DBSession        = require("./DBSession"),
    DBDictionary     = require("./DBDictionary");


/* DBConnectionPool constructor.
   IMMEDIATE.
   Does not perform any IO. 
   Throws an exception if the Properties object is not valid.
*/   
function DBConnectionPool(properties) {
  assert(properties.implementation === "sample");
  this.properties       = properties;
  this.typeConverterMap = {};
}


/* Async connect 
*/
DBConnectionPool.prototype.connect = function(userCallback) {
};


/* DBConnection.isConnected() method.
   IMMEDIATE
   Returns bool true/false
 */
DBConnectionPool.prototype.isConnected = function() {
};


/* close()
   ASYNC
*/
DBConnectionPool.prototype.close = function(userCallback) {
};


/* Creates and opens a new DBSession.
   ASYNC.
   Users's callback receives (error, DBSession)
*/
DBConnectionPool.prototype.getDBSession = function(index, userCallback) {
};


/** List all tables in the schema
  * ASYNC
  * 
  */
DBConnectionPool.prototype.listTables = function(databaseName, dbSession,
                                                 userCallback) {
  return DBDictionary.listTables(databaseName, dbSession, userCallback);
};


/** Fetch metadata for a table
  * ASYNC
  * 
  */
DBConnectionPool.prototype.getTableMetadata = function(databaseName, tableName,
                                                       dbSession, userCallback) {
  return DBDictionary.getTableMetadata(databaseName, tableName, dbSession, userCallback);
};


/* registerTypeConverter(typeName, converterObject) 
   IMMEDIATE
*/
DBConnectionPool.prototype.registerTypeConverter = function(typeName, converter) {
  this.typeConverterMap[typeName] = converter;
};


/** Create a table
  * ASYNC
  * 
  * tableMapping is a TableMapping with possible Meta annotations 
  * indicating column types and indexes.
  */
DBConnectionPool.prototype.createTable = function(tableMapping,
                                                  dbSession,
                                                  userCallback) {
  return DBDictionary.createTable(tableMapping, dbSession, userCallback);
};

module.exports = DBConnectionPool;

