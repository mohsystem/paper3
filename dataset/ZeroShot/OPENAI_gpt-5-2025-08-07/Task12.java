import java.util.*;

public class Task12 {
    public static String highestScoringWord(String s) {
        if (s == null || s.isEmpty()) return "";
        String[] words = s.trim().split("\\s+");
        long bestScore = Long.MIN_VALUE;
        String bestWord = "";
        for (String w : words) {
            long score = 0;
            for (int i = 0; i < w.length(); i++) {
                char ch = w.charAt(i);
                if (ch >= 'a' && ch <= 'z') {
                    score += (ch - 'a' + 1);
                }
            }
            if (score > bestScore) {
                bestScore = score;
                bestWord = w;
            }
        }
        return bestWord;
    }

    public static void main(String[] args) {
        String[] tests = {
            "abad aa bb",
            "man i need a taxi up to ubud",
            "what time are we climbing up the volcano",
            "take me to semynak",
            "aa b"
        };
        for (String t : tests) {
            System.out.println(highestScoringWord(t));
        }
    }
}