public class Lse91 {

    // In-memory "database" to store user credentials.
    // In a real application, this would be a persistent database (e.g., SQL, NoSQL).
    private static final java.util.Map<String, String> userDatabase = new java.util.HashMap<>();

    /**
     * Hashes a password using the SHA-256 algorithm.
     *
     * @param password The plain-text password to hash.
     * @return The SHA-256 hash of the password as a hex string.
     */
    public static String hashPassword(String password) {
        try {
            java.security.MessageDigest md = java.security.MessageDigest.getInstance("SHA-256");
            byte[] hash = md.digest(password.getBytes(java.nio.charset.StandardCharsets.UTF_8));
            java.math.BigInteger number = new java.math.BigInteger(1, hash);
            StringBuilder hexString = new StringBuilder(number.toString(16));
            // Pad with leading zeros to ensure 64 characters (256 bits)
            while (hexString.length() < 64) {
                hexString.insert(0, '0');
            }
            return hexString.toString();
        } catch (java.security.NoSuchAlgorithmException e) {
            // This should not happen for SHA-256
            throw new RuntimeException(e);
        }
    }

    /**
     * Registers a new user by hashing their password and storing it.
     *
     * @param username The username for the new account.
     * @param password The plain-text password for the new account.
     * @return true if registration is successful, false otherwise (e.g., user exists).
     */
    public static boolean registerUser(String username, String password) {
        if (userDatabase.containsKey(username)) {
            System.out.println("Registration failed: Username '" + username + "' already exists.");
            return false;
        }
        String hashedPassword = hashPassword(password);
        userDatabase.put(username, hashedPassword);
        System.out.println("User '" + username + "' registered successfully.");
        return true;
    }

    public static void main(String[] args) {
        System.out.println("--- Java User Registration System ---");
        
        // Test Case 1: Successful registration
        registerUser("alice", "password123");
        
        // Test Case 2: Another successful registration
        registerUser("bob", "bob_secure_pass");
        
        // Test Case 3: A third successful registration
        registerUser("charlie", "qwerty");
        
        // Test Case 4: Attempt to register a duplicate username
        registerUser("alice", "another_password");
        
        // Test Case 5: A final successful registration
        registerUser("david", "d@v1d_pa$$");
        
        System.out.println("\n--- Current Database ---");
        for (java.util.Map.Entry<String, String> entry : userDatabase.entrySet()) {
            System.out.println("Username: " + entry.getKey() + ", Hashed Password: " + entry.getValue());
        }
    }
}