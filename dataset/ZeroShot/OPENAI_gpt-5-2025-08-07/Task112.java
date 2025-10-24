import com.mongodb.ConnectionString;
import com.mongodb.MongoClientSettings;
import com.mongodb.client.MongoClient;
import com.mongodb.client.MongoClients;
import com.mongodb.client.MongoCollection;
import com.mongodb.client.MongoDatabase;
import com.mongodb.client.FindIterable;
import com.mongodb.client.result.DeleteResult;
import com.mongodb.client.result.InsertOneResult;
import com.mongodb.client.result.UpdateResult;
import org.bson.Document;
import org.bson.types.ObjectId;
import org.bson.json.JsonWriterSettings;

import java.util.*;
import java.util.concurrent.TimeUnit;

import static com.mongodb.client.model.Filters.eq;

public class Task112 {

    // Build a secure MongoClient from environment variable MONGODB_URI or fallback to local
    public static MongoClient createClient() {
        String uri = System.getenv("MONGODB_URI");
        if (uri == null || uri.isEmpty()) {
            uri = "mongodb://localhost:27017";
        }
        MongoClientSettings settings = MongoClientSettings.builder()
                .applyConnectionString(new ConnectionString(uri))
                .applyToSocketSettings(builder -> builder.connectTimeout(10, TimeUnit.SECONDS).readTimeout(15, TimeUnit.SECONDS))
                .applyToConnectionPoolSettings(builder -> builder.maxSize(20).minSize(0))
                .build();
        return MongoClients.create(settings);
    }

    // Validate field names for security: no keys starting with '$' or containing '.'
    private static void validateKeys(Map<String, Object> map) {
        for (String k : map.keySet()) {
            if (k == null || k.isEmpty() || k.startsWith("$") || k.contains(".")) {
                throw new IllegalArgumentException("Invalid field name: " + k);
            }
            Object val = map.get(k);
            if (val instanceof Map) {
                //noinspection unchecked
                validateKeys((Map<String, Object>) val);
            } else if (val instanceof List) {
                for (Object item : (List<?>) val) {
                    if (item instanceof Map) {
                        //noinspection unchecked
                        validateKeys((Map<String, Object>) item);
                    }
                }
            }
        }
    }

    private static Document toDocument(Map<String, Object> map) {
        Document d = new Document();
        for (Map.Entry<String, Object> e : map.entrySet()) {
            Object v = e.getValue();
            if (v instanceof Map) {
                //noinspection unchecked
                d.append(e.getKey(), toDocument((Map<String, Object>) v));
            } else if (v instanceof List) {
                List<?> lst = (List<?>) v;
                List<Object> conv = new ArrayList<>(lst.size());
                for (Object item : lst) {
                    if (item instanceof Map) {
                        //noinspection unchecked
                        conv.add(toDocument((Map<String, Object>) item));
                    } else {
                        conv.add(item);
                    }
                }
                d.append(e.getKey(), conv);
            } else {
                d.append(e.getKey(), v);
            }
        }
        return d;
    }

    private static ObjectId safeObjectId(String id) {
        try {
            return new ObjectId(id);
        } catch (Exception e) {
            throw new IllegalArgumentException("Invalid ObjectId: " + id, e);
        }
    }

    private static MongoCollection<Document> getCollection(MongoClient client, String dbName, String collectionName) {
        Objects.requireNonNull(client, "client");
        Objects.requireNonNull(dbName, "dbName");
        Objects.requireNonNull(collectionName, "collectionName");
        MongoDatabase db = client.getDatabase(dbName);
        return db.getCollection(collectionName);
    }

    public static String createOne(MongoClient client, String dbName, String collectionName, Map<String, Object> data) {
        Objects.requireNonNull(data, "data");
        validateKeys(data);
        Document doc = toDocument(data);
        MongoCollection<Document> col = getCollection(client, dbName, collectionName);
        InsertOneResult res = col.insertOne(doc);
        ObjectId id = res.getInsertedId() != null ? res.getInsertedId().asObjectId().getValue() : doc.getObjectId("_id");
        return id != null ? id.toHexString() : null;
    }

    public static String readOneById(MongoClient client, String dbName, String collectionName, String id) {
        MongoCollection<Document> col = getCollection(client, dbName, collectionName);
        Document found = col.find(eq("_id", safeObjectId(id))).limit(1).first();
        if (found == null) return null;
        return found.toJson(JsonWriterSettings.builder().outputMode(org.bson.json.JsonMode.RELAXED).build());
    }

    public static long updateOneById(MongoClient client, String dbName, String collectionName, String id, Map<String, Object> updates) {
        Objects.requireNonNull(updates, "updates");
        validateKeys(updates);
        Document setDoc = toDocument(updates);
        Document updateWrapper = new Document("$set", setDoc);
        MongoCollection<Document> col = getCollection(client, dbName, collectionName);
        UpdateResult res = col.updateOne(eq("_id", safeObjectId(id)), updateWrapper);
        return res.getModifiedCount();
    }

    public static long deleteOneById(MongoClient client, String dbName, String collectionName, String id) {
        MongoCollection<Document> col = getCollection(client, dbName, collectionName);
        DeleteResult res = col.deleteOne(eq("_id", safeObjectId(id)));
        return res.getDeletedCount();
    }

    public static List<String> listAll(MongoClient client, String dbName, String collectionName, int limit) {
        MongoCollection<Document> col = getCollection(client, dbName, collectionName);
        FindIterable<Document> it = col.find().limit(Math.max(0, limit));
        List<String> out = new ArrayList<>();
        JsonWriterSettings settings = JsonWriterSettings.builder().outputMode(org.bson.json.JsonMode.RELAXED).build();
        for (Document d : it) {
            out.add(d.toJson(settings));
        }
        return out;
    }

    public static void main(String[] args) {
        try (MongoClient client = createClient()) {
            String db = "task112_db";
            String coll = "items";

            // Clean collection for test isolation (safe attempt)
            try {
                client.getDatabase(db).getCollection(coll).drop();
            } catch (Exception ignored) {}

            // Test case 1: Create
            Map<String, Object> data = new HashMap<>();
            data.put("name", "Widget");
            data.put("qty", 10);
            data.put("meta", Map.of("color", "blue", "fragile", true));
            String id = createOne(client, db, coll, data);
            System.out.println("Created ID: " + id);

            // Test case 2: Read
            String readJson = readOneById(client, db, coll, id);
            System.out.println("Read: " + readJson);

            // Test case 3: Update
            Map<String, Object> updates = new HashMap<>();
            updates.put("qty", 42);
            updates.put("tags", List.of("sale", "new"));
            long modified = updateOneById(client, db, coll, id, updates);
            System.out.println("Modified count: " + modified);

            // Test case 4: List
            List<String> all = listAll(client, db, coll, 10);
            System.out.println("List count: " + all.size());
            for (String j : all) System.out.println(j);

            // Test case 5: Delete and verify
            long deleted = deleteOneById(client, db, coll, id);
            System.out.println("Deleted count: " + deleted);
            String shouldBeNull = readOneById(client, db, coll, id);
            System.out.println("Post-delete read (should be null): " + shouldBeNull);
        } catch (Exception e) {
            // Basic error reporting without leaking sensitive info
            System.err.println("An error occurred. Details: " + e.getClass().getSimpleName() + ": " + e.getMessage());
        }
    }
}