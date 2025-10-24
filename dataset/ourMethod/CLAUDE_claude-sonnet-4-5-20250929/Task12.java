
import java.nio.charset.StandardCharsets;
import java.util.regex.Pattern;

public class Task12 {
    private static final Pattern VALID_INPUT_PATTERN = Pattern.compile("^[a-z ]*$");
    private static final int MAX_INPUT_LENGTH = 10000;
    
    public static String highestScoringWord(String input) {
        // Validate input
        if (input == null) {
            throw new IllegalArgumentException("Input cannot be null");
        }
        
        // Validate length
        if (input.length() > MAX_INPUT_LENGTH) {
            throw new IllegalArgumentException("Input exceeds maximum length");
        }
        
        // Validate input contains only lowercase letters and spaces
        if (!VALID_INPUT_PATTERN.matcher(input).matches()) {
            throw new IllegalArgumentException("Input must contain only lowercase letters and spaces");
        }
        
        String trimmedInput = input.trim();
        if (trimmedInput.isEmpty()) {
            return "";
        }
        
        String[] words = trimmedInput.split(" ");
        String highestWord = "";
        int highestScore = 0;
        
        for (String word : words) {
            if (word.isEmpty()) {
                continue;
            }
            
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
        byte[] bytes = word.getBytes(StandardCharsets.UTF_8);
        
        for (byte b : bytes) {
            // Validate byte is lowercase letter (a-z: 97-122)
            if (b >= 97 && b <= 122) {
                score += (b - 96); // a=1, b=2, etc.
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
        System.out.println("Test 5: " + highestScoringWord("a z ba"));
    }
}
