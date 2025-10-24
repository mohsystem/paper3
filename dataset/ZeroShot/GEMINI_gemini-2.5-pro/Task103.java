public class Task103 {

    /**
     * Checks the strength of a given password.
     * A strong password must be at least 8 characters long, contain both
     * uppercase and lowercase letters, and at least one digit.
     *
     * @param password The password string to validate.
     * @return true if the password is strong, false otherwise.
     */
    public static boolean checkPasswordStrength(String password) {
        if (password == null || password.length() < 8) {
            return false;
        }

        boolean hasUpper = false;
        boolean hasLower = false;
        boolean hasDigit = false;

        for (char c : password.toCharArray()) {
            if (Character.isUpperCase(c)) {
                hasUpper = true;
            } else if (Character.isLowerCase(c)) {
                hasLower = true;
            } else if (Character.isDigit(c)) {
                hasDigit = true;
            }
        }

        return hasUpper && hasLower && hasDigit;
    }

    public static void main(String[] args) {
        String[] testCases = {
            "Password123",    // Strong
            "password123",    // Weak (no uppercase)
            "PASSWORD123",    // Weak (no lowercase)
            "Password",       // Weak (no digit)
            "pass"            // Weak (too short)
        };

        System.out.println("--- Java Test Cases ---");
        for (String password : testCases) {
            boolean isStrong = checkPasswordStrength(password);
            System.out.println("Password: \"" + password + "\" -> Strong: " + isStrong);
        }
    }
}