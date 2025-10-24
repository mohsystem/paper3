
import java.util.*;

interface Master {
    public int guess(String word);
}

class Task194 {
    private int match(String a, String b) {
        int matches = 0;
        for (int i = 0; i < 6; i++) {
            if (a.charAt(i) == b.charAt(i)) {
                matches++;
            }
        }
        return matches;
    }
    
    public void findSecretWord(String[] words, Master master) {
        List<String> candidates = new ArrayList<>(Arrays.asList(words));
        
        for (int attempt = 0; attempt < 10 && !candidates.isEmpty(); attempt++) {
            String guess = selectBestGuess(candidates);
            int matches = master.guess(guess);
            
            if (matches == 6) {
                return;
            }
            
            List<String> newCandidates = new ArrayList<>();
            for (String word : candidates) {
                if (match(guess, word) == matches) {
                    newCandidates.add(word);
                }
            }
            candidates = newCandidates;
        }
    }
    
    private String selectBestGuess(List<String> candidates) {
        if (candidates.size() <= 2) {
            return candidates.get(0);
        }
        
        String bestWord = candidates.get(0);
        int minMaxGroup = Integer.MAX_VALUE;
        
        for (String candidate : candidates) {
            int[] groups = new int[7];
            for (String word : candidates) {
                groups[match(candidate, word)]++;
            }
            
            int maxGroup = 0;
            for (int count : groups) {
                maxGroup = Math.max(maxGroup, count);
            }
            
            if (maxGroup < minMaxGroup) {
                minMaxGroup = maxGroup;
                bestWord = candidate;
            }
        }
        
        return bestWord;
    }
    
    public static void main(String[] args) {
        Task194 solution = new Task194();
        
        // Test case 1
        Master master1 = new Master() {
            private String secret = "acckzz";
            private String[] words = {"acckzz","ccbazz","eiowzz","abcczz"};
            
            public int guess(String word) {
                if (!Arrays.asList(words).contains(word)) return -1;
                int matches = 0;
                for (int i = 0; i < 6; i++) {
                    if (word.charAt(i) == secret.charAt(i)) matches++;
                }
                return matches;
            }
        };
        solution.findSecretWord(new String[]{"acckzz","ccbazz","eiowzz","abcczz"}, master1);
        System.out.println("Test 1 passed");
        
        // Test case 2
        Master master2 = new Master() {
            private String secret = "hamada";
            private String[] words = {"hamada","khaled"};
            
            public int guess(String word) {
                if (!Arrays.asList(words).contains(word)) return -1;
                int matches = 0;
                for (int i = 0; i < 6; i++) {
                    if (word.charAt(i) == secret.charAt(i)) matches++;
                }
                return matches;
            }
        };
        solution.findSecretWord(new String[]{"hamada","khaled"}, master2);
        System.out.println("Test 2 passed");
    }
}
