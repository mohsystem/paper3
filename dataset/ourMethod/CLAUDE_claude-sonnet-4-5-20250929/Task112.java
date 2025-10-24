
import com.mongodb.client.MongoClient;
import com.mongodb.client.MongoClients;
import com.mongodb.client.MongoCollection;
import com.mongodb.client.MongoDatabase;
import com.mongodb.client.result.DeleteResult;
import com.mongodb.client.result.UpdateResult;
import org.bson.Document;
import org.bson.types.ObjectId;

import java.util.ArrayList;
import java.util.List;
import java.util.regex.Pattern;

import static com.mongodb.client.model.Filters.eq;

public class Task112 {
    private static final Pattern VALID_DB_NAME = Pattern.compile("^[a-zA-Z0-9_-]{1,64}$");
    private static final Pattern VALID_COLLECTION_NAME = Pattern.compile("^[a-zA-Z0-9_-]{1,255}$");
    private static final int MAX_STRING_LENGTH = 10000;

    private final MongoClient mongoClient;
    private final MongoDatabase database;
    private final MongoCollection<Document> collection;

    public Task112(String connectionString, String databaseName, String collectionName) {
        if (connectionString == null || connectionString.trim().isEmpty()) {
            throw new IllegalArgumentException("Connection string cannot be null or empty");
        }
        if (!isValidDatabaseName(databaseName)) {
            throw new IllegalArgumentException("Invalid database name");
        }
        if (!isValidCollectionName(collectionName)) {
            throw new IllegalArgumentException("Invalid collection name");
        }

        try {
            this.mongoClient = MongoClients.create(connectionString);
            this.database = mongoClient.getDatabase(databaseName);
            this.collection = database.getCollection(collectionName);
        } catch (Exception e) {
            throw new RuntimeException("Failed to connect to MongoDB: " + e.getMessage());
        }
    }

    private boolean isValidDatabaseName(String name) {
        return name != null && VALID_DB_NAME.matcher(name).matches();
    }

    private boolean isValidCollectionName(String name) {
        return name != null && VALID_COLLECTION_NAME.matcher(name).matches();
    }

    private String validateString(String input, String fieldName) {
        if (input == null) {
            throw new IllegalArgumentException(fieldName + " cannot be null");
        }
        if (input.length() > MAX_STRING_LENGTH) {
            throw new IllegalArgumentException(fieldName + " exceeds maximum length");
        }
        return input;
    }

    private int validatePositiveInt(int value, String fieldName) {
        if (value < 0) {
            throw new IllegalArgumentException(fieldName + " must be non-negative");
        }
        return value;
    }

    public String createDocument(String name, String email, int age) {
        try {
            validateString(name, "Name");
            validateString(email, "Email");
            validatePositiveInt(age, "Age");

            if (age > 150) {
                throw new IllegalArgumentException("Age must be realistic");
            }

            Document doc = new Document("name", name)
                    .append("email", email)
                    .append("age", age);

            collection.insertOne(doc);
            return doc.getObjectId("_id").toString();
        } catch (IllegalArgumentException e) {
            throw e;
        } catch (Exception e) {
            throw new RuntimeException("Failed to create document: " + e.getMessage());
        }
    }

    public Document readDocument(String id) {
        try {
            validateString(id, "ID");
            if (!ObjectId.isValid(id)) {
                throw new IllegalArgumentException("Invalid ObjectId format");
            }

            return collection.find(eq("_id", new ObjectId(id))).first();
        } catch (IllegalArgumentException e) {
            throw e;
        } catch (Exception e) {
            throw new RuntimeException("Failed to read document: " + e.getMessage());
        }
    }

    public boolean updateDocument(String id, String name, String email, int age) {
        try {
            validateString(id, "ID");
            if (!ObjectId.isValid(id)) {
                throw new IllegalArgumentException("Invalid ObjectId format");
            }
            validateString(name, "Name");
            validateString(email, "Email");
            validatePositiveInt(age, "Age");

            if (age > 150) {
                throw new IllegalArgumentException("Age must be realistic");
            }

            Document updateDoc = new Document("$set", new Document("name", name)
                    .append("email", email)
                    .append("age", age));

            UpdateResult result = collection.updateOne(eq("_id", new ObjectId(id)), updateDoc);
            return result.getModifiedCount() > 0;
        } catch (IllegalArgumentException e) {
            throw e;
        } catch (Exception e) {
            throw new RuntimeException("Failed to update document: " + e.getMessage());
        }
    }

    public boolean deleteDocument(String id) {
        try {
            validateString(id, "ID");
            if (!ObjectId.isValid(id)) {
                throw new IllegalArgumentException("Invalid ObjectId format");
            }

            DeleteResult result = collection.deleteOne(eq("_id", new ObjectId(id)));
            return result.getDeletedCount() > 0;
        } catch (IllegalArgumentException e) {
            throw e;
        } catch (Exception e) {
            throw new RuntimeException("Failed to delete document: " + e.getMessage());
        }
    }

    public void close() {
        if (mongoClient != null) {
            mongoClient.close();
        }
    }

    public static void main(String[] args) {
        String connectionString = System.getenv("MONGODB_URI");
        if (connectionString == null || connectionString.trim().isEmpty()) {
            connectionString = "mongodb://localhost:27017";
        }

        Task112 mongoOps = null;
        try {
            mongoOps = new Task112(connectionString, "testdb", "users");

            System.out.println("Test 1: Create document");
            String id1 = mongoOps.createDocument("Alice Smith", "alice@example.com", 30);
            System.out.println("Created document with ID: " + id1);

            System.out.println("\\nTest 2: Read document");
            Document doc = mongoOps.readDocument(id1);
            System.out.println("Read document: " + (doc != null ? doc.toJson() : "null"));

            System.out.println("\\nTest 3: Update document");
            boolean updated = mongoOps.updateDocument(id1, "Alice Johnson", "alice.j@example.com", 31);
            System.out.println("Update successful: " + updated);

            System.out.println("\\nTest 4: Create multiple documents");
            String id2 = mongoOps.createDocument("Bob Brown", "bob@example.com", 25);
            String id3 = mongoOps.createDocument("Charlie Davis", "charlie@example.com", 35);
            System.out.println("Created documents with IDs: " + id2 + ", " + id3);

            System.out.println("\\nTest 5: Delete document");
            boolean deleted = mongoOps.deleteDocument(id1);
            System.out.println("Delete successful: " + deleted);

            mongoOps.deleteDocument(id2);
            mongoOps.deleteDocument(id3);

        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        } finally {
            if (mongoOps != null) {
                mongoOps.close();
            }
        }
    }
}
