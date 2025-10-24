public class Task103 {
    public static boolean isStrongPassword(String password) {
        if (password == null || password.length() < 8) return false;
        boolean hasUpper = false, hasLower = false, hasDigit = false;
        for (int i = 0; i < password.length(); i++) {
            char c = password.charAt(i);
            if (Character.isUpperCase(c)) hasUpper = true;
            else if (Character.isLowerCase(c)) hasLower = true;
            else if (Character.isDigit(c)) hasDigit = true;
        }
        return hasUpper && hasLower && hasDigit;
    }

    public static void main(String[] args) {
        String[] tests = {
            "Password1",
            "password",
            "PASSWORD1",
            "Passw1",
            "StrongPass123"
        };
        for (String t : tests) {
            System.out.println(t + " -> " + isStrongPassword(t));
        }
    }
}