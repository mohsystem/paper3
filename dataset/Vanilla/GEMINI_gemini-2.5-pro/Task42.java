public class Task42 {

    private static java.util.Map<String, String> userDatabase = new java.util.HashMap<>();

    // Hashes a password using SHA-256
    private static String hashPassword(String password) {
        try {
            java.security.MessageDigest digest = java.security.MessageDigest.getInstance("SHA-256");
            byte[] encodedhash = digest.digest(password.getBytes(java.nio.charset.StandardCharsets.UTF_8));
            StringBuilder hexString = new StringBuilder(2 * encodedhash.length);
            for (byte b : encodedhash) {
                String hex = Integer.toHexString(0xff & b);
                if (hex.length() == 1) {
                    hexString.append('0');
                }
                hexString.append(hex);
            }
            return hexString.toString();
        } catch (java.security.NoSuchAlgorithmException e) {
            // This should never happen for a standard algorithm like SHA-256
            throw new RuntimeException("SHA-256 algorithm not found", e);
        }
    }

    // Registers a new user, returning true on success and false on failure
    public static boolean registerUser(String username, String password) {
        if (userDatabase.containsKey(username)) {
            System.out.println("Registration failed: Username '" + username + "' already exists.");
            return false;
        }
        if (username == null || password == null || username.isEmpty() || password.isEmpty()) {
            System.out.println("Registration failed: Username and password cannot be empty.");
            return false;
        }
        String hashedPassword = hashPassword(password);
        userDatabase.put(username, hashedPassword);
        System.out.println("User '" + username + "' registered successfully.");
        return true;
    }

    // Authenticates a user, returning true on success and false on failure
    public static boolean loginUser(String username, String password) {
        if (!userDatabase.containsKey(username) || password == null) {
            return false; // User not found or invalid input
        }
        String storedHash = userDatabase.get(username);
        String enteredHash = hashPassword(password);
        return storedHash.equals(enteredHash);
    }

    public static void main(String[] args) {
        System.out.println("--- Java User Authentication System ---");

        // Test Case 1: Successful registration
        registerUser("alice", "password123");

        // Test Case 2: Attempt to register an existing user
        registerUser("alice", "another_password");

        // Test Case 3: Successful login
        boolean login1 = loginUser("alice", "password123");
        System.out.println("Login attempt for 'alice' with correct password: " + (login1 ? "Success" : "Failed"));

        // Test Case 4: Failed login (wrong password)
        boolean login2 = loginUser("alice", "wrong_password");
        System.out.println("Login attempt for 'alice' with incorrect password: " + (login2 ? "Success" : "Failed"));

        // Test Case 5: Failed login (non-existent user)
        boolean login3 = loginUser("bob", "password123");
        System.out.println("Login attempt for non-existent user 'bob': " + (login3 ? "Success" : "Failed"));
    }
}