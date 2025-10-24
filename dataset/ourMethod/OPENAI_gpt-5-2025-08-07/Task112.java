import com.mongodb.ConnectionString;
import com.mongodb.MongoClientSettings;
import com.mongodb.MongoException;
import com.mongodb.ReadPreference;
import com.mongodb.ServerApi;
import com.mongodb.ServerApiVersion;
import com.mongodb.WriteConcern;
import com.mongodb.client.MongoClient;
import com.mongodb.client.MongoClients;
import com.mongodb.client.MongoCollection;
import com.mongodb.client.MongoDatabase;
import com.mongodb.client.result.DeleteResult;
import com.mongodb.client.result.UpdateResult;
import org.bson.Document;
import org.bson.types.ObjectId;
import org.bson.json.JsonMode;
import org.bson.json.JsonWriterSettings;

import java.nio.charset.StandardCharsets;
import java.time.Duration;
import java.util.*;
import java.util.regex.Pattern;

public class Task112 {

    // Validation constraints
    private static final int MAX_URI_LEN = 2048;
    private static final int MAX_NAME_LEN = 64;
    private static final int MAX_KEYS = 50;
    private static final int MAX_STRING_LEN = 256;
    private static final Pattern NAME_PATTERN = Pattern.compile("^[A-Za-z0-9._-]{1,64}$");
    private static final JsonWriterSettings JSON_SETTINGS =
            JsonWriterSettings.builder().outputMode(JsonMode.RELAXED).build();

    // Validate MongoDB URI basic format and length
    public static boolean validateUri(final String uri) {
        if (uri == null) return false;
        if (uri.length() == 0 || uri.length() > MAX_URI_LEN) return false;
        String trimmed = uri.trim();
        if (!(trimmed.startsWith("mongodb://") || trimmed.startsWith("mongodb+srv://"))) return false;
        // prevent accidental credentials print, but allow presence if provided by caller
        return true;
    }

    // Validate database/collection names
    public static boolean validateName(final String name) {
        if (name == null) return false;
        if (name.length() == 0 || name.length() > MAX_NAME_LEN) return false;
        return NAME_PATTERN.matcher(name).matches();
    }

    // Sanitize and convert Map to BSON Document with restricted types and sizes
    public static Document mapToSafeDocument(final Map<String, Object> data) {
        if (data == null) throw new IllegalArgumentException("data cannot be null");
        if (data.size() > MAX_KEYS) throw new IllegalArgumentException("too many keys in document");
        Document doc = new Document();
        for (Map.Entry<String, Object> e : data.entrySet()) {
            String key = e.getKey();
            if (key == null || key.isEmpty() || key.length() > MAX_NAME_LEN) {
                throw new IllegalArgumentException("invalid key length");
            }
            if (!NAME_PATTERN.matcher(key).matches()) {
                throw new IllegalArgumentException("invalid key format: " + key);
            }
            Object val = sanitizeValue(e.getValue());
            doc.append(key, val);
        }
        return doc;
    }

    private static Object sanitizeValue(Object v) {
        if (v == null) return null;
        if (v instanceof String) {
            String s = (String) v;
            if (s.length() > MAX_STRING_LEN) {
                s = s.substring(0, MAX_STRING_LEN);
            }
            // ensure valid UTF-8 by re-encoding
            byte[] bytes = s.getBytes(StandardCharsets.UTF_8);
            return new String(bytes, StandardCharsets.UTF_8);
        } else if (v instanceof Integer || v instanceof Long || v instanceof Double || v instanceof Boolean) {
            return v;
        } else if (v instanceof Map<?, ?>) {
            @SuppressWarnings("unchecked")
            Map<String, Object> nested = (Map<String, Object>) v;
            return mapToSafeDocument(nested);
        } else if (v instanceof List<?>) {
            List<?> list = (List<?>) v;
            List<Object> safe = new ArrayList<>();
            int count = 0;
            for (Object item : list) {
                if (count++ > MAX_KEYS) break;
                safe.add(sanitizeValue(item));
            }
            return safe;
        } else {
            // Fallback to safe string representation with length limit
            String s = String.valueOf(v);
            if (s.length() > MAX_STRING_LEN) s = s.substring(0, MAX_STRING_LEN);
            return s;
        }
    }

    private static MongoClient createClient(final String uri) {
        if (!validateUri(uri)) throw new IllegalArgumentException("Invalid MongoDB URI");
        ConnectionString cs = new ConnectionString(uri);
        MongoClientSettings settings = MongoClientSettings.builder()
                .applyConnectionString(cs)
                .applyToSocketSettings(builder -> builder.connectTimeout((int) Duration.ofSeconds(5).toMillis(), java.util.concurrent.TimeUnit.MILLISECONDS)
                        .readTimeout((int) Duration.ofSeconds(10).toMillis(), java.util.concurrent.TimeUnit.MILLISECONDS))
                .applyToClusterSettings(builder -> builder.serverSelectionTimeout((int) Duration.ofSeconds(5).toMillis(), java.util.concurrent.TimeUnit.MILLISECONDS))
                .serverApi(ServerApi.builder().version(ServerApiVersion.V1).build())
                .readPreference(ReadPreference.primary())
                .writeConcern(WriteConcern.ACKNOWLEDGED)
                .applicationName("Task112App")
                .build();
        return MongoClients.create(settings);
    }

