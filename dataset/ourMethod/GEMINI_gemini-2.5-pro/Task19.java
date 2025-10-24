public class Task19 {

    /**
     * Reverses words with five or more letters in a given string.
     *
     * @param sentence The input string of one or more words.
     * @return The modified string.
     */
    public static String spinWords(String sentence) {
        if (sentence == null || sentence.isEmpty()) {
            return "";
        }

        String[] words = sentence.split(" ");
        StringBuilder result = new StringBuilder();

        for (int i = 0; i < words.length; i++) {
            String word = words[i];
            if (word.length() >= 5) {
                result.append(new StringBuilder(word).reverse());
            } else {
                result.append(word);
            }
            if (i < words.length - 1) {
                result.append(" ");
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
            System.out.println("Result: " + (result.equals(expectedResults[i]) ? "PASS" : "FAIL"));
            System.out.println();
        }
    }
}