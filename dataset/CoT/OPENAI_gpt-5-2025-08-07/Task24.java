public class Task24 {
    // Validates a PIN: must be exactly 4 or 6 ASCII digits.
    public static boolean isValidPIN(String pin) {
        if (pin == null) return false;
        int len = pin.length();
        if (len != 4 && len != 6) return false;
        for (int i = 0; i < len; i++) {
            char ch = pin.charAt(i);
            if (ch < '0' || ch > '9') return false; // ASCII digit check only
        }
        return true;
    }

    public static void main(String[] args) {
        String[] tests = {"1234", "12345", "a234", "098765", ""};
        for (String t : tests) {
            System.out.println(t + " -> " + (isValidPIN(t) ? "true" : "false"));
        }
    }
}