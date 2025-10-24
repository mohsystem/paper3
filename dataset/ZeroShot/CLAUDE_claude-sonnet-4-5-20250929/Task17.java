
public class Task17 {
    public static String toJadenCase(String phrase) {
        if (phrase == null || phrase.isEmpty()) {
            return null;
        }
        
        StringBuilder result = new StringBuilder();
        boolean capitalizeNext = true;
        
        for (int i = 0; i < phrase.length(); i++) {
            char c = phrase.charAt(i);
            
            if (Character.isWhitespace(c)) {
                result.append(c);
                capitalizeNext = true;
            } else if (capitalizeNext) {
                result.append(Character.toUpperCase(c));
                capitalizeNext = false;
            } else {
                result.append(Character.toLowerCase(c));
            }
        }
        
        return result.toString();
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1: " + toJadenCase("How can mirrors be real if our eyes aren't real"));
        System.out.println("Test 2: " + toJadenCase("the quick brown fox jumps over the lazy dog"));
        System.out.println("Test 3: " + toJadenCase("I'm a single word"));
        System.out.println("Test 4: " + toJadenCase(null));
        System.out.println("Test 5: " + toJadenCase(""));
    }
}
