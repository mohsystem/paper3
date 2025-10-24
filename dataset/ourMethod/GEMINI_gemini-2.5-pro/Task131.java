import java.util.Objects;

public class Task131 {

    /**
     * A simple data class to hold user information.
     * Made immutable for safety and simplicity.
     */
    public static final class UserData {
        private final int id;
        private final String name;

        public UserData(int id, String name) {
            // Basic validation in constructor
            if (name == null || name.isEmpty()) {
                throw new IllegalArgumentException("Name cannot be null or empty.");
            }
            this.id = id;
            this.name = name;
        }

        public int getId() {
            return id;
        }

        public String getName() {
            return name;
        }

        @Override
        public String toString() {
            return "UserData{" + "id=" + id + ", name='" + name + '\'' + '}';
        }

        @Override
        public boolean equals(Object o) {
            if (this == o) return true;
            if (o == null || getClass() != o.getClass()) return false;
            UserData userData = (UserData) o;
            return id == userData.id && Objects.equals(name, userData.name);
        }

        @Override
        public int hashCode() {
            return Objects.hash(id, name);
        }
    }

    /**
     * Serializes a UserData object into a simple string format.
     * Format: "id=<integer>;name=<string>"
     * @param user The UserData object to serialize.
     * @return A string representation of the user data.
     */
    public static String serialize(UserData user) {
        if (user == null) {
            return "";
        }
        return "id=" + user.getId() + ";name=" + user.getName();
    }

    /**
     * Deserializes a string into a UserData object.
     * This function uses a simple, custom data-only format to avoid
     * the security risks of Java's native ObjectInputStream.
     *
     * @param data The string to deserialize.
     * @return A new UserData object.
     * @throws IllegalArgumentException if the data format is invalid.
     */
    public static UserData deserialize(String data) {
        if (data == null || data.trim().isEmpty()) {
            throw new IllegalArgumentException("Input data cannot be null or empty.");
        }

        final String idPrefix = "id=";
        final String namePrefix = ";name=";
        final int MAX_NAME_LENGTH = 256;

        int idStartIndex = data.indexOf(idPrefix);
        int nameStartIndex = data.indexOf(namePrefix);

        // Validate format and order of tokens
        if (idStartIndex != 0 || nameStartIndex <= idStartIndex) {
            throw new IllegalArgumentException("Invalid data format: 'id=' and ';name=' markers are missing or in wrong order.");
        }

        try {
            String idStr = data.substring(idPrefix.length(), nameStartIndex);
            int id = Integer.parseInt(idStr);

            String name = data.substring(nameStartIndex + namePrefix.length());
            if (name.isEmpty()) {
                throw new IllegalArgumentException("Name cannot be empty.");
            }
            if (name.length() > MAX_NAME_LENGTH) {
                throw new IllegalArgumentException("Name exceeds maximum length of " + MAX_NAME_LENGTH);
            }

            return new UserData(id, name);
        } catch (NumberFormatException e) {
            throw new IllegalArgumentException("Invalid ID format, must be an integer.", e);
        } catch (IndexOutOfBoundsException e) {
            throw new IllegalArgumentException("Invalid data format, cannot parse components.", e);
        }
    }


    public static void main(String[] args) {
        System.out.println("--- Java Deserialization Tests ---");

        // Test Case 1: Valid data
        UserData user1 = new UserData(101, "Alice");
        String serialized1 = serialize(user1);
        try {
            UserData deserialized1 = deserialize(serialized1);
            System.out.println("Test 1 (Valid): PASSED - " + deserialized1);
            assert user1.equals(deserialized1);
        } catch (Exception e) {
            System.out.println("Test 1 (Valid): FAILED - " + e.getMessage());
        }

        // Test Case 2: Invalid format (wrong order)
        String serialized2 = "name=Bob;id=102";
        try {
            deserialize(serialized2);
            System.out.println("Test 2 (Wrong Order): FAILED - No exception thrown");
        } catch (IllegalArgumentException e) {
            System.out.println("Test 2 (Wrong Order): PASSED - " + e.getMessage());
        }

        // Test Case 3: Invalid ID (not a number)
        String serialized3 = "id=abc;name=Charlie";
        try {
            deserialize(serialized3);
            System.out.println("Test 3 (Invalid ID): FAILED - No exception thrown");
        } catch (IllegalArgumentException e) {
            System.out.println("Test 3 (Invalid ID): PASSED - " + e.getMessage());
        }

        // Test Case 4: Missing part (no name)
        String serialized4 = "id=104";
        try {
            deserialize(serialized4);
            System.out.println("Test 4 (Missing Part): FAILED - No exception thrown");
        } catch (IllegalArgumentException e) {
            System.out.println("Test 4 (Missing Part): PASSED - " + e.getMessage());
        }

        // Test Case 5: Empty input
        String serialized5 = "";
        try {
            deserialize(serialized5);
            System.out.println("Test 5 (Empty Input): FAILED - No exception thrown");
        } catch (IllegalArgumentException e) {
            System.out.println("Test 5 (Empty Input): PASSED - " + e.getMessage());
        }
    }
}