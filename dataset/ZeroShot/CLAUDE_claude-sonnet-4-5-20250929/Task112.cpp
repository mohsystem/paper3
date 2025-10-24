
#include <iostream>
#include <string>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/stdx.hpp>

using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::stream::close_document;

class Task112 {
private:
    mongocxx::instance instance;
    mongocxx::client client;
    mongocxx::database database;
    mongocxx::collection collection;
    
public:
    Task112() : client(mongocxx::uri("mongodb://localhost:27017")) {
        database = client["testdb"];
        collection = database["users"];
    }
    
    // Create operation
    std::string createDocument(const std::string& name, const std::string& email, int age) {
        try {
            auto builder = document{};
            builder << "name" << name
                   << "email" << email
                   << "age" << age;
            
            auto result = collection.insert_one(builder.view());
            if (result) {
                return result->inserted_id().get_oid().value.to_string();
            }
        } catch (const std::exception& e) {
            std::cerr << "Error creating document: " << e.what() << std::endl;
        }
        return "";
    }
    
    // Read operation
    std::string readDocument(const std::string& id) {
        try {
            auto oid = bsoncxx::oid(id);
            auto filter = document{} << "_id" << oid << finalize;
            auto result = collection.find_one(filter.view());
            
            if (result) {
                return bsoncxx::to_json(*result);
            }
        } catch (const std::exception& e) {
            std::cerr << "Error reading document: " << e.what() << std::endl;
        }
        return "";
    }
    
    // Update operation
    bool updateDocument(const std::string& id, const std::string& name, 
                       const std::string& email, int age) {
        try {
            auto oid = bsoncxx::oid(id);
            auto filter = document{} << "_id" << oid << finalize;
            
            auto update_builder = document{};
            auto set_builder = update_builder << "$set" << open_document;
            
            if (!name.empty()) {
                set_builder << "name" << name;
            }
            if (!email.empty()) {
                set_builder << "email" << email;
            }
            if (age > 0) {
                set_builder << "age" << age;
            }
            
            set_builder << close_document;
            
            collection.update_one(filter.view(), update_builder.view());
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error updating document: " << e.what() << std::endl;
        }
        return false;
    }
    
    // Delete operation
    bool deleteDocument(const std::string& id) {
        try {
            auto oid = bsoncxx::oid(id);
            auto filter = document{} << "_id" << oid << finalize;
            collection.delete_one(filter.view());
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error deleting document: " << e.what() << std::endl;
        }
        return false;
    }
};

int main() {
    Task112 task;
    
    std::cout << "=== Test Case 1: Create Document ===" << std::endl;
    std::string id1 = task.createDocument("John Doe", "john@example.com", 30);
    std::cout << "Created document with ID: " << id1 << std::endl;
    
    std::cout << "\\n=== Test Case 2: Read Document ===" << std::endl;
    if (!id1.empty()) {
        std::string doc = task.readDocument(id1);
        std::cout << "Retrieved document: " << doc << std::endl;
    }
    
    std::cout << "\\n=== Test Case 3: Create Multiple Documents ===" << std::endl;
    std::string id2 = task.createDocument("Jane Smith", "jane@example.com", 25);
    std::string id3 = task.createDocument("Bob Johnson", "bob@example.com", 35);
    std::cout << "Created documents with IDs: " << id2 << ", " << id3 << std::endl;
    
    std::cout << "\\n=== Test Case 4: Update Document ===" << std::endl;
    if (!id1.empty()) {
        bool updated = task.updateDocument(id1, "John Updated", "", 31);
        std::cout << "Update successful: " << updated << std::endl;
        std::string updatedDoc = task.readDocument(id1);
        std::cout << "Updated document: " << updatedDoc << std::endl;
    }
    
    std::cout << "\\n=== Test Case 5: Delete Document ===" << std::endl;
    if (!id1.empty()) {
        bool deleted = task.deleteDocument(id1);
        std::cout << "Delete successful: " << deleted << std::endl;
        std::string deletedDoc = task.readDocument(id1);
        std::cout << "Document after deletion: " << deletedDoc << std::endl;
    }
    
    return 0;
}
