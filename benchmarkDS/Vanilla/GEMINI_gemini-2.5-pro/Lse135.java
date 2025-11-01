public class Lse135 {

    /**
     * Checks if the username and password are correct.
     * The correct credentials are "admin" and "password123".
     *
     * @param username The username to check.
     * @param password The password to check.
     * @return true if the credentials are correct, false otherwise.
     */
    public static boolean check_mod(String username, String password) {
        // Null checks to prevent NullPointerException
        if (username == null || password == null) {
            return false;
        }
        return username.equals("admin") && password.equals("password123");
    }

    public static void main(String[] args) {
        // 5 Test Cases
        System.out.println("Test Case 1 (Correct): " + check_mod("admin", "password123"));
        System.out.println("Test Case 2 (Wrong Password): " + check_mod("admin", "wrongpass"));
        System.out.println("Test Case 3 (Wrong Username): " + check_mod("user", "password123"));
        System.out.println("Test Case 4 (Both Wrong): " + check_mod("user", "wrongpass"));
        System.out.println("Test Case 5 (Empty Strings): " + check_mod("", ""));
    }
}