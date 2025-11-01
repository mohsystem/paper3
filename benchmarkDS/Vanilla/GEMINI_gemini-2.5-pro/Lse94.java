public class Lse94 {

    // Simulate a database table: username -> {salt, hashedPassword}
    private static final java.util.Map<String, UserData> database = new java.util.HashMap<>();

    // Inner class to hold user data
    private static class UserData {
        private final byte[] salt;
        private final String hashedPassword;

        public UserData(byte[] salt, String hashedPassword) {
            this.salt = salt;
            this.hashedPassword = hashedPassword;
        }

        public String getHashedPassword() {
            return this.hashedPassword;
        }
    }

    // Helper to convert byte array to hex string
    private static String bytesToHex(byte[] hash) {
        StringBuilder hexString = new StringBuilder(2 * hash.length);
        for (byte b : hash) {
            String hex = Integer.toHexString(0xff & b);
            if (hex.length() == 1) {
                hexString.append('0');
            }
            hexString.append(hex);
        }
        return hexString.toString();
    }

    /**
     * Generates a salt, hashes the password, and "inserts" into the database.
     * @param username The username to register.
     * @param password The plain-text password.
     * @return The UserData object that was stored, or null on failure.
     */
    public static UserData registerUser(String username, String password) {
        try {
            // 1. Generate a random salt
            java.security.SecureRandom random = new java.security.SecureRandom();
            byte[] salt = new byte[16];
            random.nextBytes(salt);

            // 2. Hash the password with the salt using SHA-256
            java.security.MessageDigest md = java.security.MessageDigest.getInstance("SHA-256");
            
            // Combine salt and password for hashing
            byte[] passwordBytes = password.getBytes(java.nio.charset.StandardCharsets.UTF_8);
            byte[] combined = new byte[salt.length + passwordBytes.length];
            System.arraycopy(salt, 0, combined, 0, salt.length);
            System.arraycopy(passwordBytes, 0, combined, salt.length, passwordBytes.length);
            
            byte[] hashedPasswordBytes = md.digest(combined);
            String hashedPasswordHex = bytesToHex(hashedPasswordBytes);

            // 3. Store username, salt, and hashed password
            UserData newUser = new UserData(salt, hashedPasswordHex);
            database.put(username, newUser);

            System.out.println("User '" + username + "' registered successfully.");
            System.out.println("  - Salt (Hex): " + bytesToHex(salt));
            System.out.println("  - Hashed Password (Hex): " + hashedPasswordHex);
            System.out.println("-------------------------------------------------");
            
            return newUser;

        } catch (java.security.NoSuchAlgorithmException e) {
            System.err.println("Error: SHA-256 algorithm not found.");
            e.printStackTrace();
            return null;
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Running 5 Test Cases for User Registration ---\n");
        
        // Test Case 1
        registerUser("alice", "password123");

        // Test Case 2
        registerUser("bob", "my$ecretP@ss");

        // Test Case 3
        registerUser("charlie", "CharlieBrown");

        // Test Case 4
        registerUser("dave", "davespassword");

        // Test Case 5: Registering a user with the same password as alice
        registerUser("eve", "password123");

        System.out.println("\n--- Final Database State ---");
        database.forEach((username, data) -> 
            System.out.println("Username: " + username + ", HashedPassword: " + data.getHashedPassword())
        );
    }
}