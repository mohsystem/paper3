public class Task103 {

    /**
     * Checks the strength of a given password.
     *
     * @param password The password string to check.
     * @return true if the password is strong, false otherwise.
     */
    public static boolean checkPasswordStrength(String password) {
        // Handle null or empty input
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
        }

        return hasUpper && hasLower && hasDigit;
    }

    public static void main(String[] args) {
        String[] testCases = {
            "StrongPass123", // Strong
            "weak",          // Weak: too short, no upper, no digit
            "NoDigitPass",   // Weak: no digit
            "nouppercase1",  // Weak: no uppercase
            "NOLOWERCASE1"   // Weak: no lowercase
        };

        System.out.println("Java Test Cases:");
        for (String pass : testCases) {
            System.out.println("Password: \"" + pass + "\" -> Strong: " + checkPasswordStrength(pass));
        }
    }
}