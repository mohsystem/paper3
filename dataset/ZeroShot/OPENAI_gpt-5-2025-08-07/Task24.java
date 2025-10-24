public class Task24 {
    public static boolean validatePin(String s) {
        if (s == null) return false;
        int len = s.length();
        if (len != 4 && len != 6) return false;
        for (int i = 0; i < len; i++) {
            char c = s.charAt(i);
            if (c < '0' || c > '9') return false;
        }
        return true;
    }

    public static void main(String[] args) {
        String[] tests = {
            "1234",
            "12345",
            "a234",
            "0000",
            "098765"
        };
        for (String t : tests) {
            System.out.println(t + " -> " + validatePin(t));
        }
    }
}