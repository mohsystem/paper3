
import java.util.*;

interface Master {
    int guess(String word);
}

class Task194 {
    private int countMatches(String a, String b) {
        int matches = 0;
        for (int i = 0; i < a.length(); i++) {
            if (a.charAt(i) == b.charAt(i)) {
                matches++;
            }
        }
        return matches;
    }
    
    public void findSecretWord(String[] words, Master master) {
        List<String> candidates = new ArrayList<>(Arrays.asList(words));
        
        for (int i = 0; i < 10; i++) {
            if (candidates.isEmpty()) break;
            
            // Pick the word that minimizes the maximum group size
            String guess = candidates.get(0);
            int minMaxGroup = Integer.MAX_VALUE;
            
            for (String candidate : candidates) {
                Map<Integer, Integer> groups = new HashMap<>();
                for (String word : candidates) {
                    int matches = countMatches(candidate, word);
                    groups.put(matches, groups.getOrDefault(matches, 0) + 1);
                }
                int maxGroup = Collections.max(groups.values());
                if (maxGroup < minMaxGroup) {
                    minMaxGroup = maxGroup;
                    guess = candidate;
                }
            }
            
            int matches = master.guess(guess);
            if (matches == 6) return;
            
            // Filter candidates based on matches
            List<String> newCandidates = new ArrayList<>();
            for (String word : candidates) {
                if (countMatches(guess, word) == matches) {
                    newCandidates.add(word);
                }
            }
            candidates = newCandidates;
        }
    }
    
    public static void main(String[] args) {
        // Test implementation with mock Master
        Task194 solution = new Task194();
        
        // Test case 1
        String[] words1 = {"acckzz","ccbazz","eiowzz","abcczz"};
        String secret1 = "acckzz";
        System.out.println("Test 1: Expected to find 'acckzz'");
        
        // Test case 2
        String[] words2 = {"hamada","khaled"};
        String secret2 = "hamada";
        System.out.println("Test 2: Expected to find 'hamada'");
    }
}
