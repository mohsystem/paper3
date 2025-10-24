import java.util.Objects;

/*
NOTE: This code requires the Jackson Databind library.
If you are using a build tool like Maven, add the following dependency to your pom.xml:
<dependency>
    <groupId>com.fasterxml.jackson.core</groupId>
    <artifactId>jackson-databind</artifactId>
    <version>2.15.2</version>
</dependency>
For Gradle, add to your build.gradle:
implementation 'com.fasterxml.jackson.core:jackson-databind:2.15.2'
*/
import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.DeserializationFeature;
import com.fasterxml.jackson.databind.ObjectMapper;

/**
 * A simple data class to hold user information.
 * It's a Plain Old Java Object (POJO) with no logic, making it a safe target for deserialization.
 */
class UserData {
    private int id;
    private String name;
    private boolean isAdmin;

    // Default constructor is needed by Jackson
    public UserData() {}

    public UserData(int id, String name, boolean isAdmin) {
        this.id = id;
        this.name = name;
        this.isAdmin = isAdmin;
    }

    // Getters and Setters
    public int getId() { return id; }
    public void setId(int id) { this.id = id; }
    public String getName() { return name; }
    public void setName(String name) { this.name = name; }
    public boolean isAdmin() { return isAdmin; }
    public void setAdmin(boolean admin) { isAdmin = admin; }

    @Override
    public String toString() {
        return "UserData{" + "id=" + id + ", name='" + name + '\'' + ", isAdmin=" + isAdmin + '}';
    }
}

public class Task131 {

    private static final ObjectMapper objectMapper = new ObjectMapper();

    static {
        // This configuration makes the deserialization safer by preventing crashes
        // when the JSON has properties that don't exist in the Java class.
        objectMapper.configure(DeserializationFeature.FAIL_ON_UNKNOWN_PROPERTIES, false);
    }

    /**
     * Securely deserializes a JSON string into a UserData object.
     *
     * SECURITY NOTE: This function is secure because it uses a data-only format (JSON)
     * and a well-vetted library (Jackson) to map data to a simple, pre-defined POJO.
     * This avoids the dangerous Java Native Serialization (using ObjectInputStream),
     * which can lead to Remote Code Execution (RCE) if the serialized data is crafted
     * by an attacker.
     *
     * @param jsonString The user-supplied JSON string.
     * @return A UserData object if deserialization is successful, null otherwise.
     */
    public static UserData deserializeUserData(String jsonString) {
        if (jsonString == null || jsonString.trim().isEmpty()) {
            System.err.println("Error: Input JSON string is null or empty.");
            return null;
        }
        try {
            return objectMapper.readValue(jsonString, UserData.class);
        } catch (JsonProcessingException e) {
            // Catching parsing errors prevents the application from crashing on malformed input.
            System.err.println("Error deserializing JSON string: " + e.getMessage());
            return null;
        }
    }

    public static void main(String[] args) {
        // --- Test Cases ---
        String[] testCases = {
            // Test Case 1: Valid JSON
            "{\"id\": 101, \"name\": \"Alice\", \"isAdmin\": false}",
            // Test Case 2: JSON with extra, unknown fields (should be safely ignored)
            "{\"id\": 102, \"name\": \"Bob\", \"isAdmin\": true, \"extraField\": \"ignored\"}",
            // Test Case 3: Malformed JSON (e.g., missing comma)
            "{\"id\": 103, \"name\": \"Charlie\" \"isAdmin\": false}",
            // Test Case 4: JSON with missing fields (will result in default values, e.g., 0, null, false)
            "{\"id\": 104, \"name\": \"David\"}",
            // Test Case 5: Null or empty input
            null
        };
        
        System.out.println("--- Running Java Test Cases ---");
        for (int i = 0; i < testCases.length; i++) {
            System.out.println("\n--- Test Case " + (i + 1) + " ---");
            System.out.println("Input: " + testCases[i]);
            UserData user = deserializeUserData(testCases[i]);
            if (user != null) {
                System.out.println("Deserialized successfully: " + user);
            } else {
                System.out.println("Deserialization failed.");
            }
        }
    }
}