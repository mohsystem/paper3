import com.mongodb.client.MongoClient;
import com.mongodb.client.MongoClients;
import com.mongodb.client.MongoCollection;
import com.mongodb.client.MongoDatabase;
import com.mongodb.client.model.Filters;
import com.mongodb.client.model.Updates;
import com.mongodb.client.result.DeleteResult;
import com.mongodb.client.result.InsertOneResult;
import com.mongodb.client.result.UpdateResult;
import org.bson.Document;
import org.bson.types.ObjectId;

import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * Note: To run this code, you need the MongoDB Java driver.
 * Add the following dependency to your pom.xml (Maven):
 * <dependency>
 *     <groupId>org.mongodb</groupId>
 *     <artifactId>mongodb-driver-sync</artifactId>
 *     <version>4.11.1</version>
 * </dependency>
 * Or download the JAR manually.
 * Also, ensure your MongoDB server is running on the default port (27017).
 */
public class Task112 {

    // IMPORTANT: In a production environment, connection strings should never be hard-coded.
    // They should be retrieved from a secure source like environment variables or a secrets manager.
    // The user in the connection string should have the minimum required permissions.
    private static final String CONNECTION_STRING = "mongodb://localhost:27017";
    private static final String DATABASE_NAME = "mySecureDb";
    private static final String COLLECTION_NAME = "users";

    static {
        // Disable excessive MongoDB logging
        Logger.getLogger("org.mongodb.driver").setLevel(Level.WARNING);
    }

    /**
     * Creates a new document in the collection.
     * @param client The MongoClient instance.
     * @param userDocument The document to insert.
     * @return The ObjectId of the newly created document.
     */
    public static ObjectId createDocument(MongoClient client, Document userDocument) {
        try {
            MongoDatabase database = client.getDatabase(DATABASE_NAME);
            MongoCollection<Document> collection = database.getCollection(COLLECTION_NAME);

            // Input validation should be performed here in a real application
            // to prevent NoSQL injection if data comes from external sources.
            InsertOneResult result = collection.insertOne(userDocument);
            System.out.println("Create: Success. Inserted document with id: " + result.getInsertedId());
            return result.getInsertedId().asObjectId().getValue();
        } catch (Exception e) {
            System.err.println("An error occurred during create operation: " + e.getMessage());
            return null;
        }
    }

    /**
     * Reads a document from the collection by its ID.
     * @param client The MongoClient instance.
     * @param id The ObjectId of the document to find.
     * @return The found document or null if not found.
     */
    public static Document readDocument(MongoClient client, ObjectId id) {
        try {
            MongoDatabase database = client.getDatabase(DATABASE_NAME);
            MongoCollection<Document> collection = database.getCollection(COLLECTION_NAME);

            Document foundDocument = collection.find(Filters.eq("_id", id)).first();
            if (foundDocument != null) {
                System.out.println("Read: Success. Found document: " + foundDocument.toJson());
            } else {
                System.out.println("Read: Document with id " + id + " not found.");
            }
            return foundDocument;
        } catch (Exception e) {
            System.err.println("An error occurred during read operation: " + e.getMessage());
            return null;
        }
    }

    /**
     * Updates a document in the collection.
     * @param client The MongoClient instance.
     * @param id The ObjectId of the document to update.
     * @param field The field to update.
     * @param value The new value for the field.
     * @return true if the update was successful, false otherwise.
     */
    public static boolean updateDocument(MongoClient client, ObjectId id, String field, Object value) {
        try {
            MongoDatabase database = client.getDatabase(DATABASE_NAME);
            MongoCollection<Document> collection = database.getCollection(COLLECTION_NAME);

            UpdateResult result = collection.updateOne(Filters.eq("_id", id), Updates.set(field, value));
            System.out.println("Update: Success. Matched " + result.getMatchedCount() + " and modified " + result.getModifiedCount() + " document(s).");
            return result.getModifiedCount() > 0;
        } catch (Exception e) {
            System.err.println("An error occurred during update operation: " + e.getMessage());
            return false;
        }
    }

    /**
     * Deletes a document from the collection by its ID.
     * @param client The MongoClient instance.
     * @param id The ObjectId of the document to delete.
     * @return true if the deletion was successful, false otherwise.
     */
    public static boolean deleteDocument(MongoClient client, ObjectId id) {
        try {
            MongoDatabase database = client.getDatabase(DATABASE_NAME);
            MongoCollection<Document> collection = database.getCollection(COLLECTION_NAME);

            DeleteResult result = collection.deleteOne(Filters.eq("_id", id));
            System.out.println("Delete: Success. Deleted " + result.getDeletedCount() + " document(s).");
            return result.getDeletedCount() > 0;
        } catch (Exception e) {
            System.err.println("An error occurred during delete operation: " + e.getMessage());
            return false;
        }
    }

    public static void main(String[] args) {
        // Using try-with-resources to ensure the client is closed automatically
        try (MongoClient mongoClient = MongoClients.create(CONNECTION_STRING)) {
            System.out.println("Successfully connected to MongoDB.");
            
            // Cleanup before tests
            mongoClient.getDatabase(DATABASE_NAME).getCollection(COLLECTION_NAME).drop();
            System.out.println("--- Starting Test Cases ---");
            
            // --- Test Case 1: Create a new user ---
            System.out.println("\n1. === CREATE Operation ===");
            Document newUser = new Document("name", "John Doe")
                    .append("age", 30)
                    .append("email", "john.doe@example.com");
            ObjectId userId = createDocument(mongoClient, newUser);

            if (userId != null) {
                // --- Test Case 2: Read the created user ---
                System.out.println("\n2. === READ Operation (after create) ===");
                readDocument(mongoClient, userId);

                // --- Test Case 3: Update the user's age ---
                System.out.println("\n3. === UPDATE Operation ===");
                updateDocument(mongoClient, userId, "age", 31);

                // --- Test Case 4: Read the updated user ---
                System.out.println("\n4. === READ Operation (after update) ===");
                readDocument(mongoClient, userId);

                // --- Test Case 5: Delete the user ---
                System.out.println("\n5. === DELETE Operation ===");
                deleteDocument(mongoClient, userId);
                
                // --- Final check: try to read the deleted user ---
                System.out.println("\n--- Final Check: Verifying Deletion ---");
                readDocument(mongoClient, userId);
            } else {
                 System.out.println("Test cases aborted due to creation failure.");
            }
             System.out.println("\n--- Test Cases Finished ---");

        } catch (Exception e) {
            System.err.println("Failed to connect to MongoDB or execute operations: " + e.getMessage());
        }
    }
}