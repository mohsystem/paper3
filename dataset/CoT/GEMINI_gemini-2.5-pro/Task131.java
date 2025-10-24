import java.io.*;
import java.util.Arrays;
import java.util.List;
import java.util.Objects;

/**
 * Main class to demonstrate secure deserialization.
 * In Java, standard deserialization via ObjectInputStream is highly insecure
 * when used with untrusted data, as it can lead to Remote Code Execution (RCE).
 * The secure approach demonstrated here uses a custom ObjectInputStream to
 * whitelist allowed classes. A more common and interoperable solution is to use
 * a safe, data-only format like JSON with a library like Jackson or Gson.
 */
public class Task131 {

    /**
     * A simple, serializable data class.
     */
    public static class UserData implements Serializable {
        private static final long serialVersionUID = 20240314L; // Best practice for Serializable classes
        private String name;
        private int id;
        private boolean isActive;

        // Default constructor needed for some frameworks, though not strictly for this example.
        public UserData() {}

        public UserData(String name, int id, boolean isActive) {
            this.name = name;
            this.id = id;
            this.isActive = isActive;
        }

        @Override
        public String toString() {
            return "UserData{" +
                   "name='" + name + '\'' +
                   ", id=" + id +
                   ", isActive=" + isActive +
                   '}';
        }

        @Override
        public boolean equals(Object o) {
            if (this == o) return true;
            if (o == null || getClass() != o.getClass()) return false;
            UserData userData = (UserData) o;
            return id == userData.id && isActive == userData.isActive && Objects.equals(name, userData.name);
        }
    }

    /**
     * A secure ObjectInputStream that whitelists allowed classes to prevent
     * deserialization of arbitrary, potentially malicious, "gadget" classes.
     */
    public static class SecureObjectInputStream extends ObjectInputStream {
        private static final List<String> WHITELIST = Arrays.asList(
            "Task131$UserData" // The fully qualified name of the nested UserData class
        );

        public SecureObjectInputStream(InputStream in) throws IOException {
            super(in);
        }

        @Override
        protected Class<?> resolveClass(ObjectStreamClass desc) throws IOException, ClassNotFoundException {
            // SECURITY: Check the class name against a whitelist before deserializing.
            if (!WHITELIST.contains(desc.getName())) {
                throw new InvalidClassException("Unauthorized deserialization attempt", desc.getName());
            }
            return super.resolveClass(desc);
        }
    }

    /**
     * Serializes a UserData object into a byte array.
     * @param user The UserData object to serialize.
     * @return A byte array representing the serialized object.
     */
    public static byte[] serialize(UserData user) {
        try (ByteArrayOutputStream bos = new ByteArrayOutputStream();
             ObjectOutputStream out = new ObjectOutputStream(bos)) {
            out.writeObject(user);
            return bos.toByteArray();
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
    }

    /**
     * Deserializes user-supplied data securely using the whitelisting input stream.
     * @param data The byte array of serialized data.
     * @return The deserialized UserData object, or null on failure.
     */
    public static UserData deserializeSecurely(byte[] data) {
        if (data == null) {
            return null;
        }
        try (ByteArrayInputStream bis = new ByteArrayInputStream(data);
             ObjectInputStream in = new SecureObjectInputStream(bis)) { // Using the secure stream
            Object obj = in.readObject();
            return (UserData) obj;
        } catch (IOException | ClassNotFoundException | ClassCastException e) {
            // This will catch the InvalidClassException for non-whitelisted classes
            System.err.println("Deserialization failed: " + e.getMessage());
            return null;
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Java Deserialization Demo ---");

        UserData[] testCases = {
            new UserData("Alice", 101, true),
            new UserData("Bob", 202, false),
            new UserData("Charlie", 303, true),
            new UserData("David", 404, false),
            new UserData("", 0, false)
        };
        
        int testNum = 1;
        for (UserData originalUser : testCases) {
            System.out.println("\n--- Test Case " + testNum++ + " ---");
            System.out.println("Original:    " + originalUser);

            byte[] serializedData = serialize(originalUser);

            UserData deserializedUser = deserializeSecurely(serializedData);

            if (deserializedUser != null) {
                System.out.println("Deserialized: " + deserializedUser);
                System.out.println("Objects are equal: " + originalUser.equals(deserializedUser));
            } else {
                System.out.println("Deserialization failed.");
            }
        }
        
        System.out.println("\n--- Security Test Case ---");
        System.out.println("Attempting to deserialize a non-whitelisted class (java.util.HashMap)...");
        try {
            ByteArrayOutputStream bos = new ByteArrayOutputStream();
            ObjectOutputStream out = new ObjectOutputStream(bos);
            out.writeObject(new java.util.HashMap<>());
            byte[] maliciousData = bos.toByteArray();
            
            UserData result = deserializeSecurely(maliciousData);
            if (result == null) {
                System.out.println("SUCCESS: Deserialization of unauthorized class was blocked as expected.");
            } else {
                 System.out.println("FAILURE: Deserialization of unauthorized class was not blocked.");
            }
        } catch (IOException e) {
             System.err.println("Error creating malicious payload for test: " + e.getMessage());
        }
    }
}