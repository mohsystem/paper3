public class Lse39 {

    /**
     * Checks if the provided username and password are correct.
     * @param username The username to check.
     * @param password The password to check.
     * @return true if credentials are correct, false otherwise.
     */
    public static boolean login(String username, String password) {
        // Hardcoded correct credentials
        String correctUsername = "admin";
        String correctPassword = "password123";

        // Check if both username and password match
        return correctUsername.equals(username) && correctPassword.equals(password);
    }

    public static void main(String[] args) {
        // 5 test cases
        String[][] testCases = {
            {"admin", "password123"},    // 1. Correct credentials
            {"admin", "wrongpassword"},  // 2. Correct username, wrong password
            {"user", "password123"},     // 3. Wrong username, correct password
            {"user", "wrongpassword"},   // 4. Both wrong
            {"", ""}                     // 5. Empty credentials
        };

        for (String[] testCase : testCases) {
            String username = testCase[0];
            String password = testCase[1];

            System.out.println("Testing with Username: '" + username + "', Password: '" + password + "'");

            if (login(username, password)) {
                System.out.println("Login successful. Redirecting to session page.");
            } else {
                System.out.println("Incorrect username or password.");
            }
            System.out.println("--------------------");
        }
    }
}