import java.nio.charset.StandardCharsets;

public class Task103 {

    /**
     * Checks if a password meets the strength criteria.
     * Criteria:
     * - At least 8 characters long
     * - Contains at least one uppercase letter
     * - Contains at least one lowercase letter
     * - Contains at least one digit
     *
     * @param password The password string to check.
     * @return true if the password is strong, false otherwise.
     */
    public static boolean isPasswordStrong(String password) {
        if (password == null || password.length() < 8) {
            return false;
        }

        boolean hasUpper = false;
        boolean hasLower = false;
        boolean hasDigit = false;

        for (char ch : password.toCharArray()) {
            if (Character.isUpperCase(ch)) {
                hasUpper = true;
            } else if (Character.isLowerCase(ch)) {
                hasLower = true;
            } else if (Character.isDigit(ch)) {
                hasDigit = true;
            }

            // Optimization: if all conditions are met, no need to check further.
            if (hasUpper && hasLower && hasDigit) {
                return true;
            }
        }

        return hasUpper && hasLower && hasDigit;
    }

    public static void main(String[] args) {
        String[] testPasswords = {
            "StrongPwd1",      // Should be true
            "weakpwd1",        // Should be false (missing uppercase)
            "WEAKPWD1",        // Should be false (missing lowercase)
            "WeakPassword",    // Should be false (missing digit)
            "Wpwd1"            // Should be false (too short)
        };

        for (String password : testPasswords) {
            System.out.println("Password: \"" + password + "\" -> Strong: " + isPasswordStrong(password));
        }
    }
}