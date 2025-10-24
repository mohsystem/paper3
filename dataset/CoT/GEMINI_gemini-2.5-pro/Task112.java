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
import org.bson.conversions.Bson;
import org.bson.types.ObjectId;

public class Task112 {

    private MongoCollection<Document> collection;
    private MongoClient mongoClient;

    /**
     * Connects to the MongoDB database.
     * @param connectionString The connection string.
     * @param dbName The name of the database.
     * @param collectionName The name of the collection.
     */
    public boolean connect(String connectionString, String dbName, String collectionName) {
        try {
            mongoClient = MongoClients.create(connectionString);
            MongoDatabase database = mongoClient.getDatabase(dbName);
            collection = database.getCollection(collectionName);
            return true;
        } catch (Exception e) {
            System.err.println("Error connecting to MongoDB: " + e.getMessage());
            return false;
        }
    }

    /**
     * Creates (inserts) a new document.
     * @param doc The document to insert.
     * @return The ObjectId of the inserted document, or null on failure.
     */
    public ObjectId createUser(Document doc) {
        try {
            InsertOneResult result = collection.insertOne(doc);
            return result.getInsertedId().asObjectId().getValue();
        } catch (Exception e) {
            System.err.println("Error creating user: " + e.getMessage());
            return null;
        }
    }

    /**
     * Reads a document based on a filter.
     * @param filter The BSON filter to find the document.
     * @return The found document, or null if not found.
     */
    public Document readUser(Bson filter) {
        try {
            return collection.find(filter).first();
        } catch (Exception e) {
            System.err.println("Error reading user: " + e.getMessage());
            return null;
        }
    }

    /**
     * Updates a single document.
     * @param filter The BSON filter to find the document.
     * @param update The BSON update operation.
     * @return The number of documents modified.
     */
    public long updateUser(Bson filter, Bson update) {
        try {
            UpdateResult result = collection.updateOne(filter, update);
            return result.getModifiedCount();
        } catch (Exception e) {
            System.err.println("Error updating user: " + e.getMessage());
            return 0;
        }
    }



    /**
     * Deletes a single document.
     * @param filter The BSON filter to find the document.
     * @return The number of documents deleted.
     */
    public long deleteUser(Bson filter) {
        try {
            DeleteResult result = collection.deleteOne(filter);
            return result.getDeletedCount();
        } catch (Exception e) {
            System.err.println("Error deleting user: " + e.getMessage());
            return 0;
        }
    }

    /**
     * Closes the MongoDB client connection.
     */
    public void close() {
        if (mongoClient != null) {
            mongoClient.close();
        }
    }

    public static void main(String[] args) {
        // NOTE: You need the MongoDB Java Driver in your classpath.
        // Example Maven dependency:
        // <dependency>
        //     <groupId>org.mongodb</groupId>
        //     <artifactId>mongodb-driver-sync</artifactId>
        //     <version>4.11.1</version>
        // </dependency>
        
        Task112 crud = new Task112();
        String connectionString = "mongodb://localhost:27017";
        String dbName = "java_test_db";
        String collectionName = "users";

        if (!crud.connect(connectionString, dbName, collectionName)) {
            return;
        }

        // Clean up collection before starting tests
        crud.collection.drop();
        System.out.println("--- Starting MongoDB CRUD Test Cases ---");

        // Test Case 1: Create
        System.out.println("\n--- Test Case 1: Create User ---");
        Document newUser = new Document("name", "John Doe")
                .append("age", 30)
                .append("city", "New York");
        ObjectId userId = crud.createUser(newUser);
        if (userId != null) {
            System.out.println("User created successfully with ID: " + userId);
        } else {
            System.out.println("User creation failed.");
        }

        // Test Case 2: Read
        System.out.println("\n--- Test Case 2: Read User ---");
        Bson readFilter = Filters.eq("_id", userId);
        Document foundUser = crud.readUser(readFilter);
        if (foundUser != null) {
            System.out.println("User found: " + foundUser.toJson());
        } else {
            System.out.println("User not found.");
        }

        // Test Case 3: Update
        System.out.println("\n--- Test Case 3: Update User ---");
        Bson updateOperation = Updates.set("age", 31);
        long modifiedCount = crud.updateUser(readFilter, updateOperation);
        System.out.println("Users updated: " + modifiedCount);

        // Test Case 4: Read after Update
        System.out.println("\n--- Test Case 4: Read User After Update ---");
        Document updatedUser = crud.readUser(readFilter);
        if (updatedUser != null) {
            System.out.println("User found after update: " + updatedUser.toJson());
        } else {
            System.out.println("User not found.");
        }

        // Test Case 5: Delete
        System.out.println("\n--- Test Case 5: Delete User ---");
        long deletedCount = crud.deleteUser(readFilter);
        System.out.println("Users deleted: " + deletedCount);
        Document deletedUser = crud.readUser(readFilter);
        if (deletedUser == null) {
            System.out.println("User successfully deleted and not found.");
        } else {
            System.out.println("User deletion failed.");
        }
        
        crud.close();
        System.out.println("\n--- Tests Finished ---");
    }
}