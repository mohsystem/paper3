
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
            } else {
                if (capitalizeNext && Character.isLetter(c)) {
                    result.append(Character.toUpperCase(c));
                    capitalizeNext = false;
                } else {
                    result.append(c);
                }
            }
        }
        
        return result.toString();
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println(toJadenCase("How can mirrors be real if our eyes aren't real"));
        System.out.println(toJadenCase("the quick brown fox jumps over the lazy dog"));
        System.out.println(toJadenCase("i love programming"));
        System.out.println(toJadenCase(""));
        System.out.println(toJadenCase(null));
    }
}
