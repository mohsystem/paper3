
import java.util.*;

interface Master {
    int guess(String word);
}

class Task194 {
    public void findSecretWord(String[] words, Master master) {
        List<String> candidates = new ArrayList<>(Arrays.asList(words));
        
        for (int i = 0; i < 10; i++) {
            if (candidates.isEmpty()) break;
            
            String guess = getBestGuess(candidates);
            int matches = master.guess(guess);
            
            if (matches == 6) return;
            
            candidates = filterCandidates(candidates, guess, matches);
        }
    }
    
    private String getBestGuess(List<String> candidates) {
        String bestWord = candidates.get(0);
        int minMaxGroup = Integer.MAX_VALUE;
        
        for (String word : candidates) {
            int[] matchCounts = new int[7];
            for (String candidate : candidates) {
                int matches = countMatches(word, candidate);
                matchCounts[matches]++;
            }
            
            int maxGroup = 0;
            for (int count : matchCounts) {
                maxGroup = Math.max(maxGroup, count);
            }
            
            if (maxGroup < minMaxGroup) {
                minMaxGroup = maxGroup;
                bestWord = word;
            }
        }
        
        return bestWord;
    }
    
    private List<String> filterCandidates(List<String> candidates, String guess, int targetMatches) {
        List<String> filtered = new ArrayList<>();
        for (String word : candidates) {
            if (countMatches(word, guess) == targetMatches) {
                filtered.add(word);
            }
        }
        return filtered;
    }
    
    private int countMatches(String a, String b) {
        int count = 0;
        for (int i = 0; i < 6; i++) {
            if (a.charAt(i) == b.charAt(i)) count++;
        }
        return count;
    }
    
    public static void main(String[] args) {
        // Test cases
        Task194 solution = new Task194();
        
        // Test 1
        Master master1 = new Master() {
            String secret = "acckzz";
            public int guess(String word) {
                if (!Arrays.asList("acckzz","ccbazz","eiowzz","abcczz").contains(word)) return -1;
                int count = 0;
                for (int i = 0; i < 6; i++) {
                    if (word.charAt(i) == secret.charAt(i)) count++;
                }
                return count;
            }
        };
        solution.findSecretWord(new String[]{"acckzz","ccbazz","eiowzz","abcczz"}, master1);
        System.out.println("Test 1 completed");
        
        // Test 2
        Master master2 = new Master() {
            String secret = "hamada";
            public int guess(String word) {
                if (!Arrays.asList("hamada","khaled").contains(word)) return -1;
                int count = 0;
                for (int i = 0; i < 6; i++) {
                    if (word.charAt(i) == secret.charAt(i)) count++;
                }
                return count;
            }
        };
        solution.findSecretWord(new String[]{"hamada","khaled"}, master2);
        System.out.println("Test 2 completed");
    }
}
