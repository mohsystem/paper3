public class Task17 {

    public String toJadenCase(String phrase) {
        if (phrase == null || phrase.isEmpty()) {
            return null;
        }

        char[] chars = phrase.toCharArray();
        boolean capitalizeNext = true;

        for (int i = 0; i < chars.length; i++) {
            if (Character.isWhitespace(chars[i])) {
                capitalizeNext = true;
            } else if (capitalizeNext) {
                chars[i] = Character.toUpperCase(chars[i]);
                capitalizeNext = false;
            }
        }
        return new String(chars);
    }

    public static void main(String[] args) {
        Task17 jadenCaser = new Task17();
        String[] testCases = {
            "How can mirrors be real if our eyes aren't real",
            "most trees are blue",
            "",
            null,
            "all the flowers are crying."
        };

        for (int i = 0; i < testCases.length; i++) {
            System.out.println("Test Case " + (i + 1) + ":");
            System.out.println("Original: " + testCases[i]);
            String jadenCased = jadenCaser.toJadenCase(testCases[i]);
            System.out.println("Jaden-Cased: " + jadenCased);
            System.out.println();
        }
    }
}