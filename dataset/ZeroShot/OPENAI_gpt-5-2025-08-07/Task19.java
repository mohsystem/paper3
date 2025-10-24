import java.util.Arrays;

public class Task19 {
    public static String spinWords(String s) {
        if (s == null) return "";
        StringBuilder result = new StringBuilder(s.length());
        int n = s.length();
        int i = 0;
        while (i < n) {
            if (s.charAt(i) == ' ') {
                result.append(' ');
                i++;
            } else {
                int start = i;
                while (i < n && s.charAt(i) != ' ') {
                    i++;
                }
                int len = i - start;
                if (len >= 5) {
                    for (int k = i - 1; k >= start; k--) {
                        result.append(s.charAt(k));
                    }
                } else {
                    result.append(s, start, i);
                }
            }
        }
        return result.toString();
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "Hey fellow warriors",
            "This is a test",
            "This is another test",
            "",
            "Hi   there"
        };
        for (String t : tests) {
            String out = spinWords(t);
            System.out.println("Input: \"" + t + "\" -> Output: \"" + out + "\"");
        }
    }
}