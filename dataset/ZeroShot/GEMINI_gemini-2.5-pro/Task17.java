public class Task17 {

    public static String toJadenCase(String phrase) {
        if (phrase == null || phrase.isEmpty()) {
            return null;
        }

        String[] words = phrase.split("\\s+");
        StringBuilder jadenCased = new StringBuilder();

        for (int i = 0; i < words.length; i++) {
            String word = words[i];
            if (!word.isEmpty()) {
                jadenCased.append(Character.toUpperCase(word.charAt(0)));
                jadenCased.append(word.substring(1));
                if (i < words.length - 1) {
                    jadenCased.append(" ");
                }
            }
        }
        return jadenCased.toString();
    }

    public static void main(String[] args) {
        // Test cases
        String test1 = "How can mirrors be real if our eyes aren't real";
        System.out.println("Original: " + test1);
        System.out.println("Jaden-Cased: " + toJadenCase(test1));

        String test2 = "most trees are blue";
        System.out.println("\nOriginal: " + test2);
        System.out.println("Jaden-Cased: " + toJadenCase(test2));

        String test3 = "";
        System.out.println("\nOriginal: \"" + test3 + "\"");
        System.out.println("Jaden-Cased: " + toJadenCase(test3));
        
        String test4 = null;
        System.out.println("\nOriginal: " + test4);
        System.out.println("Jaden-Cased: " + toJadenCase(test4));

        String test5 = "if a book store never runs out of a certain book, does that mean that nobody reads it, or everybody reads it";
        System.out.println("\nOriginal: " + test5);
        System.out.println("Jaden-Cased: " + toJadenCase(test5));
    }
}