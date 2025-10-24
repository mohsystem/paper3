import java.util.*;

// This is the Master's API interface.
// You should not implement it, or speculate about its implementation.
interface MasterAPI {
    public int guess(String word);
}

// For testing purposes, we create a concrete implementation of the Master API.
class Master implements MasterAPI {
    private final String secret;
    private final Set<String> wordSet;
    private final int allowedGuesses;
    private int guessesMade;
    private boolean found;

    public Master(String secret, String[] words, int allowedGuesses) {
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
        return countMatches(secret, word);
    }
    
    public String getResult() {
        if (found && guessesMade <= allowedGuesses) {
            return "You guessed the secret word correctly.";
        } else {
            return "Either you took too many guesses, or you did not find the secret word.";
        }
    }

    private int countMatches(String w1, String w2) {
        int matches = 0;
        for (int i = 0; i < w1.length(); i++) {
            if (w1.charAt(i) == w2.charAt(i)) {
                matches++;
            }
        }
        return matches;
    }
}

public class Task194 {

    private int countMatches(String w1, String w2) {
        int matches = 0;
        for (int i = 0; i < w1.length(); i++) {
            if (w1.charAt(i) == w2.charAt(i)) {
                matches++;
            }
        }
        return matches;
    }

    public void findSecretWord(String[] words, MasterAPI master, int allowedGuesses) {
        List<String> candidates = new ArrayList<>(Arrays.asList(words));

        for (int i = 0; i < allowedGuesses && !candidates.isEmpty(); i++) {
            String bestGuess = "";
            int minMaxGroupSize = Integer.MAX_VALUE;

            for (String w1 : words) {
                int[] groups = new int[7]; // 0-6 matches
                for (String w2 : candidates) {
                    groups[countMatches(w1, w2)]++;
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

    public static void main(String[] args) {
        Task194 solver = new Task194();

        // Test Case 1
        String[] words1 = {"acckzz", "ccbazz", "eiowzz", "abcczz"};
        Master master1 = new Master("acckzz", words1, 10);
        solver.findSecretWord(words1, master1, 10);
        System.out.println("Test Case 1: " + master1.getResult());

        // Test Case 2
        String[] words2 = {"hamada", "khaled"};
        Master master2 = new Master("hamada", words2, 10);
        solver.findSecretWord(words2, master2, 10);
        System.out.println("Test Case 2: " + master2.getResult());

        // Test Case 3
        String[] words3 = {"aaaaaa", "bbbbbb", "cccccc", "dddddd", "eeeeee", "ffffff"};
        Master master3 = new Master("aaaaaa", words3, 10);
        solver.findSecretWord(words3, master3, 10);
        System.out.println("Test Case 3: " + master3.getResult());

        // Test Case 4
        String[] words4 = {"abacab", "abacba", "ababca", "ababac", "ababab", "bababa", "babacb", "bababc", "bacaba", "bacbab"};
        Master master4 = new Master("abacab", words4, 10);
        solver.findSecretWord(words4, master4, 10);
        System.out.println("Test Case 4: " + master4.getResult());

        // Test Case 5
        String[] words5 = {"abcdef", "zzzzzz", "aaaaaa", "bbbbbb", "cccccc", "dddddd", "eeeeee", "ffffff", "gggggg", "hhhhhh"};
        Master master5 = new Master("zzzzzz", words5, 15);
        solver.findSecretWord(words5, master5, 15);
        System.out.println("Test Case 5: " + master5.getResult());
    }
}