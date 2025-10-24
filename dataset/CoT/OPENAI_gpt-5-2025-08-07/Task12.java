// Step 1: Problem understanding
// Implement function to find the highest scoring word based on letter positions.
// Step 2: Security requirements
// Handle null/empty inputs safely. Avoid assumptions about content and use safe loops.
// Step 3: Secure coding generation
// Step 4: Code review
// Step 5: Secure code output

import java.util.*;

public class Task12 {
    public static String highestScoringWord(String s) {
        if (s == null || s.isEmpty()) return "";
        String[] words = s.trim().split("\\s+");
        String bestWord = "";
        long bestScore = Long.MIN_VALUE;

        for (String word : words) {
            long score = 0L;
            for (int i = 0; i < word.length(); i++) {
                char ch = word.charAt(i);
                if (ch >= 'a' && ch <= 'z') {
                    score += (ch - 'a' + 1);
                }
            }
            if (score > bestScore) {
                bestScore = score;
                bestWord = word;
            }
        }
        return bestWord;
    }

    public static void main(String[] args) {
        // 5 test cases
        String[] tests = new String[] {
            "man i need a taxi up to ubud",
            "what time are we climbing up the volcano",
            "take me to semynak",
            "aa b",
            "zoo aaaa"
        };
        for (String t : tests) {
            System.out.println(highestScoringWord(t));
        }
    }
}