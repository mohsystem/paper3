import java.util.*;

public class Task194 {

    // This is the Master interface provided by the problem's environment.
    // A concrete implementation (TestMaster) is provided below for testing purposes.
    interface Master {
        int guess(String word);
    }

    // Helper method to calculate the number of exact matches between two words.
    private int match(String a, String b) {
        int matches = 0;
        for (int i = 0; i < a.length(); i++) {
            if (a.charAt(i) == b.charAt(i)) {
                matches++;
            }
        }
        return matches;
    }

    /**
     * Finds the secret word using a minimax strategy.
     * @param words The list of possible words.
     * @param master The master object to make guesses with.
     */
    public void findSecretWord(String[] words, Master master) {
        // Use a mutable list for the candidates, which will be pruned in each step.
        List<String> candidates = new ArrayList<>(Arrays.asList(words));
        // The problem constraints state allowedGuesses is at most 30.
        // We loop a sufficient number of times, and the external environment tracks the guess limit.
        for (int i = 0; i < 30; i++) {
            if (candidates.isEmpty()) {
                break;
            }

            // Minimax strategy: Choose a word from the current candidates that minimizes the size
            // of the largest remaining group of candidates, no matter the outcome of the guess.
            String bestGuess = candidates.get(0);
            int minMaxGroupSize = Integer.MAX_VALUE;

            for (String w1 : candidates) {
                int[] groups = new int[7]; // groups[i] stores count of words with i matches
                for (String w2 : candidates) {
                    groups[match(w1, w2)]++;
                }
                
                int maxGroup = 0;
                for (int size : groups) {
                    if (size > maxGroup) {
                        maxGroup = size;
                    }
                }
                
                if (maxGroup < minMaxGroupSize) {
                    minMaxGroupSize = maxGroup;
                    bestGuess = w1;
                }
            }

            // Make a guess with the best word found.
            int matches = master.guess(bestGuess);

            // If matches is 6, we've found the secret word.
            if (matches == 6) {
                return;
            }

            // Prune the candidate list based on the feedback (number of matches).
            List<String> nextCandidates = new ArrayList<>();
            for (String word : candidates) {
                if (match(bestGuess, word) == matches) {
                    nextCandidates.add(word);
                }
            }
            candidates = nextCandidates;
        }
    }

    // A concrete implementation of the Master interface for local testing.
    static class TestMaster implements Master {
        private final String secret;
        private final Set<String> wordSet;
        private final int allowedGuesses;
        private int guessesMade;
        private boolean found;

        public TestMaster(String secret, String[] words, int allowedGuesses) {
            this.secret = secret;
            this.wordSet = new HashSet<>(Arrays.asList(words));
            this.allowedGuesses = allowedGuesses;
            this.guessesMade = 0;
            this.found = false;
        }

        @Override
        public int guess(String word) {
            guessesMade++;
            if (!wordSet.contains(word)) {
                return -1;
            }
            if (word.equals(secret)) {
                found = true;
            }
            return new Task194().match(word, secret);
        }

        public void report() {
            if (found && guessesMade <= allowedGuesses) {
                System.out.println("You guessed the secret word correctly.");
            } else {
                System.out.println("Either you took too many guesses, or you did not find the secret word.");
            }
            System.out.println("Guesses made: " + guessesMade);
        }
    }

    public static void main(String[] args) {
        Task194 solver = new Task194();

        // Test Case 1
        System.out.println("Test Case 1:");
        String[] words1 = {"acckzz", "ccbazz", "eiowzz", "abcczz"};
        String secret1 = "acckzz";
        Master master1 = new TestMaster(secret1, words1, 10);
        solver.findSecretWord(words1, master1);
        ((TestMaster) master1).report();

        // Test Case 2
        System.out.println("\nTest Case 2:");
        String[] words2 = {"hamada", "khaled"};
        String secret2 = "hamada";
        Master master2 = new TestMaster(secret2, words2, 10);
        solver.findSecretWord(words2, master2);
        ((TestMaster) master2).report();

        // Test Case 3
        System.out.println("\nTest Case 3:");
        String[] words3 = {"wichbx", "oahwep", "tpulot", "eqznzs", "vvmplb", "eywinm"};
        String secret3 = "oahwep";
        Master master3 = new TestMaster(secret3, words3, 10);
        solver.findSecretWord(words3, master3);
        ((TestMaster) master3).report();

        // Test Case 4
        System.out.println("\nTest Case 4:");
        String[] words4 = {"hbaczn", "aqqbxu", "ajasjw", "rdexkw", "fcceuc", "pcblmb", "bcckqc", "ccbzzw", "agagxt", "ccbazy"};
        String secret4 = "hbaczn";
        Master master4 = new TestMaster(secret4, words4, 10);
        solver.findSecretWord(words4, master4);
        ((TestMaster) master4).report();

        // Test Case 5
        System.out.println("\nTest Case 5:");
        String[] words5 = {"aaaaaa", "bbbbbb", "cccccc"};
        String secret5 = "bbbbbb";
        Master master5 = new TestMaster(secret5, words5, 10);
        solver.findSecretWord(words5, master5);
        ((TestMaster) master5).report();
    }
}