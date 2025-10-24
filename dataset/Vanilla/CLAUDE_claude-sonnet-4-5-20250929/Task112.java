
import com.mongodb.client.MongoClient;
import com.mongodb.client.MongoClients;
import com.mongodb.client.MongoCollection;
import com.mongodb.client.MongoDatabase;
import org.bson.Document;
import com.mongodb.client.result.DeleteResult;
import com.mongodb.client.result.UpdateResult;
import static com.mongodb.client.model.Filters.*;
import static com.mongodb.client.model.Updates.*;

public class Task112 {
    private MongoClient mongoClient;
    private MongoDatabase database;
    private MongoCollection<Document> collection;

    public Task112(String connectionString, String dbName, String collectionName) {
        this.mongoClient = MongoClients.create(connectionString);
        this.database = mongoClient.getDatabase(dbName);
        this.collection = database.getCollection(collectionName);
    }

    // Create operation
    public String create(String name, int age, String email) {
        Document doc = new Document("name", name)
                .append("age", age)
                .append("email", email);
        collection.insertOne(doc);
        return "Created document with ID: " + doc.getObjectId("_id").toString();
    }

    // Read operation
    public String read(String name) {
        Document doc = collection.find(eq("name", name)).first();
        if (doc != null) {
            return "Found: " + doc.toJson();
        }
        return "Document not found";
    }

    // Update operation
    public String update(String name, int newAge) {
        UpdateResult result = collection.updateOne(eq("name", name), set("age", newAge));
        return "Modified count: " + result.getModifiedCount();
    }

    // Delete operation
    public String delete(String name) {
        DeleteResult result = collection.deleteOne(eq("name", name));
        return "Deleted count: " + result.getDeletedCount();
    }

    public void close() {
        if (mongoClient != null) {
            mongoClient.close();
        }
    }

    public static void main(String[] args) {
        // Note: Replace with your MongoDB connection string
        String connectionString = "mongodb://localhost:27017";
        String dbName = "testdb";
        String collectionName = "users";

        Task112 mongoOps = new Task112(connectionString, dbName, collectionName);

        System.out.println("Test Case 1: Create a document");
        System.out.println(mongoOps.create("John Doe", 30, "john@example.com"));

        System.out.println("\\nTest Case 2: Read a document");
        System.out.println(mongoOps.read("John Doe"));

        System.out.println("\\nTest Case 3: Update a document");
        System.out.println(mongoOps.update("John Doe", 31));

        System.out.println("\\nTest Case 4: Read updated document");
        System.out.println(mongoOps.read("John Doe"));

        System.out.println("\\nTest Case 5: Delete a document");
        System.out.println(mongoOps.delete("John Doe"));

        mongoOps.close();
    }
}
