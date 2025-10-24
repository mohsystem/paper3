import java.nio.charset.StandardCharsets;

public class Task17 {

    public static String toJadenCase(String s) {
        if (s == null || s.isEmpty()) {
            return null;
        }
        StringBuilder sb = new StringBuilder(s.length());
        boolean capitalize = true;
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (Character.isLetter(c)) {
                if (capitalize) {
                    sb.append(Character.toUpperCase(c));
                    capitalize = false;
                } else {
                    sb.append(Character.toLowerCase(c));
                }
            } else {
                sb.append(c);
                if (c == ' ') {
                    capitalize = true;
                }
            }
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "How can mirrors be real if our eyes aren't real",
            "",
            null,
            "  leading and trailing  ",
            "don't stop-believin' tEST-inG"
        };

        for (int i = 0; i < tests.length; i++) {
            String input = tests[i];
            String output = toJadenCase(input);
            System.out.println("Test " + (i + 1) + ": " + String.valueOf(input) + " -> " + String.valueOf(output));
        }
    }
}