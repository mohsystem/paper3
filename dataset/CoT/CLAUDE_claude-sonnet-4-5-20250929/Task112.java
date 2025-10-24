
import com.mongodb.client.*;
import com.mongodb.client.model.Filters;
import com.mongodb.client.model.Updates;
import org.bson.Document;
import org.bson.types.ObjectId;
import com.mongodb.MongoClientSettings;
import com.mongodb.MongoCredential;
import com.mongodb.ServerAddress;
import com.mongodb.ConnectionString;
import java.util.Arrays;
import java.util.logging.Level;
import java.util.logging.Logger;

public class Task112 {
    private MongoClient mongoClient;
    private MongoDatabase database;
    private MongoCollection<Document> collection;
    
    // Secure connection with authentication
    public Task112(String connectionString, String dbName, String collectionName) {
        try {
            // Disable MongoDB driver logging for cleaner output
            Logger.getLogger("org.mongodb.driver").setLevel(Level.WARNING);
            
            // Secure connection with connection string (supports authentication)
            this.mongoClient = MongoClients.create(connectionString);
            this.database = mongoClient.getDatabase(dbName);
            this.collection = database.getCollection(collectionName);
        } catch (Exception e) {
            System.err.println("Error connecting to MongoDB: " + e.getMessage());
            throw new RuntimeException("Failed to connect to MongoDB", e);
        }
    }
    
    // CREATE operation with input validation
    public String createDocument(String name, int age, String email) {
        try {
            // Input validation
            if (name == null || name.trim().isEmpty()) {
                throw new IllegalArgumentException("Name cannot be null or empty");
            }
            if (age < 0 || age > 150) {
                throw new IllegalArgumentException("Age must be between 0 and 150");
            }
            if (email == null || !email.matches("^[A-Za-z0-9+_.-]+@(.+)$")) {
                throw new IllegalArgumentException("Invalid email format");
            }
            
            Document doc = new Document("name", name.trim())
                    .append("age", age)
                    .append("email", email.trim())
                    .append("createdAt", System.currentTimeMillis());
            
            collection.insertOne(doc);
            return doc.getObjectId("_id").toString();
        } catch (Exception e) {
            System.err.println("Error creating document: " + e.getMessage());
            return null;
        }
    }
    
    // READ operation with safe query
    public Document readDocument(String id) {
        try {
            // Validate ObjectId format
            if (id == null || !ObjectId.isValid(id)) {
                throw new IllegalArgumentException("Invalid ObjectId format");
            }
            
            return collection.find(Filters.eq("_id", new ObjectId(id))).first();
        } catch (Exception e) {
            System.err.println("Error reading document: " + e.getMessage());
            return null;
        }
    }
    
    // READ ALL operation
    public void readAllDocuments() {
        try {
            FindIterable<Document> documents = collection.find();
            for (Document doc : documents) {
                System.out.println(doc.toJson());
            }
        } catch (Exception e) {
            System.err.println("Error reading all documents: " + e.getMessage());
        }
    }
    
    // UPDATE operation with input validation
    public boolean updateDocument(String id, String name, int age, String email) {
        try {
            // Validate ObjectId format
            if (id == null || !ObjectId.isValid(id)) {
                throw new IllegalArgumentException("Invalid ObjectId format");
            }
            
            // Input validation
            if (name != null && name.trim().isEmpty()) {
                throw new IllegalArgumentException("Name cannot be empty");
            }
            if (age < 0 || age > 150) {
                throw new IllegalArgumentException("Age must be between 0 and 150");
            }
            if (email != null && !email.matches("^[A-Za-z0-9+_.-]+@(.+)$")) {
                throw new IllegalArgumentException("Invalid email format");
            }
            
            Document updateDoc = new Document();
            if (name != null) updateDoc.append("name", name.trim());
            if (age > 0) updateDoc.append("age", age);
            if (email != null) updateDoc.append("email", email.trim());
            updateDoc.append("updatedAt", System.currentTimeMillis());
            
            var result = collection.updateOne(
                Filters.eq("_id", new ObjectId(id)),
                new Document("$set", updateDoc)
            );
            
            return result.getModifiedCount() > 0;
        } catch (Exception e) {
            System.err.println("Error updating document: " + e.getMessage());
            return false;
        }
    }
    
    // DELETE operation with validation
    public boolean deleteDocument(String id) {
        try {
            // Validate ObjectId format
            if (id == null || !ObjectId.isValid(id)) {
                throw new IllegalArgumentException("Invalid ObjectId format");
            }
            
            var result = collection.deleteOne(Filters.eq("_id", new ObjectId(id)));
            return result.getDeletedCount() > 0;
        } catch (Exception e) {
            System.err.println("Error deleting document: " + e.getMessage());
            return false;
        }
    }
    
    // Close connection securely
    public void close() {
        if (mongoClient != null) {
            mongoClient.close();
        }
    }
    
    public static void main(String[] args) {
        // Use secure connection string with authentication
        String connectionString = "mongodb://localhost:27017";
        String dbName = "testdb";
        String collectionName = "users";
        
        Task112 mongoOps = new Task112(connectionString, dbName, collectionName);
        
        try {
            System.out.println("=== Test Case 1: Create Documents ===");
            String id1 = mongoOps.createDocument("John Doe", 30, "john@example.com");
            System.out.println("Created document with ID: " + id1);
            
            String id2 = mongoOps.createDocument("Jane Smith", 25, "jane@example.com");
            System.out.println("Created document with ID: " + id2);
            
            System.out.println("\\n=== Test Case 2: Read Document ===");
            Document doc = mongoOps.readDocument(id1);
            if (doc != null) {
                System.out.println("Read document: " + doc.toJson());
            }
            
            System.out.println("\\n=== Test Case 3: Read All Documents ===");
            mongoOps.readAllDocuments();
            
            System.out.println("\\n=== Test Case 4: Update Document ===");
            boolean updated = mongoOps.updateDocument(id1, "John Updated", 31, "johnupdated@example.com");
            System.out.println("Update successful: " + updated);
            
            System.out.println("\\n=== Test Case 5: Delete Document ===");
            boolean deleted = mongoOps.deleteDocument(id2);
            System.out.println("Delete successful: " + deleted);
            
            System.out.println("\\n=== Final State ===");
            mongoOps.readAllDocuments();
            
        } catch (Exception e) {
            System.err.println("Error in main: " + e.getMessage());
        } finally {
            mongoOps.close();
        }
    }
}
