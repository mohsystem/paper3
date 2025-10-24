import java.util.*;

public class Task19 {
    public static String spinWords(String sentence) {
        String[] words = sentence.split(" ");
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < words.length; i++) {
            String w = words[i];
            if (w.length() >= 5) {
                sb.append(new StringBuilder(w).reverse());
            } else {
                sb.append(w);
            }
            if (i < words.length - 1) sb.append(" ");
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "Hey fellow warriors",
            "This is a test",
            "This is another test",
            "Welcome",
            "CodeWars is nice place"
        };
        for (String t : tests) {
            System.out.println(spinWords(t));
        }
    }
}