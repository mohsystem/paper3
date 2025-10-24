import java.util.*;

public class Task4 {

    public static boolean isIsogram(String s) {
        if (s == null) {
            return false; // fail closed for null input
        }
        boolean[] seen = new boolean[26];
        for (int i = 0; i < s.length(); i++) {
            char ch = s.charAt(i);
            if (ch >= 'A' && ch <= 'Z') {
                ch = (char) (ch + ('a' - 'A'));
            } else if (!(ch >= 'a' && ch <= 'z')) {
                return false; // invalid character, fail closed
            }
            int idx = ch - 'a';
            if (seen[idx]) {
                return false;
            }
            seen[idx] = true;
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
            System.out.println("\"" + t + "\" -> " + isIsogram(t));
        }
    }
}