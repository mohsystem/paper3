
public class Task12 {
    public static String highestScoringWord(String s) {
        String[] words = s.split(" ");
        String highestWord = "";
        int highestScore = 0;
        
        for (String word : words) {
            int score = 0;
            for (char c : word.toCharArray()) {
                score += c - 'a' + 1;
            }
            if (score > highestScore) {
                highestScore = score;
                highestWord = word;
            }
        }
        
        return highestWord;
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println(highestScoringWord("man i need a taxi up to ubud"));
        
        // Test case 2
        System.out.println(highestScoringWord("what time are we climbing up the volcano"));
        
        // Test case 3
        System.out.println(highestScoringWord("take me to semynak"));
        
        // Test case 4
        System.out.println(highestScoringWord("aa b"));
        
        // Test case 5
        System.out.println(highestScoringWord("abad"));
    }
}
