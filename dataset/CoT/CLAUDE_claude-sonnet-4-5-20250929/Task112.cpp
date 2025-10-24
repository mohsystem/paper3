
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <iostream>
#include <string>
#include <regex>
#include <memory>
#include <ctime>

using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;

class Task112 {
private:
    mongocxx::client client;
    mongocxx::database db;
    mongocxx::collection collection;
    
    bool validateEmail(const std::string& email) {
        const std::regex pattern("^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+$");
        return std::regex_match(email, pattern);
    }
    
    std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(' ');
        if (std::string::npos == first) {
            return str;
        }
        size_t last = str.find_last_not_of(' ');
        return str.substr(first, (last - first + 1));
    }
    
public:
    Task112(const std::string& connection_string, const std::string& db_name, 
            const std::string& collection_name) {
        try {
            mongocxx::uri uri(connection_string);
            client = mongocxx::client(uri);
            db = client[db_name];
            collection = db[collection_name];
        } catch (const std::exception& e) {
            std::cerr << "Error connecting to MongoDB: " << e.what() << std::endl;
            throw;
        }
    }
    
    std::string createDocument(const std::string& name, int age, const std::string& email) {
        try {
            // Input validation
            std::string trimmed_name = trim(name);
            if (trimmed_name.empty()) {
                throw std::invalid_argument("Name cannot be null or empty");
            }
            if (age < 0 || age > 150) {
                throw std::invalid_argument("Age must be between 0 and 150");
            }
            std::string trimmed_email = trim(email);
            if (!validateEmail(trimmed_email)) {
                throw std::invalid_argument("Invalid email format");
            }
            
            auto doc = document{}
                << "name" << trimmed_name
                << "age" << age
                << "email" << trimmed_email
                << "createdAt" << static_cast<long>(std::time(nullptr))
                << finalize;
            
            auto result = collection.insert_one(doc.view());
            if (result) {
                return result->inserted_id().get_oid().value.to_string();
            }
            return "";
        } catch (const std::exception& e) {
            std::cerr << "Error creating document: " << e.what() << std::endl;
            return "";
        }
    }
    
    std::string readDocument(const std::string& id) {
        try {
            // Validate ObjectId format
            if (id.length() != 24) {
                throw std::invalid_argument("Invalid ObjectId format");
            }
            
            auto filter = document{} << "_id" << bsoncxx::oid(id) << finalize;
            auto result = collection.find_one(filter.view());
            
            if (result) {
                return bsoncxx::to_json(result->view());
            }
            return "";
        } catch (const std::exception& e) {
            std::cerr << "Error reading document: " << e.what() << std::endl;
            return "";
        }
    }
    
    void readAllDocuments() {
        try {
            auto cursor = collection.find({});
            for (auto&& doc : cursor) {
                std::cout << bsoncxx::to_json(doc) << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error reading all documents: " << e.what() << std::endl;
        }
    }
    
    bool updateDocument(const std::string& id, const std::string& name, 
                       int age, const std::string& email) {
        try {
            // Validate ObjectId format
            if (id.length() != 24) {
                throw std::invalid_argument("Invalid ObjectId format");
            }
            
            // Input validation
            std::string trimmed_name = trim(name);
            if (!name.empty() && trimmed_name.empty()) {
                throw std::invalid_argument("Name cannot be empty");
            }
            if (age < 0 || age > 150) {
                throw std::invalid_argument("Age must be between 0 and 150");
            }
            std::string trimmed_email = trim(email);
            if (!email.empty() && !validateEmail(trimmed_email)) {
                throw std::invalid_argument("Invalid email format");
            }
            
            auto filter = document{} << "_id" << bsoncxx::oid(id) << finalize;
            
            auto update_doc = document{};
            auto set_doc = update_doc << "$set" << bsoncxx::builder::stream::open_document;
            
            if (!trimmed_name.empty()) {
                set_doc << "name" << trimmed_name;
            }
            if (age > 0) {
                set_doc << "age" << age;
            }
            if (!trimmed_email.empty()) {
                set_doc << "email" << trimmed_email;
            }
            set_doc << "updatedAt" << static_cast<long>(std::time(nullptr));
            set_doc << bsoncxx::builder::stream::close_document << finalize;
            
            auto result = collection.update_one(filter.view(), update_doc.view());
            
            return result && result->modified_count() > 0;
        } catch (const std::exception& e) {
            std::cerr << "Error updating document: " << e.what() << std::endl;
            return false;
        }
    }
    
    bool deleteDocument(const std::string& id) {
        try {
            // Validate ObjectId format
            if (id.length() != 24) {
                throw std::invalid_argument("Invalid ObjectId format");
            }
            
            auto filter = document{} << "_id" << bsoncxx::oid(id) << finalize;
            auto result = collection.delete_one(filter.view());
            
            return result && result->deleted_count() > 0;
        } catch (const std::exception& e) {
            std::cerr << "Error deleting document: " << e.what() << std::endl;
            return false;
        }
    }
};

int main() {
    mongocxx::instance instance{};
    
    std::string connection_string = "mongodb://localhost:27017";
    std::string db_name = "testdb";
    std::string collection_name = "users";
    
    try {
        Task112 mongoOps(connection_string, db_name, collection_name);
        
        std::cout << "=== Test Case 1: Create Documents ===" << std::endl;
        std::string id1 = mongoOps.createDocument("John Doe", 30, "john@example.com");
        std::cout << "Created document with ID: " << id1 << std::endl;
        
        std::string id2 = mongoOps.createDocument("Jane Smith", 25, "jane@example.com");
        std::cout << "Created document with ID: " << id2 << std::endl;
        
        std::cout << "\\n=== Test Case 2: Read Document ===" << std::endl;
        std::string doc = mongoOps.readDocument(id1);
        if (!doc.empty()) {
            std::cout << "Read document: " << doc << std::endl;
        }
        
        std::cout << "\\n=== Test Case 3: Read All Documents ===" << std::endl;
        mongoOps.readAllDocuments();
        
        std::cout << "\\n=== Test Case 4: Update Document ===" << std::endl;
        bool updated = mongoOps.updateDocument(id1, "John Updated", 31, "johnupdated@example.com");
        std::cout << "Update successful: " << (updated ? "true" : "false") << std::endl;
        
        std::cout << "\\n=== Test Case 5: Delete Document ===" << std::endl;
        bool deleted = mongoOps.deleteDocument(id2);
        std::cout << "Delete successful: " << (deleted ? "true" : "false") << std::endl;
        
        std::cout << "\\n=== Final State ===" << std::endl;
        mongoOps.readAllDocuments();
        
    } catch (const std::exception& e) {
        std::cerr << "Error in main: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
