
import com.mongodb.client.*;
import com.mongodb.client.model.Filters;
import com.mongodb.client.model.Updates;
import org.bson.Document;
import org.bson.types.ObjectId;

public class Task112 {
    private static final String CONNECTION_STRING = "mongodb://localhost:27017";
    private static final String DATABASE_NAME = "testdb";
    private static final String COLLECTION_NAME = "users";
    
    private MongoClient mongoClient;
    private MongoDatabase database;
    private MongoCollection<Document> collection;
    
    public Task112() {
        try {
            mongoClient = MongoClients.create(CONNECTION_STRING);
            database = mongoClient.getDatabase(DATABASE_NAME);
            collection = database.getCollection(COLLECTION_NAME);
        } catch (Exception e) {
            System.err.println("Error connecting to MongoDB: " + e.getMessage());
        }
    }
    
    // Create operation
    public String createDocument(String name, String email, int age) {
        try {
            Document doc = new Document("name", name)
                    .append("email", email)
                    .append("age", age);
            collection.insertOne(doc);
            return doc.getObjectId("_id").toString();
        } catch (Exception e) {
            System.err.println("Error creating document: " + e.getMessage());
            return null;
        }
    }
    
    // Read operation
    public Document readDocument(String id) {
        try {
            ObjectId objectId = new ObjectId(id);
            return collection.find(Filters.eq("_id", objectId)).first();
        } catch (Exception e) {
            System.err.println("Error reading document: " + e.getMessage());
            return null;
        }
    }
    
    // Update operation
    public boolean updateDocument(String id, String name, String email, int age) {
        try {
            ObjectId objectId = new ObjectId(id);
            Document updateDoc = new Document();
            if (name != null) updateDoc.append("name", name);
            if (email != null) updateDoc.append("email", email);
            if (age > 0) updateDoc.append("age", age);
            
            collection.updateOne(Filters.eq("_id", objectId), 
                    new Document("$set", updateDoc));
            return true;
        } catch (Exception e) {
            System.err.println("Error updating document: " + e.getMessage());
            return false;
        }
    }
    
    // Delete operation
    public boolean deleteDocument(String id) {
        try {
            ObjectId objectId = new ObjectId(id);
            collection.deleteOne(Filters.eq("_id", objectId));
            return true;
        } catch (Exception e) {
            System.err.println("Error deleting document: " + e.getMessage());
            return false;
        }
    }
    
    public void close() {
        if (mongoClient != null) {
            mongoClient.close();
        }
    }
    
    public static void main(String[] args) {
        Task112 task = new Task112();
        
        System.out.println("=== Test Case 1: Create Document ===");
        String id1 = task.createDocument("John Doe", "john@example.com", 30);
        System.out.println("Created document with ID: " + id1);
        
        System.out.println("\\n=== Test Case 2: Read Document ===");
        if (id1 != null) {
            Document doc = task.readDocument(id1);
            System.out.println("Retrieved document: " + doc);
        }
        
        System.out.println("\\n=== Test Case 3: Create Multiple Documents ===");
        String id2 = task.createDocument("Jane Smith", "jane@example.com", 25);
        String id3 = task.createDocument("Bob Johnson", "bob@example.com", 35);
        System.out.println("Created documents with IDs: " + id2 + ", " + id3);
        
        System.out.println("\\n=== Test Case 4: Update Document ===");
        if (id1 != null) {
            boolean updated = task.updateDocument(id1, "John Updated", null, 31);
            System.out.println("Update successful: " + updated);
            Document updatedDoc = task.readDocument(id1);
            System.out.println("Updated document: " + updatedDoc);
        }
        
        System.out.println("\\n=== Test Case 5: Delete Document ===");
        if (id1 != null) {
            boolean deleted = task.deleteDocument(id1);
            System.out.println("Delete successful: " + deleted);
            Document deletedDoc = task.readDocument(id1);
            System.out.println("Document after deletion: " + deletedDoc);
        }
        
        task.close();
    }
}
