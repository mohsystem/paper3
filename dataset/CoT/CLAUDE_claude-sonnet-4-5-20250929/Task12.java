
public class Task12 {
    public static String highestScoringWord(String str) {
        if (str == null || str.trim().isEmpty()) {
            return "";
        }
        
        String[] words = str.trim().split("\\\\s+");
        String highestWord = "";
        int maxScore = 0;
        
        for (String word : words) {
            int score = calculateScore(word);
            if (score > maxScore) {
                maxScore = score;
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
        System.out.println("Test 1: " + highestScoringWord("man i need a taxi up to ubud"));
        System.out.println("Test 2: " + highestScoringWord("what time are we climbing up the volcano"));
        System.out.println("Test 3: " + highestScoringWord("take me to semynak"));
        System.out.println("Test 4: " + highestScoringWord("aa b"));
        System.out.println("Test 5: " + highestScoringWord("a z"));
    }
}
