public class Task103 {

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

            if (hasUpper && hasLower && hasDigit) {
                return true;
            }
        }

        return false;
    }

    public static void main(String[] args) {
        String[] testPasswords = {
            "StrongPassword123",
            "weak",
            "NoDigitsHere",
            "nouppercase123",
            "NOLOWERCASE123"
        };

        for (String password : testPasswords) {
            boolean isStrong = checkPasswordStrength(password);
            System.out.println("Password: \"" + password + "\" -> " + (isStrong ? "Strong" : "Weak"));
        }
    }
}