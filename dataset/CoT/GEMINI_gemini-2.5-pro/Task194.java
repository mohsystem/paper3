import java.util.*;

/**
 * This is the Master's API interface.
 * You should not implement it, or speculate about its implementation
 */
interface Master {
    public int guess(String word);
}

public class Task194 {

    // Helper Master class for testing purposes
    static class MasterImpl implements Master {
        private String secret;
        private Set<String> wordSet;
        private int guessCount = 0;
        private int allowedGuesses;
        private boolean solved = false;

        public MasterImpl(String secret, String[] words, int allowedGuesses) {
            this.secret = secret;
            this.wordSet = new HashSet<>(Arrays.asList(words));
            this.allowedGuesses = allowedGuesses;
        }

        @Override
        public int guess(String word) {
            // The problem statement implies the guess limit check is external to the Master
            guessCount++;
            if (!wordSet.contains(word)) {
                return -1;
            }
            int matches = match(secret, word);
            if (matches == secret.length()) {
                solved = true;
            }
            return matches;
        }
        
        // This method is for the test harness to check the final result.
        public void printResult() {
             if (solved && guessCount <= allowedGuesses) {
                System.out.println("You guessed the secret word correctly.");
            } else {
                System.out.println("Either you took too many guesses, or you did not find the secret word.");
            }
        }
    }

    private static int match(String a, String b) {
        int matches = 0;
        for (int i = 0; i < a.length(); i++) {
            if (a.charAt(i) == b.charAt(i)) {
                matches++;
            }
        }
        return matches;
    }

    public void findSecretWord(String[] words, Master master, int allowedGuesses) {
        List<String> candidates = new ArrayList<>(Arrays.asList(words));

        for (int i = 0; i < allowedGuesses; i++) {
            if (candidates.isEmpty()) {
                return;
            }

            // Minimax strategy: find the word that minimizes the size of the largest possible
            // remaining group of candidates.
            String bestGuess = candidates.get(0);
            int minMaxGroupSize = candidates.size();

            for (String w1 : candidates) {
                int[] groups = new int[7]; // 0 to 6 matches
                for (String w2 : candidates) {
                    groups[match(w1, w2)]++;
                }
                
                int maxGroupSize = 0;
                for (int size : groups) {
                    if (size > maxGroupSize) {
                        maxGroupSize = size;
                    }
                }

                if (maxGroupSize < minMaxGroupSize) {
                    minMaxGroupSize = maxGroupSize;
                    bestGuess = w1;
                }
            }

            int matches = master.guess(bestGuess);
            if (matches == 6) {
                return; // Success
            }

            // Filter the candidates list based on the result
            List<String> nextCandidates = new ArrayList<>();
            for (String candidate : candidates) {
                if (match(candidate, bestGuess) == matches) {
                    nextCandidates.add(candidate);
                }
            }
            candidates = nextCandidates;
        }
    }

    public static void main(String[] args) {
        Task194 solver = new Task194();

        // Test Case 1
        System.out.println("Test Case 1:");
        String[] words1 = {"acckzz","ccbazz","eiowzz","abcczz"};
        String secret1 = "acckzz";
        int allowedGuesses1 = 10;
        MasterImpl master1 = new MasterImpl(secret1, words1, allowedGuesses1);
        solver.findSecretWord(words1, master1, allowedGuesses1);
        master1.printResult();

        // Test Case 2
        System.out.println("\nTest Case 2:");
        String[] words2 = {"hamada", "khaled"};
        String secret2 = "hamada";
        int allowedGuesses2 = 10;
        MasterImpl master2 = new MasterImpl(secret2, words2, allowedGuesses2);
        solver.findSecretWord(words2, master2, allowedGuesses2);
        master2.printResult();

        // Test Case 3
        System.out.println("\nTest Case 3:");
        String[] words3 = {"rccqcb","acckzz","ccbazz","eiowzz","abcczz", "hamada", "bobson", "vrtrva"};
        String secret3 = "bobson";
        int allowedGuesses3 = 10;
        MasterImpl master3 = new MasterImpl(secret3, words3, allowedGuesses3);
        solver.findSecretWord(words3, master3, allowedGuesses3);
        master3.printResult();
        
        // Test Case 4
        System.out.println("\nTest Case 4:");
        String[] words4 = {"banana","cabana","banaba","bandan","banban","bonbon"};
        String secret4 = "banana";
        int allowedGuesses4 = 15;
        MasterImpl master4 = new MasterImpl(secret4, words4, allowedGuesses4);
        solver.findSecretWord(words4, master4, allowedGuesses4);
        master4.printResult();
        
        // Test Case 5
        System.out.println("\nTest Case 5:");
        String[] words5 = {"abcdef","abdfed","acbdef","aefdcb","abdcef","abcefd"};
        String secret5 = "abcdef";
        int allowedGuesses5 = 10;
        MasterImpl master5 = new MasterImpl(secret5, words5, allowedGuesses5);
        solver.findSecretWord(words5, master5, allowedGuesses5);
        master5.printResult();
    }
}