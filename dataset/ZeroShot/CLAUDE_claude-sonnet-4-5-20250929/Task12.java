
public class Task12 {
    public static String highestScoringWord(String s) {
        if (s == null || s.isEmpty()) {
            return "";
        }
        
        String[] words = s.split(" ");
        String highestWord = "";
        int highestScore = 0;
        
        for (String word : words) {
            int score = calculateScore(word);
            if (score > highestScore) {
                highestScore = score;
                highestWord = word;
            }
        }
        
        return highestWord;
    }
    
    private static int calculateScore(String word) {
        int score = 0;
        for (char c : word.toCharArray()) {
            if (c >= 'a' && c <= 'z') {
                score += (c - 'a' + 1);
            }
        }
        return score;
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println("Test 1: " + highestScoringWord("man i need a taxi up to ubud"));
        
        // Test case 2
        System.out.println("Test 2: " + highestScoringWord("what time are we climbing up the volcano"));
        
        // Test case 3
        System.out.println("Test 3: " + highestScoringWord("take me to semynak"));
        
        // Test case 4
        System.out.println("Test 4: " + highestScoringWord("aa b"));
        
        // Test case 5
        System.out.println("Test 5: " + highestScoringWord("abad"));
    }
}
