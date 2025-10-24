
import com.google.gson.Gson;
import com.google.gson.JsonSyntaxException;
import java.util.HashMap;
import java.util.Map;

class Task131 {
    static class UserData {
        private String name;
        private int age;
        private String email;
        
        public UserData() {}
        
        public UserData(String name, int age, String email) {
            this.name = name;
            this.age = age;
            this.email = email;
        }
        
        public String getName() { return name; }
        public int getAge() { return age; }
        public String getEmail() { return email; }
        
        @Override
        public String toString() {
            return "UserData{name='" + name + "', age=" + age + ", email='" + email + "'}";
        }
    }
    
    public static UserData deserializeUserData(String jsonData) {
        if (jsonData == null || jsonData.trim().isEmpty()) {
            throw new IllegalArgumentException("Input data cannot be null or empty");
        }
        
        try {
            Gson gson = new Gson();
            UserData userData = gson.fromJson(jsonData, UserData.class);
            
            // Validate deserialized data
            if (userData.getName() == null || userData.getName().isEmpty()) {
                throw new IllegalArgumentException("Name cannot be null or empty");
            }
            if (userData.getAge() < 0 || userData.getAge() > 150) {
                throw new IllegalArgumentException("Age must be between 0 and 150");
            }
            if (userData.getEmail() == null || !userData.getEmail().matches("^[A-Za-z0-9+_.-]+@(.+)$")) {
                throw new IllegalArgumentException("Invalid email format");
            }
            
            return userData;
        } catch (JsonSyntaxException e) {
            throw new IllegalArgumentException("Invalid JSON format: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        System.out.println("=== Secure Deserialization Test Cases ===\\n");
        
        // Test Case 1: Valid data
        try {
            String json1 = "{\\"name\\":\\"John Doe\\",\\"age\\":30,\\"email\\":\\"john@example.com\\"}";
            UserData user1 = deserializeUserData(json1);
            System.out.println("Test 1 - Valid data: " + user1);
        } catch (Exception e) {
            System.out.println("Test 1 failed: " + e.getMessage());
        }
        
        // Test Case 2: Valid data with different values
        try {
            String json2 = "{\\"name\\":\\"Jane Smith\\",\\"age\\":25,\\"email\\":\\"jane@test.com\\"}";
            UserData user2 = deserializeUserData(json2);
            System.out.println("Test 2 - Valid data: " + user2);
        } catch (Exception e) {
            System.out.println("Test 2 failed: " + e.getMessage());
        }
        
        // Test Case 3: Invalid age (negative)
        try {
            String json3 = "{\\"name\\":\\"Bob\\",\\"age\\":-5,\\"email\\":\\"bob@example.com\\"}";
            UserData user3 = deserializeUserData(json3);
            System.out.println("Test 3 - Should fail: " + user3);
        } catch (Exception e) {
            System.out.println("Test 3 - Expected failure: " + e.getMessage());
        }
        
        // Test Case 4: Invalid email format
        try {
            String json4 = "{\\"name\\":\\"Alice\\",\\"age\\":28,\\"email\\":\\"invalid-email\\"}";
            UserData user4 = deserializeUserData(json4);
            System.out.println("Test 4 - Should fail: " + user4);
        } catch (Exception e) {
            System.out.println("Test 4 - Expected failure: " + e.getMessage());
        }
        
        // Test Case 5: Invalid JSON syntax
        try {
            String json5 = "{\\"name\\":\\"Charlie\\",\\"age\\":35,\\"email\\":\\"charlie@test.com\\"";
            UserData user5 = deserializeUserData(json5);
            System.out.println("Test 5 - Should fail: " + user5);
        } catch (Exception e) {
            System.out.println("Test 5 - Expected failure: " + e.getMessage());
        }
    }
}
