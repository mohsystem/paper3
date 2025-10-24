import java.util.*;

// The Master interface is provided by the problem's environment.
// This is a mock implementation for testing purposes.
interface Master {
    public int guess(String word);
}

public class Task194 {

    // Mock implementation of Master for local testing
    static class MasterImpl implements Master {
        private String secret;
        private Set<String> wordSet;
        private int guesses;
        private int allowedGuesses;
        private boolean found = false;

        public MasterImpl(String secret, String[] words, int allowedGuesses) {
            this.secret = secret;
            this.wordSet = new HashSet<>(Arrays.asList(words));
            this.allowedGuesses = allowedGuesses;
            this.guesses = 0;
        }

        @Override
        public int guess(String word) {
            System.out.println("Guessing: " + word);
            guesses++;
            if (guesses > allowedGuesses) {
                 System.out.println("Exceeded allowed guesses!");
            }
            if (!wordSet.contains(word)) {
                return -1;
            }
            if (word.equals(secret)) {
                found = true;
            }
            return countMatches(secret, word);
        }

        public void printResult() {
            if (found && guesses <= allowedGuesses) {
                System.out.println("You guessed the secret word correctly.");
            } else {
                System.out.println("Either you took too many guesses, or you did not find the secret word.");
            }
        }
    }

    private static int countMatches(String word1, String word2) {
        int matches = 0;
        for (int i = 0; i < word1.length(); i++) {
            if (word1.charAt(i) == word2.charAt(i)) {
                matches++;
            }
        }
        return matches;
    }

    public void findSecretWord(String[] words, Master master) {
        List<String> candidates = new ArrayList<>(Arrays.asList(words));

        for (int i = 0; i < 30; i++) { // Max allowed guesses from constraints
            if (candidates.isEmpty()) break;

            String bestGuess = findBestGuess(words, candidates);
            int matches = master.guess(bestGuess);
            if (matches == 6) {
                return;
            }

            List<String> nextCandidates = new ArrayList<>();
            for (String candidate : candidates) {
                if (countMatches(bestGuess, candidate) == matches) {
                    nextCandidates.add(candidate);
                }
            }
            candidates = nextCandidates;
        }
    }

    private String findBestGuess(String[] allWords, List<String> candidates) {
        if (candidates.size() <= 2) {
            return candidates.get(0);
        }
        String bestWord = "";
        int minMaxPartitionSize = Integer.MAX_VALUE;

        for (String guessWord : allWords) {
            int[] partitionCounts = new int[7];
            for (String candidateWord : candidates) {
                partitionCounts[countMatches(guessWord, candidateWord)]++;
            }
            
            int maxPartitionSize = 0;
            for (int count : partitionCounts) {
                if (count > maxPartitionSize) {
                    maxPartitionSize = count;
                }
            }
            
            if (maxPartitionSize < minMaxPartitionSize) {
                minMaxPartitionSize = maxPartitionSize;
                bestWord = guessWord;
            }
        }
        return bestWord;
    }

    public static void main(String[] args) {
        Task194 solution = new Task194();
        
        // Test Case 1
        System.out.println("--- Test Case 1 ---");
        String[] words1 = {"acckzz","ccbazz","eiowzz","abcczz"};
        MasterImpl master1 = new MasterImpl("acckzz", words1, 10);
        solution.findSecretWord(words1, master1);
        master1.printResult();
        
        // Test Case 2
        System.out.println("\n--- Test Case 2 ---");
        String[] words2 = {"hamada","khaled"};
        MasterImpl master2 = new MasterImpl("hamada", words2, 10);
        solution.findSecretWord(words2, master2);
        master2.printResult();
        
        // Test Case 3
        System.out.println("\n--- Test Case 3 ---");
        String[] words3 = {"wichbx","oahwep","tpulot","eqznrt","vjhszz","pcmatp","xqmsgr",
                           "bnrhml","ccoyyo","ajcwbj","arwfnl","nafmtm","xoaumd","vbejda",
                           "kaefne","swcrkh","reeyhj","vmcwaf","chxitv","qkwjna","vklpkp",
                           "xfnayl","ktgmfn","xrmzzm","fgtuki","zcffuv","srxuus","pydgmq"};
        MasterImpl master3 = new MasterImpl("ccoyyo", words3, 15);
        solution.findSecretWord(words3, master3);
        master3.printResult();
        
        // Test Case 4
        System.out.println("\n--- Test Case 4 ---");
        String[] words4 = {"aaaaaa", "bbbbbb", "cccccc", "dddddd", "eeeeee", "ffffff"};
        MasterImpl master4 = new MasterImpl("aaaaaa", words4, 10);
        solution.findSecretWord(words4, master4);
        master4.printResult();

        // Test Case 5
        System.out.println("\n--- Test Case 5 ---");
        String[] words5 = {"abacac", "abadaf", "adafac", "afacab"};
        MasterImpl master5 = new MasterImpl("abacac", words5, 10);
        solution.findSecretWord(words5, master5);
        master5.printResult();
    }
}