import com.mongodb.ConnectionString;
import com.mongodb.MongoClientSettings;
import com.mongodb.ReadConcern;
import com.mongodb.WriteConcern;
import com.mongodb.client.*;
import com.mongodb.client.model.Filters;
import com.mongodb.client.model.Updates;
import com.mongodb.client.result.DeleteResult;
import com.mongodb.client.result.UpdateResult;
import org.bson.Document;
import org.bson.conversions.Bson;
import org.bson.types.ObjectId;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.TimeUnit;

public class Task112 {

    // Secure client creation using env var and timeouts
    public static MongoClient createClient() {
        String uri = System.getenv("MONGODB_URI");
        if (uri == null || uri.isBlank()) {
            uri = "mongodb://localhost:27017/?retryWrites=true&w=majority";
        }
        ConnectionString cs = new ConnectionString(uri);
        MongoClientSettings settings = MongoClientSettings.builder()
                .applyConnectionString(cs)
                .applyToClusterSettings(builder -> builder.serverSelectionTimeout(10, TimeUnit.SECONDS))
                .applyToSocketSettings(builder -> {
                    builder.connectTimeout(10, TimeUnit.SECONDS);
                    builder.readTimeout(15, TimeUnit.SECONDS);
                })
                .retryWrites(true)
                .build();
        return MongoClients.create(settings);
    }

    public static MongoCollection<Document> getCollection(MongoClient client, String dbName, String collName) {
        MongoDatabase db = client.getDatabase(dbName);
        return db.getCollection(collName).withReadConcern(ReadConcern.MAJORITY).withWriteConcern(WriteConcern.MAJORITY);
    }

    public static ObjectId createDocument(MongoCollection<Document> collection, Document doc) {
        if (collection == null || doc == null) throw new IllegalArgumentException("collection/doc cannot be null");
        collection.insertOne(doc);
        return doc.getObjectId("_id");
    }

    public static List<Document> readDocuments(MongoCollection<Document> collection, Bson filter) {
        if (collection == null) throw new IllegalArgumentException("collection cannot be null");
        if (filter == null) filter = new Document();
        List<Document> out = new ArrayList<>();
        try (MongoCursor<Document> cursor = collection.find(filter).iterator()) {
            while (cursor.hasNext()) out.add(cursor.next());
        }
        return out;
    }

    public static long updateDocuments(MongoCollection<Document> collection, Bson filter, Bson update) {
        if (collection == null || filter == null || update == null) throw new IllegalArgumentException("null args");
        UpdateResult r = collection.updateMany(filter, update);
        return r.getModifiedCount();
    }

    public static long deleteDocuments(MongoCollection<Document> collection, Bson filter) {
        if (collection == null || filter == null) throw new IllegalArgumentException("null args");
        DeleteResult r = collection.deleteMany(filter);
        return r.getDeletedCount();
    }

    // 5 test cases in main
    public static void main(String[] args) {
        String dbName = "task112db";
        String collName = "users";
        String tagValue = "task112";
        try (MongoClient client = createClient()) {
            MongoCollection<Document> coll = getCollection(client, dbName, collName);

            // Test 1: Cleanup old test data
            long cleaned = deleteDocuments(coll, Filters.eq("tag", tagValue));
            System.out.println("Test1-Cleanup deleted: " + cleaned);

            // Test 2: Create a document
            Document alice = new Document("username", "alice")
                    .append("email", "alice.task112@example.com")
                    .append("age", 30)
                    .append("tag", tagValue);
            ObjectId aliceId = createDocument(coll, alice);
            System.out.println("Test2-Create insertedId: " + aliceId);

            // Test 3: Read the document
            List<Document> found = readDocuments(coll, Filters.and(Filters.eq("username", "alice"), Filters.eq("tag", tagValue)));
            System.out.println("Test3-Read count: " + found.size());

            // Test 4: Update the document
            long modified = updateDocuments(coll,
                    Filters.and(Filters.eq("username", "alice"), Filters.eq("tag", tagValue)),
                    Updates.set("age", 31));
            System.out.println("Test4-Update modified: " + modified);

            // Test 5: Delete the document
            long deleted = deleteDocuments(coll, Filters.and(Filters.eq("username", "alice"), Filters.eq("tag", tagValue)));
            System.out.println("Test5-Delete deleted: " + deleted);
        }
    }
}