public class Task73 {

    private static final int MIN_PASSWORD_LENGTH = 12;
    private static final String[] COMMON_PASSWORDS = {
        "password", "123456", "123456789", "qwerty", "12345", "12345678", "111111", "1234567", "password123"
    };

    /**
     * Checks if a given password is valid for an admin user based on security policies.
     *
     * @param password The password string to validate.
     * @return true if the password is valid, false otherwise.
     */
    public static boolean isAdminPasswordValid(String password) {
        if (password == null || password.length() < MIN_PASSWORD_LENGTH) {
            return false;
        }

        boolean hasUpper = false;
        boolean hasLower = false;
        boolean hasDigit = false;
        boolean hasSpecial = false;

        for (char c : password.toCharArray()) {
            if (Character.isUpperCase(c)) {
                hasUpper = true;
            } else if (Character.isLowerCase(c)) {
                hasLower = true;
            } else if (Character.isDigit(c)) {
                hasDigit = true;
            } else {
                // A simple check for common special characters
                if ("!@#$%^&*()_+-=[]{};':\"\\|,.<>/?~`".indexOf(c) >= 0) {
                    hasSpecial = true;
                }
            }
        }

        if (!hasUpper || !hasLower || !hasDigit || !hasSpecial) {
            return false;
        }

        // Check against a list of common passwords
        for (String commonPass : COMMON_PASSWORDS) {
            if (password.equals(commonPass)) {
                return false;
            }
        }

        // Check if the password contains "admin" (case-insensitive)
        if (password.toLowerCase().contains("admin")) {
            return false;
        }

        return true;
    }

    public static void main(String[] args) {
        String[] testPasswords = {
            "ValidP@ssw0rd!2024",  // Valid
            "Sh0rt!P@s",           // Invalid: Too short
            "NoDigitPassword!",    // Invalid: Missing digit
            "password123",         // Invalid: Common password
            "MyAdm1nPassword!"     // Invalid: Contains "admin"
        };

        System.out.println("Running Java Test Cases:");
        for (String pwd : testPasswords) {
            System.out.println("Password: \"" + pwd + "\" -> Is Valid: " + isAdminPasswordValid(pwd));
        }
    }
}