    // Create operation: returns inserted ObjectId hex string
    public static String createOne(final String uri, final String dbName, final String collName, final Map<String, Object> data) {
        if (!validateName(dbName) || !validateName(collName)) {
            throw new IllegalArgumentException("Invalid database or collection name");
        }
        Document doc = mapToSafeDocument(data);
        try (MongoClient client = createClient(uri)) {
            MongoDatabase db = client.getDatabase(dbName);
            MongoCollection<Document> coll = db.getCollection(collName);
            coll.insertOne(doc);
            ObjectId id = doc.getObjectId("_id");
            return id != null ? id.toHexString() : null;
        }
    }

    // Read by ID: return JSON string or null if not found
    public static String readOneById(final String uri, final String dbName, final String collName, final String idHex) {
        if (!validateName(dbName) || !validateName(collName)) {
            throw new IllegalArgumentException("Invalid database or collection name");
        }
        ObjectId id;
        try {
            id = new ObjectId(idHex);
        } catch (IllegalArgumentException e) {
            throw new IllegalArgumentException("Invalid ObjectId format");
        }
        try (MongoClient client = createClient(uri)) {
            MongoDatabase db = client.getDatabase(dbName);
            MongoCollection<Document> coll = db.getCollection(collName);
            Document found = coll.find(new Document("_id", id)).first();
            if (found == null) return null;
            return found.toJson(JSON_SETTINGS);
        }
    }

    // Read many: return list of JSON strings with a limit
    public static List<String> readMany(final String uri, final String dbName, final String collName, final int limit) {
        if (!validateName(dbName) || !validateName(collName)) {
            throw new IllegalArgumentException("Invalid database or collection name");
        }
        int safeLimit = Math.max(0, Math.min(limit, 100));
        List<String> results = new ArrayList<>();
        try (MongoClient client = createClient(uri)) {
            MongoDatabase db = client.getDatabase(dbName);
            MongoCollection<Document> coll = db.getCollection(collName);
            for (Document d : coll.find().limit(safeLimit)) {
                results.add(d.toJson(JSON_SETTINGS));
            }
        }
        return results;
    }

    // Update by ID: returns true if a document was modified or matched
    public static boolean updateOneById(final String uri, final String dbName, final String collName, final String idHex, final Map<String, Object> updates) {
        if (!validateName(dbName) || !validateName(collName)) {
            throw new IllegalArgumentException("Invalid database or collection name");
        }
        ObjectId id;
        try {
            id = new ObjectId(idHex);
        } catch (IllegalArgumentException e) {
            throw new IllegalArgumentException("Invalid ObjectId format");
        }
        Document setDoc = mapToSafeDocument(updates);
        try (MongoClient client = createClient(uri)) {
            MongoDatabase db = client.getDatabase(dbName);
            MongoCollection<Document> coll = db.getCollection(collName);
            UpdateResult res = coll.updateOne(new Document("_id", id), new Document("$set", setDoc));
            return res.getMatchedCount() > 0;
        }
    }

    // Delete by ID: returns true if a document was deleted
    public static boolean deleteOneById(final String uri, final String dbName, final String collName, final String idHex) {
        if (!validateName(dbName) || !validateName(collName)) {
            throw new IllegalArgumentException("Invalid database or collection name");
        }
        ObjectId id;
        try {
            id = new ObjectId(idHex);
        } catch (IllegalArgumentException e) {
            throw new IllegalArgumentException("Invalid ObjectId format");
        }
        try (MongoClient client = createClient(uri)) {
            MongoDatabase db = client.getDatabase(dbName);
            MongoCollection<Document> coll = db.getCollection(collName);
            DeleteResult res = coll.deleteOne(new Document("_id", id));
            return res.getDeletedCount() > 0;
        }
    }

    public static void main(String[] args) {
        // Five test cases demonstrating CRUD. Adjust URI if needed.
        final String uri = (args != null && args.length > 0 && validateUri(args[0])) ? args[0] : "mongodb://127.0.0.1:27017";
        final String db = "testdb_secure";
        final String coll = "items_secure";

        try {
            // 1) Create
            Map<String, Object> doc = new HashMap<>();
            doc.put("name", "alpha");
            doc.put("count", 1);
            doc.put("active", true);
            String id = createOne(uri, db, coll, doc);
            System.out.println("Create insertedId: " + id);

            // 2) Read by ID
            String json = readOneById(uri, db, coll, id);
            System.out.println("Read by ID: " + json);

            // 3) Update by ID
            Map<String, Object> updates = new HashMap<>();
            updates.put("count", 2);
            updates.put("desc", "updated");
            boolean updated = updateOneById(uri, db, coll, id, updates);
            System.out.println("Update result: " + updated);

            // 4) Read many with limit
            List<String> all = readMany(uri, db, coll, 5);
            System.out.println("Read many (" + all.size() + "): " + all);

            // 5) Delete by ID
            boolean deleted = deleteOneById(uri, db, coll, id);
            System.out.println("Delete result: " + deleted);

            // Extra: Attempt to read non-existing document
            String missing = readOneById(uri, db, coll, "000000000000000000000000");
            System.out.println("Read missing (should be null): " + missing);

        } catch (IllegalArgumentException | MongoException ex) {
            System.out.println("Operation failed securely: " + ex.getMessage());
        }
    }
}