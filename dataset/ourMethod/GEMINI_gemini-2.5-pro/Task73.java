import java.util.regex.Pattern;

public class Task73 {

    private static final int MIN_LENGTH = 12;
    private static final int MAX_LENGTH = 128;
    // Note: Backslash must be escaped in the string literal
    private static final String SPECIAL_CHARACTERS = "!@#$%^&*()_+-=[]{};':\"\\|,.<>/?~";

    /**
     * Checks if a given password is valid for an admin user based on a strong password policy.
     * Policy:
     * - Length between 12 and 128 characters.
     * - Contains at least one uppercase letter.
     * - Contains at least one lowercase letter.
     * - Contains at least one digit.
     * - Contains at least one special character.
     *
     * @param password The password string to validate.
     * @return true if the password is valid, false otherwise.
     */
    public static boolean isAdminPasswordValid(String password) {
        if (password == null || password.length() < MIN_LENGTH || password.length() > MAX_LENGTH) {
            return false;
        }

        boolean hasUpper = false;
        boolean hasLower = false;
        boolean hasDigit = false;
        boolean hasSpecial = false;

        for (char ch : password.toCharArray()) {
            if (Character.isUpperCase(ch)) {
                hasUpper = true;
            } else if (Character.isLowerCase(ch)) {
                hasLower = true;
            } else if (Character.isDigit(ch)) {
                hasDigit = true;
            } else if (SPECIAL_CHARACTERS.indexOf(ch) >= 0) {
                hasSpecial = true;
            }
            
            // Optimization: if all conditions are met, we can exit early.
            if (hasUpper && hasLower && hasDigit && hasSpecial) {
                return true;
            }
        }

        return hasUpper && hasLower && hasDigit && hasSpecial;
    }

    public static void main(String[] args) {
        String[] testPasswords = {
            "Admin@1234567",         // Valid
            "admin@1234567",         // Invalid: no uppercase
            "Admin1234567",          // Invalid: no special character
            "Admin@",                // Invalid: too short
            "ADMIN@1234567"          // Invalid: no lowercase
        };

        for (String password : testPasswords) {
            System.out.println("Password: \"" + password + "\" is valid: " + isAdminPasswordValid(password));
        }
    }
}