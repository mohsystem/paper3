public class Task24 {
    public static boolean validatePin(String pin) {
        if (pin == null) return false;
        int n = pin.length();
        if (n != 4 && n != 6) return false;
        for (int i = 0; i < n; i++) {
            char c = pin.charAt(i);
            if (c < '0' || c > '9') return false;
        }
        return true;
    }

    public static void main(String[] args) {
        String[] tests = {"1234", "12345", "a234", "0000", "098765"};
        for (String t : tests) {
            System.out.println(t + " -> " + validatePin(t));
        }
    }
}