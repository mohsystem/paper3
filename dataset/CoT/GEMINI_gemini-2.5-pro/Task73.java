public class Task73 {

    /**
     * Checks if a given password is valid for an admin user.
     * A valid admin password must:
     * 1. Be at least 12 characters long.
     * 2. Contain at least one uppercase letter.
     * 3. Contain at least one lowercase letter.
     * 4. Contain at least one digit.
     * 5. Contain at least one special character from the set: !@#$%^&*()-_=+[]{}|;:'",.<>/?
     *
     * @param password The password string to validate.
     * @return true if the password is valid, false otherwise.
     */
    public static boolean isValidAdminPassword(String password) {
        // 1. Rule: Check for null and minimum length (at least 12 characters)
        if (password == null || password.length() < 12) {
            return false;
        }

        boolean hasUpper = false;
        boolean hasLower = false;
        boolean hasDigit = false;
        boolean hasSpecial = false;

        String specialChars = "!@#$%^&*()-_=+[]{}|;:'\",.<>/?";

        for (char c : password.toCharArray()) {
            if (Character.isUpperCase(c)) {
                hasUpper = true;
            } else if (Character.isLowerCase(c)) {
                hasLower = true;
            } else if (Character.isDigit(c)) {
                hasDigit = true;
            } else if (specialChars.indexOf(c) != -1) {
                hasSpecial = true;
            }
        }

        // Check if all complexity rules are met
        return hasUpper && hasLower && hasDigit && hasSpecial;
    }

    public static void main(String[] args) {
        String[] testPasswords = {
            "AdminP@ssw0rd123",  // Valid
            "short",              // Invalid: too short
            "NoDigitPassword!",   // Invalid: no digit
            "nouppercase@123",    // Invalid: no uppercase
            "NoSpecialChar123",   // Invalid: no special character
        };

        System.out.println("Running Java Test Cases:");
        for (String pass : testPasswords) {
            System.out.println("Password: \"" + pass + "\" is valid: " + isValidAdminPassword(pass));
        }
    }
}