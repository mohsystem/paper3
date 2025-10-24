import java.io.*;
import java.util.Arrays;

// A simple class that needs to be serialized and deserialized.
// It must implement the Serializable interface.
class UserData implements Serializable {
    // A version UID is good practice for serialization
    private static final long serialVersionUID = 1L;
    
    private int id;
    private String name;
    private transient String password; // transient fields are not serialized

    public UserData(int id, String name, String password) {
        this.id = id;
        this.name = name;
        this.password = password;
    }

    @Override
    public String toString() {
        // Password will be null after deserialization because it's transient
        return "UserData{id=" + id + ", name='" + name + "', password='" + password + "'}";
    }
}

public class Task131 {

    /**
     * Serializes a UserData object into a byte array.
     * @param user The UserData object to serialize.
     * @return A byte array representing the serialized object, or null on error.
     */
    public static byte[] serializeData(UserData user) {
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
     * Deserializes a byte array back into a UserData object.
     * @param data The user-supplied byte array.
     * @return The deserialized UserData object, or null on error.
     */
    public static UserData deserializeData(byte[] data) {
        if (data == null) {
            return null;
        }
        try (ByteArrayInputStream bis = new ByteArrayInputStream(data);
             ObjectInputStream in = new ObjectInputStream(bis)) {
            return (UserData) in.readObject();
        } catch (IOException | ClassNotFoundException e) {
            System.err.println("Deserialization failed: " + e.getMessage());
            return null;
        }
    }

    public static void main(String[] args) {
        // Test Cases
        UserData[] users = {
            new UserData(1, "Alice", "pass123"),
            new UserData(2, "Bob", "qwerty"),
            new UserData(3, "Charlie", "secret"),
            new UserData(4, "David", "d@v1d"),
            new UserData(5, "Eve", "evepass")
        };
        
        System.out.println("--- Java Deserialization Test ---");
        int testNum = 1;
        for (UserData originalUser : users) {
            System.out.println("\n--- Test Case " + (testNum++) + " ---");
            System.out.println("Original Object: " + originalUser);
            
            // 1. Serialize the object
            byte[] serializedData = serializeData(originalUser);
            System.out.println("Serialized Data (bytes): " + Arrays.toString(serializedData));

            // 2. Deserialize the data
            UserData deserializedUser = deserializeData(serializedData);

            if (deserializedUser != null) {
                System.out.println("Deserialized Object: " + deserializedUser);
            } else {
                System.out.println("Deserialization resulted in null.");
            }
        }
    }
}