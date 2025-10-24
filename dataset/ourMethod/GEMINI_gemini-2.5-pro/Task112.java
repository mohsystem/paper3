/*
 * To compile and run this code, you need the MongoDB Java driver.
 * You can add it as a dependency in your project.
 * For Maven, add this to your pom.xml:
 *
 * <dependency>
 *     <groupId>org.mongodb</groupId>
 *     <artifactId>mongodb-driver-sync</artifactId>
 *     <version>4.11.1</version>
 * </dependency>
 *
 * Also ensure that a MongoDB instance is running on localhost:27017.
 */
import com.mongodb.client.MongoClient;
import com.mongodb.client.MongoClients;
import com.mongodb.client.MongoCollection;
import com.mongodb.client.MongoDatabase;
import com.mongodb.client.result.DeleteResult;
import com.mongodb.client.result.InsertOneResult;
import com.mongodb.client.result.UpdateResult;
import com.mongodb.MongoException;

import org.bson.Document;
import org.bson.types.ObjectId;

import static com.mongodb.client.model.Filters.eq;
import static com.mongodb.client.model.Updates.set;

import java.util.Objects;

public class Task112 {

    // NOTE: For a production environment, the connection string should not be hardcoded.
    // It should be loaded from a secure configuration source or environment variables.
    private static final String URI = "mongodb://localhost:27017";
    private static final String DATABASE_NAME = "my_java_db";
    private static final String COLLECTION_NAME = "users";

    /**
     * Creates a new document in the specified collection.
     * @param collection The MongoDB collection.
     * @param name The name of the user.
     * @param email The email of the user.
     * @return The ObjectId of the inserted document as a hex string, or null on failure.
     */
    public static String createUser(MongoCollection<Document> collection, String name, String email) {
        if (name == null || name.trim().isEmpty() || email == null || email.trim().isEmpty()) {
            System.err.println("Error: Name and email cannot be null or empty.");
            return null;
        }
        try {
            Document doc = new Document("name", name).append("email", email);
            InsertOneResult result = collection.insertOne(doc);
            ObjectId id = Objects.requireNonNull(result.getInsertedId()).asObjectId().getValue();
            return id.toHexString();
        } catch (MongoException e) {
            System.err.println("Error creating user: " + e.getMessage());
            return null;
        }
    }

    /**
     * Reads a document from the collection by name.
     * @param collection The MongoDB collection.
     * @param name The name of the user to find.
     * @return The found Document, or null if not found or an error occurs.
     */
    public static Document readUserByName(MongoCollection<Document> collection, String name) {
        if (name == null || name.trim().isEmpty()) {
            System.err.println("Error: Name cannot be null or empty.");
            return null;
        }
        try {
            return collection.find(eq("name", name)).first();
        } catch (MongoException e) {
            System.err.println("Error reading user: " + e.getMessage());
            return null;
        }
    }

    /**
     * Updates a user's email by their name.
     * @param collection The MongoDB collection.
     * @param name The name of the user to update.
     * @param newEmail The new email address.
     * @return true if the update was successful (at least one document modified), false otherwise.
     */
    public static boolean updateUserEmail(MongoCollection<Document> collection, String name, String newEmail) {
        if (name == null || name.trim().isEmpty() || newEmail == null || newEmail.trim().isEmpty()) {
            System.err.println("Error: Name and new email cannot be null or empty.");
            return false;
        }
        try {
            UpdateResult result = collection.updateOne(eq("name", name), set("email", newEmail));
            return result.getModifiedCount() > 0;
        } catch (MongoException e) {
            System.err.println("Error updating user: " + e.getMessage());
            return false;
        }
    }

    /**
     * Deletes a user by their name.
     * @param collection The MongoDB collection.
     * @param name The name of the user to delete.
     * @return true if the deletion was successful (at least one document deleted), false otherwise.
     */
    public static boolean deleteUser(MongoCollection<Document> collection, String name) {
        if (name == null || name.trim().isEmpty()) {
            System.err.println("Error: Name cannot be null or empty.");
            return false;
        }
        try {
            DeleteResult result = collection.deleteOne(eq("name", name));
            return result.getDeletedCount() > 0;
        } catch (MongoException e) {
            System.err.println("Error deleting user: " + e.getMessage());
            return false;
        }
    }

    public static void main(String[] args) {
        // Using try-with-resources to ensure the client is closed automatically.
        try (MongoClient mongoClient = MongoClients.create(URI)) {
            MongoDatabase database = mongoClient.getDatabase(DATABASE_NAME);
            MongoCollection<Document> collection = database.getCollection(COLLECTION_NAME);

            // Clean up the collection before running tests
            try {
                collection.drop();
                System.out.println("Collection dropped for a clean test run.");
            } catch (MongoException e) {
                // Ignore if collection doesn't exist
            }

            System.out.println("--- MongoDB CRUD Operations in Java ---");

            // 1. Create Operation
            System.out.println("\n[Test Case 1: Create]");
            String newUserId = createUser(collection, "John Doe", "john.doe@example.com");
            if (newUserId != null) {
                System.out.println("User created successfully with ID: " + newUserId);
            } else {
                System.out.println("User creation failed.");
            }

            // 2. Read Operation
            System.out.println("\n[Test Case 2: Read]");
            Document user = readUserByName(collection, "John Doe");
            if (user != null) {
                System.out.println("User found: " + user.toJson());
            } else {
                System.out.println("User 'John Doe' not found.");
            }

            // 3. Update Operation
            System.out.println("\n[Test Case 3: Update]");
            boolean updated = updateUserEmail(collection, "John Doe", "j.doe.new@example.com");
            if (updated) {
                System.out.println("User 'John Doe' updated successfully.");
            } else {
                System.out.println("User update failed.");
            }

            // 4. Read again to verify update
            System.out.println("\n[Test Case 4: Verify Update]");
            user = readUserByName(collection, "John Doe");
            if (user != null) {
                System.out.println("User found after update: " + user.toJson());
            } else {
                System.out.println("User 'John Doe' not found after update.");
            }

            // 5. Delete Operation
            System.out.println("\n[Test Case 5: Delete]");
            boolean deleted = deleteUser(collection, "John Doe");
            if (deleted) {
                System.out.println("User 'John Doe' deleted successfully.");
            } else {
                System.out.println("User deletion failed.");
            }
            
            // Verify deletion
            user = readUserByName(collection, "John Doe");
            if (user == null) {
                System.out.println("Successfully verified that 'John Doe' is no longer in the database.");
            } else {
                System.out.println("Verification failed: 'John Doe' still exists.");
            }

        } catch (MongoException e) {
            System.err.println("An error occurred with MongoDB connection: " + e.getMessage());
        }
    }
}