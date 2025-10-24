
import java.util.*;

interface Master {
    int guess(String word);
}

class MockMaster implements Master {
    private String secret;
    private int guessCount;
    private int allowedGuesses;
    private Set<String> wordSet;
    
    public MockMaster(String secret, String[] words, int allowedGuesses) {
        if (secret == null || secret.length() != 6) {
            throw new IllegalArgumentException("Secret must be 6 characters");
        }
        this.secret = secret;
        this.guessCount = 0;
        this.allowedGuesses = allowedGuesses;
        this.wordSet = new HashSet<>(Arrays.asList(words));
        if (!wordSet.contains(secret)) {
            throw new IllegalArgumentException("Secret must be in words");
        }
    }
    
    public int guess(String word) {
        if (word == null || word.length() != 6) {
            return -1;
        }
        guessCount++;
        if (guessCount > allowedGuesses) {
            return -1;
        }
        if (!wordSet.contains(word)) {
            return -1;
        }
        
        int matches = 0;
        for (int i = 0; i < 6; i++) {
            if (word.charAt(i) == secret.charAt(i)) {
                matches++;
            }
        }
        return matches;
    }
    
    public boolean isSuccess() {
        return guessCount <= allowedGuesses;
    }
}

public class Task194 {
    
    private static int calculateMatches(String word1, String word2) {
        if (word1 == null || word2 == null || word1.length() != 6 || word2.length() != 6) {
            return 0;
        }
        int matches = 0;
        for (int i = 0; i < 6; i++) {
            if (word1.charAt(i) == word2.charAt(i)) {
                matches++;
            }
        }
        return matches;
    }
    
    public static void findSecretWord(String[] words, Master master) {
        if (words == null || words.length == 0) {
            return;
        }
        
        List<String> candidates = new ArrayList<>();
        for (String word : words) {
            if (word != null && word.length() == 6) {
                candidates.add(word);
            }
        }
        
        if (candidates.isEmpty()) {
            return;
        }
        
        int maxGuesses = 10;
        int guessCount = 0;
        
        while (!candidates.isEmpty() && guessCount < maxGuesses) {
            String guess = candidates.get(0);
            int matches = master.guess(guess);
            guessCount++;
            
            if (matches == 6) {
                return;
            }
            
            List<String> newCandidates = new ArrayList<>();
            for (String candidate : candidates) {
                if (calculateMatches(guess, candidate) == matches) {
                    newCandidates.add(candidate);
                }
            }
            candidates = newCandidates;
        }
    }
    
    public static void main(String[] args) {
        String[] words1 = {"acckzz", "ccbazz", "eiowzz", "abcczz"};
        MockMaster master1 = new MockMaster("acckzz", words1, 10);
        findSecretWord(words1, master1);
        System.out.println(master1.isSuccess() ? "Test 1 Passed" : "Test 1 Failed");
        
        String[] words2 = {"hamada", "khaled"};
        MockMaster master2 = new MockMaster("hamada", words2, 10);
        findSecretWord(words2, master2);
        System.out.println(master2.isSuccess() ? "Test 2 Passed" : "Test 2 Failed");
        
        String[] words3 = {"gaxckt", "trlccr", "jxwhkz", "ycbfps", "peayuf", "yiejjw"};
        MockMaster master3 = new MockMaster("gaxckt", words3, 10);
        findSecretWord(words3, master3);
        System.out.println(master3.isSuccess() ? "Test 3 Passed" : "Test 3 Failed");
        
        String[] words4 = {"abcdef"};
        MockMaster master4 = new MockMaster("abcdef", words4, 10);
        findSecretWord(words4, master4);
        System.out.println(master4.isSuccess() ? "Test 4 Passed" : "Test 4 Failed");
        
        String[] words5 = {"ccbazz", "eiowzz", "abcczz", "acckzz"};
        MockMaster master5 = new MockMaster("acckzz", words5, 10);
        findSecretWord(words5, master5);
        System.out.println(master5.isSuccess() ? "Test 5 Passed" : "Test 5 Failed");
    }
}
