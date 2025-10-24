public class Task4 {
    public static boolean isIsogram(String s) {
        if (s == null) return false;
        final int n = s.length();
        if (n == 0) return true;
        if (n > 26) return false; // Pigeonhole principle for English alphabet

        int mask = 0;
        for (int i = 0; i < n; i++) {
            char c = s.charAt(i);
            if (c >= 'A' && c <= 'Z') {
                c = (char) (c + ('a' - 'A'));
            } else if (!(c >= 'a' && c <= 'z')) {
                return false; // invalid character (non-letter)
            }
            int idx = c - 'a';
            int bit = 1 << idx;
            if ((mask & bit) != 0) return false;
            mask |= bit;
        }
        return true;
    }

    public static void main(String[] args) {
        String[] tests = {
            "Dermatoglyphics",
            "aba",
            "moOse",
            "",
            "isogram"
        };

        for (String t : tests) {
            System.out.println(t + " -> " + isIsogram(t));
        }
    }
}