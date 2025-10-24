
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <memory>
#include <mongoc/mongoc.h>

// Security: Validate string length to prevent buffer overflow (CWE-120, CWE-787)
bool isValidStringLength(const std::string& str, size_t maxLen) {
    return str.length() > 0 && str.length() <= maxLen;
}

// Security: Validate input parameters before database operations (CWE-20)
bool validateInput(const std::string& input) {
    if (input.empty() || input.length() > 1000) {
        return false;
    }
    // Security: Check for null bytes that could truncate strings (CWE-170)
    if (input.find('\\0') != std::string::npos) {
        return false;
    }
    return true;
}

class MongoDBClient {
private:
    mongoc_client_t* client;
    mongoc_database_t* database;
    mongoc_collection_t* collection;
    bool initialized;

    // Security: Prevent copying to avoid resource management issues
    MongoDBClient(const MongoDBClient&) = delete;
    MongoDBClient& operator=(const MongoDBClient&) = delete;

public:
    MongoDBClient() : client(nullptr), database(nullptr), collection(nullptr), initialized(false) {}

    // Security: Use environment variable for connection string (CWE-798)
    bool connect(const std::string& dbName, const std::string& collName) {
        if (!validateInput(dbName) || !validateInput(collName)) {
            std::cerr << "Invalid database or collection name\\n";
            return false;
        }

        const char* uri_string = std::getenv("MONGODB_URI");
        if (!uri_string) {
            uri_string = "mongodb://localhost:27017";
        }

        mongoc_init();
        
        // Security: Create client with error handling (CWE-755)
        mongoc_uri_t* uri = mongoc_uri_new_with_error(uri_string, nullptr);
        if (!uri) {
            std::cerr << "Failed to parse URI\\n";
            return false;
        }

        client = mongoc_client_new_from_uri(uri);
        mongoc_uri_destroy(uri);
        
        if (!client) {
            std::cerr << "Failed to create client\\n";
            return false;
        }

        // Security: Validate connection parameters before use
        database = mongoc_client_get_database(client, dbName.c_str());
        collection = mongoc_client_get_collection(client, dbName.c_str(), collName.c_str());
        
        initialized = true;
        return true;
    }

    // Create operation - Security: Validate all inputs (CWE-20)
    bool createDocument(const std::string& name, int age) {
        if (!initialized) {
            std::cerr << "Client not initialized\\n";
            return false;
        }

        if (!validateInput(name)) {
            std::cerr << "Invalid name\\n";
            return false;
        }

        // Security: Check for integer overflow (CWE-190)
        if (age < 0 || age > 150) {
            std::cerr << "Invalid age\\n";
            return false;
        }

        bson_error_t error;
        bson_t* doc = bson_new();
        
        // Security: Use BSON API properly to prevent injection (CWE-943)
        BSON_APPEND_UTF8(doc, "name", name.c_str());
        BSON_APPEND_INT32(doc, "age", age);

        bool success = mongoc_collection_insert_one(collection, doc, nullptr, nullptr, &error);
        
        if (!success) {
            std::cerr << "Insert failed: " << error.message << "\\n";
        }

        bson_destroy(doc);
        return success;
    }

    // Read operation - Security: Limit result size to prevent DoS (CWE-400)
    std::vector<std::string> readDocuments(const std::string& name) {
        std::vector<std::string> results;
        
        if (!initialized) {
            std::cerr << "Client not initialized\\n";
            return results;
        }

        if (!validateInput(name)) {
            std::cerr << "Invalid name\\n";
            return results;
        }

        bson_t* query = bson_new();
        BSON_APPEND_UTF8(query, "name", name.c_str());

        // Security: Set maximum number of results to prevent resource exhaustion
        mongoc_cursor_t* cursor = mongoc_collection_find_with_opts(collection, query, nullptr, nullptr);
        
        const bson_t* doc;
        int count = 0;
        const int MAX_RESULTS = 100;

        while (mongoc_cursor_next(cursor, &doc) && count < MAX_RESULTS) {
            char* str = bson_as_canonical_extended_json(doc, nullptr);
            if (str) {
                results.push_back(std::string(str));
                bson_free(str);
            }
            count++;
        }

        // Security: Check for cursor errors (CWE-755)
        bson_error_t error;
        if (mongoc_cursor_error(cursor, &error)) {
            std::cerr << "Cursor error: " << error.message << "\\n";
        }

        mongoc_cursor_destroy(cursor);
        bson_destroy(query);
        
        return results;
    }

