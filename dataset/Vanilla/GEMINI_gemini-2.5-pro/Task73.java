class Task73 {
    /**
     * Checks if a given password is valid for an admin user.
     * A valid admin password must:
     * 1. Be at least 10 characters long.
     * 2. Contain at least one uppercase letter.
     * 3. Contain at least one lowercase letter.
     * 4. Contain at least one digit.
     * 5. Contain at least one special character from the set !@#$%^&*()-_=+[]{}|;:'",.<>/?
     * 6. Contain the substring "admin" (case-insensitive).
     *
     * @param password The password string to validate.
     * @return true if the password is valid, false otherwise.
     */
    public static boolean isValidAdminPassword(String password) {
        if (password == null || password.length() < 10) {
            return false;
        }

        boolean hasUppercase = false;
        boolean hasLowercase = false;
        boolean hasDigit = false;
        boolean hasSpecialChar = false;
        String specialChars = "!@#$%^&*()-_=+[]{}|;:'\",.<>/?";

        for (char c : password.toCharArray()) {
            if (Character.isUpperCase(c)) {
                hasUppercase = true;
            } else if (Character.isLowerCase(c)) {
                hasLowercase = true;
            } else if (Character.isDigit(c)) {
                hasDigit = true;
            } else if (specialChars.indexOf(c) >= 0) {
                hasSpecialChar = true;
            }
        }

        boolean hasAdminSubstring = password.toLowerCase().contains("admin");

        return hasUppercase && hasLowercase && hasDigit && hasSpecialChar && hasAdminSubstring;
    }

    public static void main(String[] args) {
        String[] testPasswords = {
            "ValidAdminPass123!",   // Should be true
            "shortAd1!",              // Should be false (too short)
            "nouppercaseadmin1!",     // Should be false (no uppercase)
            "NoSpecialAdmin123",      // Should be false (no special character)
            "NoKeyword123!@#",        // Should be false (no "admin" substring)
            "NoDigitsADMIN!@#"        // Should be false (no digits)
        };

        System.out.println("Running Java Test Cases:");
        for (String pass : testPasswords) {
            System.out.println("Password: \"" + pass + "\" is valid? " + isValidAdminPassword(pass));
        }
    }
}