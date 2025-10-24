import java.util.*;

public class Task194 {
    private static final String SUCCESS_MSG = "You guessed the secret word correctly.";
    private static final String FAILURE_MSG = "Either you took too many guesses, or you did not find the secret word.";

    static class Master {
        private final Set<String> wordSet;
        private final String secret;
        private final int allowed;
        private int guessCount;
        private boolean guessed;

        Master(String[] words, String secret, int allowedGuesses) {
            this.wordSet = new HashSet<>();
            if (words != null) {
                for (String w : words) {
                    if (w != null && w.length() == 6) {
                        this.wordSet.add(w);
                    }
                }
            }
            this.secret = secret;
            this.allowed = Math.max(0, allowedGuesses);
            this.guessCount = 0;
            this.guessed = false;
        }

        public int guess(String word) {
            if (word == null || word.length() != 6) {
                // still count the guess attempt
                guessCount++;
                return -1;
            }
            if (!wordSet.contains(word)) {
                guessCount++;
                return -1;
            }
            guessCount++;
            int matches = matchCount(word, secret);
            if (matches == 6) {
                guessed = true;
            }
            return matches;
        }

        public int getGuessCount() {
            return guessCount;
        }

        public String resultMessage() {
            if (guessed && guessCount <= allowed) {
                return SUCCESS_MSG;
            }
            return FAILURE_MSG;
        }
    }

    private static int matchCount(String a, String b) {
        int m = 0;
        for (int i = 0; i < 6; i++) {
            if (a.charAt(i) == b.charAt(i)) m++;
        }
        return m;
    }

    public static String solve(String[] words, String secret, int allowedGuesses) {
        if (words == null || secret == null || secret.length() != 6) {
            return FAILURE_MSG;
        }
        Master master = new Master(words, secret, allowedGuesses);
        List<Integer> candidates = new ArrayList<>();
        for (int i = 0; i < words.length; i++) {
            if (words[i] != null && words[i].length() == 6) {
                candidates.add(i);
            }
        }
        while (!candidates.isEmpty() && master.getGuessCount() < allowedGuesses) {
            int bestIdx = -1;
            int bestScore = Integer.MAX_VALUE;
            String bestWord = null;

            for (int idx : candidates) {
                String w = words[idx];
                int[] buckets = new int[7];
                for (int jdx : candidates) {
                    String v = words[jdx];
                    int m = matchCount(w, v);
                    buckets[m]++;
                }
                int worst = 0;
                for (int c : buckets) worst = Math.max(worst, c);
                if (worst < bestScore || (worst == bestScore && (bestWord == null || w.compareTo(bestWord) < 0))) {
                    bestScore = worst;
                    bestIdx = idx;
                    bestWord = w;
                }
            }

            if (bestIdx == -1) break;

            String guessWord = words[bestIdx];
            int res = master.guess(guessWord);
            if (res == 6) break;
            if (res < 0) {
                // remove invalid guess from candidates
                List<Integer> next = new ArrayList<>();
                for (int idx : candidates) {
                    if (idx != bestIdx) next.add(idx);
                }
                candidates = next;
                continue;
            }
            List<Integer> filtered = new ArrayList<>();
            for (int idx : candidates) {
                if (matchCount(words[idx], guessWord) == res) {
                    filtered.add(idx);
                }
            }
            candidates = filtered;
        }
        return master.resultMessage();
    }

    public static void main(String[] args) {
        String[][] testsWords = new String[5][];
        String[] testsSecret = new String[5];
        int[] testsAllowed = new int[5];

        testsWords[0] = new String[]{"acckzz", "ccbazz", "eiowzz", "abcczz"};
        testsSecret[0] = "acckzz";
        testsAllowed[0] = 10;

        testsWords[1] = new String[]{"hamada", "khaled"};
        testsSecret[1] = "hamada";
        testsAllowed[1] = 10;

        testsWords[2] = new String[]{"aaaaaa", "bbbbbb", "cccccc", "dddddd", "eeffee", "ffeeee", "fedcba", "abcdef"};
        testsSecret[2] = "abcdef";
        testsAllowed[2] = 10;

        testsWords[3] = new String[]{"absdef", "abqdef", "abzdef", "abzcef", "abzcdx", "abzcdz", "abzcdy", "abztdy", "qbztdy", "xyzabc"};
        testsSecret[3] = "abzcdz";
        testsAllowed[3] = 10;

        testsWords[4] = new String[]{"nearer", "bearer", "hearer", "seared", "neared", "dealer", "leader", "linear"};
        testsSecret[4] = "neared";
        testsAllowed[4] = 12;

        for (int i = 0; i < 5; i++) {
            String result = solve(testsWords[i], testsSecret[i], testsAllowed[i]);
            System.out.println(result);
        }
    }
}