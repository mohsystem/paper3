
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
            } else if (capitalizeNext && Character.isLetter(c)) {
                result.append(Character.toUpperCase(c));
                capitalizeNext = false;
            } else {
                result.append(c);
            }
        }
        
        return result.toString();
    }
    
    public static void main(String[] args) {
        String[] testCases = {
            "How can mirrors be real if our eyes aren't real",
            "hello world",
            "i love java programming",
            "it's a beautiful day",
            null
        };
        
        for (int i = 0; i < testCases.length; i++) {
            String result = toJadenCase(testCases[i]);
            System.out.println("Test " + (i + 1) + ": " + result);
        }
    }
}
