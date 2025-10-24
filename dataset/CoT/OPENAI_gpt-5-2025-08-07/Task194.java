import java.util.*;

/*
Chain-of-Through process:
1) Problem understanding: Implement a secure solver that finds a secret 6-letter word from a list by querying a Master.guess API under a guess limit.
2) Security requirements: Validate inputs, avoid out-of-bounds, avoid nulls, use safe collections, and prevent misuse of APIs.
3) Secure coding generation: Encapsulate logic, avoid exposing internal states, sanitize usage of external data, and handle edge cases.
4) Code review: Ensure deterministic strategy (minimax), control flow guards, and no excessive resource usage.
5) Secure code output: Return only the specified messages, do not leak internals, enforce constraints.
*/
public class Task194 {
    // Master helper encapsulating the secret, dictionary, and allowed guesses
    static final class Master {
        private final Set<String> dict;
        private final String secret;
        private final int allowed;
        private int calls;
        private boolean success;

        Master(List<String> words, String secret, int allowedGuesses) {
            if (words == null || secret == null) throw new IllegalArgumentException("Null input");
            this.dict = new HashSet<>(words);
            if (!dict.contains(secret)) throw new IllegalArgumentException("Secret not in words");
            if (allowedGuesses < 0) throw new IllegalArgumentException("Negative allowed");
            this.secret = secret;
            this.allowed = allowedGuesses;
            this.calls = 0;
            this.success = false;
        }

        public int guess(String word) {
            if (word == null || word.length() != 6) return -1;
            if (!dict.contains(word)) return -1;
            calls++;
            int m = matchCount(secret, word);
            if (m == 6) success = true;
            return m;
        }

        public String resultMessage() {
            if (success && calls <= allowed) {
                return "You guessed the secret word correctly.";
            }
            return "Either you took too many guesses, or you did not find the secret word.";
        }
    }

    private static int matchCount(String a, String b) {
        int m = 0;
        for (int i = 0; i < 6; i++) if (a.charAt(i) == b.charAt(i)) m++;
        return m;
    }

    // Choose next guess using minimax strategy on current candidates
    private static String chooseGuess(List<String> candidates) {
        String best = candidates.get(0);
        int bestScore = Integer.MAX_VALUE;
        int n = candidates.size();
        for (int i = 0; i < n; i++) {
            String w = candidates.get(i);
            int[] bucket = new int[7];
            for (int j = 0; j < n; j++) {
                int mc = matchCount(w, candidates.get(j));
                bucket[mc]++;
            }
            int worst = 0;
            for (int c : bucket) worst = Math.max(worst, c);
            if (worst < bestScore) {
                bestScore = worst;
                best = w;
            }
        }
        return best;
    }

    // Solver function per requirements
    public static String solve(String[] words, String secret, int allowedGuesses) {
        if (words == null || secret == null) return "Either you took too many guesses, or you did not find the secret word.";
        List<String> wordList = new ArrayList<>();
        for (String w : words) {
            if (w != null && w.length() == 6) wordList.add(w);
        }
        Master master;
        try {
            master = new Master(wordList, secret, allowedGuesses);
        } catch (IllegalArgumentException e) {
            return "Either you took too many guesses, or you did not find the secret word.";
        }
        List<String> candidates = new ArrayList<>(wordList);

        for (int g = 0; g < allowedGuesses && !candidates.isEmpty(); g++) {
            String guess = chooseGuess(candidates);
            int res = master.guess(guess);
            if (res == 6) break;
            List<String> next = new ArrayList<>();
            for (String w : candidates) {
                if (matchCount(w, guess) == res) next.add(w);
            }
            candidates = next;
        }
        return master.resultMessage();
    }

    public static void main(String[] args) {
        // 5 test cases
        String[][] testsWords = new String[][]{
            {"acckzz","ccbazz","eiowzz","abcczz"},
            {"hamada","khaled"},
            {"bbbbbb"},
            {"kitten","kittey","bitten","sitten","mitten","kittzz"},
            {"acbdef","ghijkl","monkey","abcdef","zzzzzz","qwerty","yellow","hammer","flower","planet"}
        };
        String[] testsSecret = new String[]{
            "acckzz",
            "hamada",
            "bbbbbb",
            "mitten",
            "flower"
        };
        int[] testsAllowed = new int[]{10,10,1,10,20};

        for (int i = 0; i < testsSecret.length; i++) {
            String res = solve(testsWords[i], testsSecret[i], testsAllowed[i]);
            System.out.println(res);
        }
    }
}