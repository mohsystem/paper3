import java.util.*;

public class Task194 {

    static class Master {
        private final Set<String> wordSet;
        private final String secret;
        private final int allowedGuesses;
        private int calls = 0;
        private boolean found = false;

        Master(List<String> words, String secret, int allowedGuesses) {
            this.wordSet = new HashSet<>(words);
            this.secret = secret;
            this.allowedGuesses = allowedGuesses;
        }

        int guess(String word) {
            if (!wordSet.contains(word)) return -1;
            calls++;
            int match = matches(word, secret);
            if (match == 6) found = true;
            return match;
        }

        int getCalls() { return calls; }
        boolean isFound() { return found; }
        int getAllowedGuesses() { return allowedGuesses; }

        private int matches(String a, String b) {
            int cnt = 0;
            for (int i = 0; i < 6; i++) if (a.charAt(i) == b.charAt(i)) cnt++;
            return cnt;
        }
    }

    private static int matches(String a, String b) {
        int cnt = 0;
        for (int i = 0; i < 6; i++) if (a.charAt(i) == b.charAt(i)) cnt++;
        return cnt;
    }

    private static String pickBest(List<String> candidates) {
        int n = candidates.size();
        int bestIdx = 0;
        int bestScore = Integer.MAX_VALUE;
        for (int i = 0; i < n; i++) {
            String wi = candidates.get(i);
            int[] buckets = new int[7];
            for (int j = 0; j < n; j++) {
                int m = matches(wi, candidates.get(j));
                buckets[m]++;
            }
            int worst = 0;
            for (int b : buckets) if (b > worst) worst = b;
            if (worst < bestScore) {
                bestScore = worst;
                bestIdx = i;
            }
        }
        return candidates.get(bestIdx);
    }

    public static String guessSecret(List<String> words, String secret, int allowedGuesses) {
        Master master = new Master(words, secret, allowedGuesses);
        List<String> candidates = new ArrayList<>(words);
        while (!candidates.isEmpty() && !master.isFound() && master.getCalls() < master.getAllowedGuesses()) {
            String guess = pickBest(candidates);
            int res = master.guess(guess);
            if (res == 6) break;
            List<String> next = new ArrayList<>();
            for (String w : candidates) if (matches(w, guess) == res) next.add(w);
            candidates = next;
            // Ensure we don't get stuck guessing the same word again if res == 0 and next still contains guess
            candidates.remove(guess);
        }
        if (master.isFound() && master.getCalls() <= master.getAllowedGuesses()) {
            return "You guessed the secret word correctly.";
        } else {
            return "Either you took too many guesses, or you did not find the secret word.";
        }
    }

    public static void main(String[] args) {
        // Test case 1
        List<String> words1 = Arrays.asList("acckzz","ccbazz","eiowzz","abcczz");
        System.out.println(guessSecret(words1, "acckzz", 10));

        // Test case 2
        List<String> words2 = Arrays.asList("hamada","khaled");
        System.out.println(guessSecret(words2, "hamada", 10));

        // Test case 3
        List<String> words3 = Arrays.asList("python","typhon","thynop","cython","pebble","little","button","people","planet","plenty");
        System.out.println(guessSecret(words3, "python", 10));

        // Test case 4
        List<String> words4 = Arrays.asList("planet","planer","planed","planes","platen","palnet","placet","planar");
        System.out.println(guessSecret(words4, "planet", 10));

        // Test case 5
        List<String> words5 = Arrays.asList("silver","salver","sliver","sailor","slider","silken","sinker","sifter","sinner","single");
        System.out.println(guessSecret(words5, "silver", 10));
    }
}