    // Update operation - Security: Validate inputs
    bool updateDocument(const std::string& name, int newAge) {
        if (!initialized) {
            std::cerr << "Client not initialized\\n";
            return false;
        }

        if (!validateInput(name)) {
            std::cerr << "Invalid name\\n";
            return false;
        }

        if (newAge < 0 || newAge > 150) {
            std::cerr << "Invalid age\\n";
            return false;
        }

        bson_t* query = bson_new();
        BSON_APPEND_UTF8(query, "name", name.c_str());

        bson_t* update = bson_new();
        bson_t child;
        BSON_APPEND_DOCUMENT_BEGIN(update, "$set", &child);
        BSON_APPEND_INT32(&child, "age", newAge);
        bson_append_document_end(update, &child);

        bson_error_t error;
        bool success = mongoc_collection_update_one(collection, query, update, nullptr, nullptr, &error);

        if (!success) {
            std::cerr << "Update failed: " << error.message << "\\n";
        }

        bson_destroy(query);
        bson_destroy(update);
        
        return success;
    }

    // Delete operation - Security: Validate input
    bool deleteDocument(const std::string& name) {
        if (!initialized) {
            std::cerr << "Client not initialized\\n";
            return false;
        }

        if (!validateInput(name)) {
            std::cerr << "Invalid name\\n";
            return false;
        }

        bson_t* query = bson_new();
        BSON_APPEND_UTF8(query, "name", name.c_str());

        bson_error_t error;
        bool success = mongoc_collection_delete_one(collection, query, nullptr, nullptr, &error);

        if (!success) {
            std::cerr << "Delete failed: " << error.message << "\\n";
        }

        bson_destroy(query);
        return success;
    }

    // Security: RAII cleanup (CWE-404)
    ~MongoDBClient() {
        if (collection) {
            mongoc_collection_destroy(collection);
        }
        if (database) {
            mongoc_database_destroy(database);
        }
        if (client) {
            mongoc_client_destroy(client);
        }
        if (initialized) {
            mongoc_cleanup();
        }
    }
};

int main() {
    // Security: Use try-catch for exception handling (CWE-755)
    try {
        MongoDBClient client;
        
        if (!client.connect("testdb", "users")) {
            std::cerr << "Failed to connect to MongoDB\\n";
            return 1;
        }

        std::cout << "Test 1: Create document\\n";
        if (client.createDocument("Alice", 30)) {
            std::cout << "Created document for Alice\\n";
        }

        std::cout << "\\nTest 2: Create another document\\n";
        if (client.createDocument("Bob", 25)) {
            std::cout << "Created document for Bob\\n";
        }

        std::cout << "\\nTest 3: Read documents\\n";
        auto results = client.readDocuments("Alice");
        for (const auto& result : results) {
            std::cout << "Found: " << result << "\\n";
        }

        std::cout << "\\nTest 4: Update document\\n";
        if (client.updateDocument("Alice", 31)) {
            std::cout << "Updated Alice's age\\n";
        }
        
        results = client.readDocuments("Alice");
        for (const auto& result : results) {
            std::cout << "After update: " << result << "\\n";
        }

        std::cout << "\\nTest 5: Delete document\\n";
        if (client.deleteDocument("Bob")) {
            std::cout << "Deleted Bob's document\\n";
        }

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\\n";
        return 1;
    }
}
