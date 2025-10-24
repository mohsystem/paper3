import java.util.StringJoiner;

public class Task19 {

    /**
     * Reverses words in a string that have five or more letters.
     *
     * @param sentence The input string of one or more words.
     * @return The string with long words reversed.
     */
    public static String spinWords(String sentence) {
        if (sentence == null || sentence.trim().isEmpty()) {
            return sentence;
        }

        String[] words = sentence.split("\\s+");
        StringJoiner result = new StringJoiner(" ");

        for (String word : words) {
            if (word.length() >= 5) {
                result.add(new StringBuilder(word).reverse().toString());
            } else {
                result.add(word);
            }
        }
        return result.toString();
    }

    public static void main(String[] args) {
        String[] testCases = {
            "Hey fellow warriors",
            "This is a test",
            "This is another test",
            "Welcome",
            "Just kidding there is still one more"
        };

        String[] expectedResults = {
            "Hey wollef sroirraw",
            "This is a test",
            "This is rehtona test",
            "emocleW",
            "Just gniddik ereht is llits one more"
        };

        for (int i = 0; i < testCases.length; i++) {
            String result = spinWords(testCases[i]);
            System.out.println("Test Case " + (i + 1) + ":");
            System.out.println("Input:    \"" + testCases[i] + "\"");
            System.out.println("Output:   \"" + result + "\"");
            System.out.println("Expected: \"" + expectedResults[i] + "\"");
            System.out.println("Result: " + (result.equals(expectedResults[i]) ? "Passed" : "Failed"));
            System.out.println();
        }
    }
}