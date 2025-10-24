import java.util.*;

public class Task194 {

    // Master interface simulator
    static class Master {
        private final Set<String> wordSet;
        private final String secret;
        private final int allowed;
        private int calls;

        Master(List<String> words, String secret, int allowed) {
            if (words == null || secret == null) throw new IllegalArgumentException("Null inputs");
            this.wordSet = new HashSet<>(words);
            if (!wordSet.contains(secret)) throw new IllegalArgumentException("Secret not in words");
            if (allowed < 0) throw new IllegalArgumentException("allowed must be non-negative");
            this.secret = secret;
            this.allowed = allowed;
            this.calls = 0;
        }

        public int guess(String word) {
            // Count every call, even invalid ones, as a guess attempt
            calls++;
            if (!wordSet.contains(word)) return -1;
            return matchCount(word, secret);
        }

        public int getCalls() {
            return calls;
        }

        public int getAllowed() {
            return allowed;
        }
    }

    // Count matches at same positions
    static int matchCount(String a, String b) {
        if (a == null || b == null || a.length() != b.length()) return -1;
        int cnt = 0;
        for (int i = 0; i < a.length(); i++) {
            if (a.charAt(i) == b.charAt(i)) cnt++;
        }
        return cnt;
    }

    // Choose the next guess using minimax on zero-match heuristic
    static String chooseBestGuess(List<String> candidates) {
        int n = candidates.size();
        int bestScore = Integer.MAX_VALUE;
        String best = candidates.get(0);
        for (int i = 0; i < n; i++) {
            String wi = candidates.get(i);
            int zeros = 0;
            for (int j = 0; j < n; j++) {
                if (i == j) continue;
                if (matchCount(wi, candidates.get(j)) == 0) zeros++;
            }
            if (zeros < bestScore) {
                bestScore = zeros;
                best = wi;
            }
        }
        return best;
    }

    // Core solver: returns true if guessed within allowed guesses
    static boolean findSecretWord(List<String> words, Master master) {
        List<String> candidates = new ArrayList<>(words);
        while (!candidates.isEmpty() && master.getCalls() < master.getAllowed()) {
            String guess = chooseBestGuess(candidates);
            int res = master.guess(guess);
            if (res == 6) return true;
            if (res < 0) return false; // invalid guess shouldn't happen in this solver
            List<String> next = new ArrayList<>();
            for (String w : candidates) {
                if (matchCount(w, guess) == res) next.add(w);
            }
            candidates = next;
            if (candidates.size() == 1 && master.getCalls() < master.getAllowed()) {
                int finalRes = master.guess(candidates.get(0));
                return finalRes == 6;
            }
        }
        return false;
    }

    // Public API: accepts inputs and returns required message
    public static String solveTestCase(String secret, String[] wordsArr, int allowedGuesses) {
        if (wordsArr == null) return "Either you took too many guesses, or you did not find the secret word.";
        List<String> words = new ArrayList<>();
        for (String w : wordsArr) {
            if (w != null && w.length() == 6) words.add(w);
        }
        Master master = new Master(words, secret, allowedGuesses);
        boolean guessed = findSecretWord(words, master);
        if (guessed && master.getCalls() <= allowedGuesses) {
            return "You guessed the secret word correctly.";
        } else {
            return "Either you took too many guesses, or you did not find the secret word.";
        }
    }

    public static void main(String[] args) {
        // Test case 1 (Example 1)
        String[] words1 = {"acckzz","ccbazz","eiowzz","abcczz"};
        System.out.println(solveTestCase("acckzz", words1, 10));

        // Test case 2 (Example 2)
        String[] words2 = {"hamada","khaled"};
        System.out.println(solveTestCase("hamada", words2, 10));

        // Test case 3
        String[] words3 = {"spiral","sprint","sprout","sprite","spirit","spring"};
        System.out.println(solveTestCase("sprite", words3, 10));

        // Test case 4
        String[] words4 = {"abacus","absurd","acumen","agenda","zenith","shrimp","zigzag","little","letter","legend"};
        System.out.println(solveTestCase("zenith", words4, 15));

        // Test case 5
        String[] words5 = {"orange","banana","tomato","pepper","potato","carrot","onions","radish","celery","garlic"};
        System.out.println(solveTestCase("orange", words5, 12));
    }
}