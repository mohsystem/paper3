import java.util.Objects;

public class Task17 {

    /**
     * Converts a string to Jaden Case.
     *
     * @param phrase The string to convert. Can be null.
     * @return The Jaden-Cased string, or null if the input is null or empty.
     */
    public String toJadenCase(String phrase) {
        if (phrase == null || phrase.isEmpty()) {
            return null;
        }

        char[] chars = phrase.toCharArray();
        boolean capitalizeNext = true;

        for (int i = 0; i < chars.length; i++) {
            if (Character.isWhitespace(chars[i])) {
                capitalizeNext = true;
            } else if (capitalizeNext && Character.isLetter(chars[i])) {
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
            "the quick brown fox jumps over the lazy dog.",
            "a string   with   multiple   spaces"
        };

        System.out.println("Java Test Cases:");
        for (String test : testCases) {
            String result = jadenCaser.toJadenCase(test);
            System.out.println("Original: \"" + test + "\"");
            System.out.println("Jaden-Cased: \"" + result + "\"\n");
        }

        // Test case for null input
        String nullTest = null;
        String nullResult = jadenCaser.toJadenCase(nullTest);
        System.out.println("Original: null");
        System.out.println("Jaden-Cased: " + nullResult + "\n");
    }
}