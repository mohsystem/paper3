import java.util.*;

public class Task12 {
    public static String high(String s) {
        String[] words = s.split("\\s+");
        String best = "";
        int bestScore = -1;
        for (String w : words) {
            if (w.isEmpty()) continue;
            int score = 0;
            for (int i = 0; i < w.length(); i++) {
                char c = w.charAt(i);
                if (c >= 'a' && c <= 'z') {
                    score += (c - 'a' + 1);
                }
            }
            if (score > bestScore) {
                bestScore = score;
                best = w;
            }
        }
        return best;
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "man i need a taxi up to ubud",
            "what time are we climbing up the volcano",
            "take me to semynak",
            "aa b",
            "b aa"
        };
        for (String t : tests) {
            System.out.println(high(t));
        }
    }
}