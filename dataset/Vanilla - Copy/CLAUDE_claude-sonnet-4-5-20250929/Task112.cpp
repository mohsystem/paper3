
#include <iostream>
#include <string>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>

using namespace std;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;

class Task112 {
private:
    mongocxx::client client;
    mongocxx::database database;
    mongocxx::collection collection;

public:
    Task112(const string& connection_string, const string& db_name, const string& collection_name) {
        mongocxx::uri uri(connection_string);
        client = mongocxx::client(uri);
        database = client[db_name];
        collection = database[collection_name];
    }

    // Create operation
    string create(const string& name, int age, const string& email) {
        auto doc = document{} 
            << "name" << name 
            << "age" << age 
            << "email" << email 
            << finalize;
        
        auto result = collection.insert_one(doc.view());
        if (result) {
            return "Created document with ID: " + result->inserted_id().get_oid().value.to_string();
        }
        return "Failed to create document";
    }

    // Read operation
    string read(const string& name) {
        auto filter = document{} << "name" << name << finalize;
        auto result = collection.find_one(filter.view());
        
        if (result) {
            return "Found: " + bsoncxx::to_json(*result);
        }
        return "Document not found";
    }

    // Update operation
    string update(const string& name, int new_age) {
        auto filter = document{} << "name" << name << finalize;
        auto update_doc = document{} << "$set" << bsoncxx::builder::stream::open_document
            << "age" << new_age
            << bsoncxx::builder::stream::close_document << finalize;
        
        auto result = collection.update_one(filter.view(), update_doc.view());
        if (result) {
            return "Modified count: " + to_string(result->modified_count());
        }
        return "Update failed";
    }

    // Delete operation
    string deleteDoc(const string& name) {
        auto filter = document{} << "name" << name << finalize;
        auto result = collection.delete_one(filter.view());
        
        if (result) {
            return "Deleted count: " + to_string(result->deleted_count());
        }
        return "Delete failed";
    }
};

int main() {
    // Initialize the MongoDB driver (must be done once)
    mongocxx::instance instance{};
    
    // Note: Replace with your MongoDB connection string
    string connection_string = "mongodb://localhost:27017";
    string db_name = "testdb";
    string collection_name = "users";

    Task112 mongoOps(connection_string, db_name, collection_name);

    cout << "Test Case 1: Create a document" << endl;
    cout << mongoOps.create("John Doe", 30, "john@example.com") << endl;

    cout << "\\nTest Case 2: Read a document" << endl;
    cout << mongoOps.read("John Doe") << endl;

    cout << "\\nTest Case 3: Update a document" << endl;
    cout << mongoOps.update("John Doe", 31) << endl;

    cout << "\\nTest Case 4: Read updated document" << endl;
    cout << mongoOps.read("John Doe") << endl;

    cout << "\\nTest Case 5: Delete a document" << endl;
    cout << mongoOps.deleteDoc("John Doe") << endl;

    return 0;
}
