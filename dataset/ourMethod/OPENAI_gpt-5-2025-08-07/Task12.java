import java.nio.charset.StandardCharsets;

public final class Task12 {

    private static final int MAX_LEN = 1_000_000;

    public static String highestScoringWord(String s) {
        if (s == null) {
            throw new IllegalArgumentException("Input cannot be null");
        }
        if (s.length() > MAX_LEN) {
            throw new IllegalArgumentException("Input too long");
        }

        int bestStart = -1;
        int bestLen = 0;
        int bestScore = -1;

        int curStart = -1;
        int curLen = 0;
        int curScore = 0;

        for (int i = 0; i < s.length(); i++) {
            char ch = s.charAt(i);
            if (!Character.isWhitespace(ch)) {
                if (curLen == 0) {
                    curStart = i;
                }
                int sc = (ch >= 'a' && ch <= 'z') ? (ch - 'a' + 1) : 0;
                curScore += sc;
                curLen++;
            } else {
                if (curLen > 0) {
                    if (curScore > bestScore) {
                        bestScore = curScore;
                        bestStart = curStart;
                        bestLen = curLen;
                    }
                    curLen = 0;
                    curScore = 0;
                }
            }
        }

        if (curLen > 0) {
            if (curScore > bestScore) {
                bestScore = curScore;
                bestStart = curStart;
                bestLen = curLen;
            }
        }

        if (bestStart == -1) {
            return "";
        }
        return s.substring(bestStart, bestStart + bestLen);
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "man i need a taxi up to ubud",
            "what time are we climbing up the volcano",
            "take me to semynak",
            "aa b",
            "abad zzz a b c"
        };

        for (String t : tests) {
            String res = highestScoringWord(t);
            System.out.println("Input: \"" + t + "\" -> Highest: \"" + res + "\"");
        }
    }
}