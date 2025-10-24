public class Task17 {

    public String toJadenCase(String phrase) {
        if (phrase == null || phrase.isEmpty()) {
            return null;
        }

        char[] chars = phrase.toCharArray();
        boolean capitalizeNext = true;

        for (int i = 0; i < chars.length; i++) {
            if (capitalizeNext && Character.isLetter(chars[i])) {
                chars[i] = Character.toUpperCase(chars[i]);
                capitalizeNext = false;
            } else if (Character.isWhitespace(chars[i])) {
                capitalizeNext = true;
            }
        }
        return new String(chars);
    }

    public static void main(String[] args) {
        Task17 jadenCaser = new Task17();
        String[] testCases = {
            "How can mirrors be real if our eyes aren't real",
            "",
            null,
            "most trees are blue",
            "the quick brown fox jumps over the lazy dog"
        };

        System.out.println("Java Test Cases:");
        for (int i = 0; i < testCases.length; i++) {
            String original = testCases[i];
            String jadenCased = jadenCaser.toJadenCase(original);
            System.out.println("Test Case " + (i + 1));
            System.out.println("Original: \"" + original + "\"");
            System.out.println("Jaden-Cased: \"" + jadenCased + "\"\n");
        }
    }
}