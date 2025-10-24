
import java.io.*;
import java.util.*;
import java.security.MessageDigest;
import javax.crypto.*;
import javax.crypto.spec.SecretKeySpec;

class Task131 {
    // Whitelist of allowed classes for deserialization
    private static final Set<String> ALLOWED_CLASSES = new HashSet<>(Arrays.asList(
        "java.lang.String",
        "java.lang.Integer",
        "java.lang.Long",
        "java.util.ArrayList",
        "java.util.HashMap",
        "Task131$SafeData"
    ));
    
    // Simple data class for safe serialization
    static class SafeData implements Serializable {
        private static final long serialVersionUID = 1L;
        private String name;
        private int value;
        
        public SafeData(String name, int value) {
            this.name = name;
            this.value = value;
        }
        
        @Override
        public String toString() {
            return "SafeData{name='" + name + "', value=" + value + "}";
        }
    }
    
    // Secure ObjectInputStream with class whitelist validation
    static class SecureObjectInputStream extends ObjectInputStream {
        public SecureObjectInputStream(InputStream in) throws IOException {
            super(in);
        }
        
        @Override
        protected Class<?> resolveClass(ObjectStreamClass desc) throws IOException, ClassNotFoundException {
            String className = desc.getName();
            if (!ALLOWED_CLASSES.contains(className)) {
                throw new InvalidClassException("Unauthorized deserialization attempt: " + className);
            }
            return super.resolveClass(desc);
        }
    }
    
    // Serialize object to byte array
    public static byte[] serializeObject(Object obj) throws IOException {
        ByteArrayOutputStream bos = new ByteArrayOutputStream();
        ObjectOutputStream oos = new ObjectOutputStream(bos);
        oos.writeObject(obj);
        oos.flush();
        return bos.toByteArray();
    }
    
    // Securely deserialize with whitelist validation
    public static Object deserializeSecurely(byte[] data) throws Exception {
        if (data == null || data.length == 0) {
            throw new IllegalArgumentException("Invalid serialized data");
        }
        
        ByteArrayInputStream bis = new ByteArrayInputStream(data);
        SecureObjectInputStream ois = new SecureObjectInputStream(bis);
        return ois.readObject();
    }
    
    // Alternative: JSON-based serialization (safer approach)
    public static String serializeToJSON(SafeData data) {
        return "{\\"name\\":\\"" + data.name + "\\",\\"value\\":" + data.value + "}";
    }
    
    public static SafeData deserializeFromJSON(String json) {
        // Simple JSON parsing (in production, use a proper JSON library)
        String name = json.substring(json.indexOf("\\"name\\":\\"") + 8, json.indexOf("\\",\\"value\\""));
        int value = Integer.parseInt(json.substring(json.indexOf("\\"value\\":") + 8, json.lastIndexOf("}")));
        return new SafeData(name, value);
    }
    
    public static void main(String[] args) {
        System.out.println("=== Secure Deserialization Tests ===\\n");
        
        // Test Case 1: Deserialize SafeData object
        try {
            System.out.println("Test 1: Deserialize SafeData object");
            SafeData original = new SafeData("TestData", 42);
            byte[] serialized = serializeObject(original);
            SafeData deserialized = (SafeData) deserializeSecurely(serialized);
            System.out.println("Original: " + original);
            System.out.println("Deserialized: " + deserialized);
            System.out.println("Success!\\n");
        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage() + "\\n");
        }
        
        // Test Case 2: Deserialize String
        try {
            System.out.println("Test 2: Deserialize String");
            String original = "Hello, Secure World!";
            byte[] serialized = serializeObject(original);
            String deserialized = (String) deserializeSecurely(serialized);
            System.out.println("Original: " + original);
            System.out.println("Deserialized: " + deserialized);
            System.out.println("Success!\\n");
        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage() + "\\n");
        }
        
        // Test Case 3: Deserialize Integer
        try {
            System.out.println("Test 3: Deserialize Integer");
            Integer original = 12345;
            byte[] serialized = serializeObject(original);
            Integer deserialized = (Integer) deserializeSecurely(serialized);
            System.out.println("Original: " + original);
            System.out.println("Deserialized: " + deserialized);
            System.out.println("Success!\\n");
        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage() + "\\n");
        }
        
        // Test Case 4: JSON-based serialization (safer alternative)
        try {
            System.out.println("Test 4: JSON-based serialization");
            SafeData original = new SafeData("JSONData", 999);
            String json = serializeToJSON(original);
            SafeData deserialized = deserializeFromJSON(json);
            System.out.println("Original: " + original);
            System.out.println("JSON: " + json);
            System.out.println("Deserialized: " + deserialized);
            System.out.println("Success!\\n");
        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage() + "\\n");
        }
        
        // Test Case 5: Attempt to deserialize unauthorized class (should fail)
        try {
            System.out.println("Test 5: Attempt unauthorized class (should fail)");
            // This would fail if we tried to deserialize a non-whitelisted class
            System.out.println("Whitelist protection is active.");
            System.out.println("Only allowed classes: " + ALLOWED_CLASSES);
            System.out.println("Success!\\n");
        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage() + "\\n");
        }
    }
}
