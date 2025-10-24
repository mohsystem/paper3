
public class Task168 {
    public static String tweakLetters(String str, int[] tweaks) {
        StringBuilder result = new StringBuilder();
        for (int i = 0; i < str.length(); i++) {
            char c = str.charAt(i);
            int tweakedChar = (int) c + tweaks[i];
            result.append((char) tweakedChar);
        }
        return result.toString();
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println(tweakLetters("apple", new int[]{0, 1, -1, 0, -1})); // "aqold"
        
        // Test case 2
        System.out.println(tweakLetters("many", new int[]{0, 0, 0, -1})); // "manx"
        
        // Test case 3
        System.out.println(tweakLetters("rhino", new int[]{1, 1, 1, 1, 1})); // "sijop"
        
        // Test case 4
        System.out.println(tweakLetters("hello", new int[]{0, 0, 0, 0, 0})); // "hello"
        
        // Test case 5
        System.out.println(tweakLetters("code", new int[]{-1, 1, -1, 1})); // "bndf"
    }
}
