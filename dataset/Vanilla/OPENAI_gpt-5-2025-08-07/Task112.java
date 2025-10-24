import com.mongodb.client.*;
import com.mongodb.client.result.DeleteResult;
import com.mongodb.client.result.InsertOneResult;
import com.mongodb.client.result.UpdateResult;
import org.bson.Document;

import static com.mongodb.client.model.Filters.eq;
import static com.mongodb.client.model.Updates.combine;
import static com.mongodb.client.model.Updates.set;

public class Task112 {

    public static boolean createDocument(MongoCollection<Document> collection, String id, String name, int age) {
        try {
            Document doc = new Document("_id", id).append("name", name).append("age", age);
            InsertOneResult res = collection.insertOne(doc);
            return res.wasAcknowledged();
        } catch (Exception e) {
            return false;
        }
    }

    public static Document readDocument(MongoCollection<Document> collection, String id) {
        try {
            return collection.find(eq("_id", id)).first();
        } catch (Exception e) {
            return null;
        }
    }

    public static boolean updateDocument(MongoCollection<Document> collection, String id, String newName, int newAge) {
        try {
            UpdateResult res = collection.updateOne(eq("_id", id), combine(set("name", newName), set("age", newAge)));
            return res.getModifiedCount() > 0;
        } catch (Exception e) {
            return false;
        }
    }

    public static boolean deleteDocument(MongoCollection<Document> collection, String id) {
        try {
            DeleteResult res = collection.deleteOne(eq("_id", id));
            return res.getDeletedCount() > 0;
        } catch (Exception e) {
            return false;
        }
    }

    public static void main(String[] args) {
        String uri = "mongodb://localhost:27017";
        try (MongoClient client = MongoClients.create(uri)) {
            MongoDatabase db = client.getDatabase("task112db");
            MongoCollection<Document> people = db.getCollection("java_people");
            // Cleanup old test docs
            people.deleteMany(new Document("_id", new Document("$in", java.util.Arrays.asList("u1", "u2", "u3", "u4", "u5"))));

            // Test case 1: Create
            System.out.println("Create u1: " + createDocument(people, "u1", "Alice", 30));

            // Test case 2: Read
            System.out.println("Read u1: " + readDocument(people, "u1"));

            // Test case 3: Update
            System.out.println("Update u1: " + updateDocument(people, "u1", "Alicia", 31));
            System.out.println("Read u1 after update: " + readDocument(people, "u1"));

            // Test case 4: Create another
            System.out.println("Create u2: " + createDocument(people, "u2", "Bob", 25));

            // Test case 5: Delete
            System.out.println("Delete u2: " + deleteDocument(people, "u2"));
        }
    }
}