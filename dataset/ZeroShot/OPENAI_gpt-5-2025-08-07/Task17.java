import java.util.*;

public class Task17 {
    public static String toJadenCase(String phrase) {
        if (phrase == null || phrase.isEmpty()) {
            return null;
        }
        StringBuilder sb = new StringBuilder(phrase.length());
        boolean newWord = true;
        for (int i = 0; i < phrase.length(); i++) {
            char ch = phrase.charAt(i);
            if (Character.isWhitespace(ch)) {
                sb.append(ch);
                newWord = true;
            } else {
                if (newWord && Character.isLetter(ch)) {
                    sb.append(Character.toUpperCase(ch));
                } else if (Character.isLetter(ch)) {
                    sb.append(Character.toLowerCase(ch));
                } else {
                    sb.append(ch);
                }
                newWord = false;
            }
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "How can mirrors be real if our eyes aren't real",
            "",
            "most trees are blue",
            "HELLO WORLD",
            "multiple   spaces\tand tabs"
        };
        for (String t : tests) {
            System.out.println(toJadenCase(t));
        }
    }
}