
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
                result.append(c);
            }
        }
        
        return result.toString();
    }
    
    public static void main(String[] args) {
        System.out.println("Test 1: " + toJadenCase("How can mirrors be real if our eyes aren't real"));
        System.out.println("Test 2: " + toJadenCase("this is a test"));
        System.out.println("Test 3: " + toJadenCase("i love programming"));
        System.out.println("Test 4: " + toJadenCase(null));
        System.out.println("Test 5: " + toJadenCase(""));
    }
}
