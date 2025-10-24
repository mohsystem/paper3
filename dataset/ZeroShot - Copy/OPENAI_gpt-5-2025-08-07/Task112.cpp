// Implementation requires MongoDB C++ (mongocxx) driver and setup which is environment-specific.
// For a secure CRUD implementation, use mongocxx::client constructed from an environment-provided URI,
// validate field names (no keys starting with '$' or containing '.'), and use BSON builders for filters/updates.
// Include 5 test cases similar to the Java/Python examples once the driver is available and linked.