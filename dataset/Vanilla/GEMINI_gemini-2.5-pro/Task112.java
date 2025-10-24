import com.mongodb.client.MongoClients;
import com.mongodb.client.MongoClient;
import com.mongodb.client.MongoDatabase;
import com.mongodb.client.MongoCollection;
import com.mongodb.client.result.DeleteResult;
import com.mongodb.client.result.UpdateResult;
import org.bson.Document;
import com.mongodb.client.model.Updates;
import static com.mongodb.client.model.Filters.eq;

/*
NOTE: To run this code, you need the MongoDB Java driver.
You can add it to your project using Maven or Gradle.

Maven dependency:
<dependency>
    <groupId>org.mongodb</groupId>
    <artifactId>mongodb-driver-sync</artifactId>
    <version>4.11.1</version>
</dependency>
*/
public class Task112 {

    // CREATE
    public static void createDocument(MongoCollection<Document> collection, Document doc) {
        try {
            collection.insertOne(doc);
            System.out.println("Success: Document inserted.");
        } catch (Exception e) {
            System.err.println("Error inserting document: " + e);
        }
    }

    // READ
    public static Document readDocument(MongoCollection<Document> collection, String key, String value) {
        try {
            return collection.find(eq(key, value)).first();
        } catch (Exception e) {
            System.err.println("Error reading document: " + e);
            return null;
        }
    }

    // UPDATE
    public static void updateDocument(MongoCollection<Document> collection, String filterKey, String filterValue, String updateKey, Object updateValue) {
        try {
            UpdateResult result = collection.updateOne(eq(filterKey, filterValue), Updates.set(updateKey, updateValue));
            System.out.println("Success: Matched " + result.getMatchedCount() + " and modified " + result.getModifiedCount() + " document(s).");
        } catch (Exception e) {
            System.err.println("Error updating document: " + e);
        }
    }

    // DELETE
    public static void deleteDocument(MongoCollection<Document> collection, String key, String value) {
        try {
            DeleteResult result = collection.deleteOne(eq(key, value));
            System.out.println("Success: Deleted " + result.getDeletedCount() + " document(s).");
        } catch (Exception e) {
            System.err.println("Error deleting document: " + e);
        }
    }

    public static void main(String[] args) {
        // --- Connection Setup ---
        // Ensure you have MongoDB running on localhost:27017
        String uri = "mongodb://localhost:27017";
        String dbName = "javaTestDB";
        String collectionName = "users";

        try (MongoClient mongoClient = MongoClients.create(uri)) {
            MongoDatabase database = mongoClient.getDatabase(dbName);
            MongoCollection<Document> collection = database.getCollection(collectionName);
            
            // Clean up previous runs
            collection.drop();
            System.out.println("--- Starting CRUD Test Cases ---");

            // --- Test Case 1: Create ---
            System.out.println("\n1. CREATE Operation");
            Document doc1 = new Document("name", "Ada Lovelace")
                                  .append("age", 36)
                                  .append("profession", "Mathematician");
            createDocument(collection, doc1);

            // --- Test Case 2: Read ---
            System.out.println("\n2. READ Operation");
            Document foundDoc = readDocument(collection, "name", "Ada Lovelace");
            System.out.println("Found document: " + (foundDoc != null ? foundDoc.toJson() : "null"));

            // --- Test Case 3: Update ---
            System.out.println("\n3. UPDATE Operation");
            updateDocument(collection, "name", "Ada Lovelace", "age", 40);

            // --- Test Case 4: Read after Update ---
            System.out.println("\n4. READ Operation (after update)");
            Document updatedDoc = readDocument(collection, "name", "Ada Lovelace");
            System.out.println("Found updated document: " + (updatedDoc != null ? updatedDoc.toJson() : "null"));

            // --- Test Case 5: Delete ---
            System.out.println("\n5. DELETE Operation");
            deleteDocument(collection, "name", "Ada Lovelace");
            Document deletedDoc = readDocument(collection, "name", "Ada Lovelace");
            System.out.println("Document after deletion attempt: " + (deletedDoc != null ? deletedDoc.toJson() : "null"));

            System.out.println("\n--- CRUD Test Cases Finished ---");
            
            // --- Final Cleanup ---
            System.out.println("Dropping collection...");
            collection.drop();
            System.out.println("Collection dropped.");

        } catch (Exception e) {
            System.err.println("An error occurred: " + e);
        }
    }
}