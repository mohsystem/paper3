
public class Task3 {
    public static boolean isPangram(String sentence) {
        if (sentence == null || sentence.isEmpty()) {
            return false;
        }
        
        boolean[] alphabet = new boolean[26];
        String lowerSentence = sentence.toLowerCase();
        
        for (int i = 0; i < lowerSentence.length(); i++) {
            char c = lowerSentence.charAt(i);
            if (c >= 'a' && c <= 'z') {
                alphabet[c - 'a'] = true;
            }
        }
        
        for (boolean letterPresent : alphabet) {
            if (!letterPresent) {
                return false;
            }
        }
        
        return true;
    }
    
    public static void main(String[] args) {
        // Test case 1
        String test1 = "The quick brown fox jumps over the lazy dog";
        System.out.println("Test 1: \\"" + test1 + "\\" -> " + isPangram(test1));
        
        // Test case 2
        String test2 = "Hello World";
        System.out.println("Test 2: \\"" + test2 + "\\" -> " + isPangram(test2));
        
        // Test case 3
        String test3 = "abcdefghijklmnopqrstuvwxyz";
        System.out.println("Test 3: \\"" + test3 + "\\" -> " + isPangram(test3));
        
        // Test case 4
        String test4 = "Pack my box with five dozen liquor jugs!";
        System.out.println("Test 4: \\"" + test4 + "\\" -> " + isPangram(test4));
        
        // Test case 5
        String test5 = "1234567890!@#$%^&*()";
        System.out.println("Test 5: \\"" + test5 + "\\" -> " + isPangram(test5));
    }
}
