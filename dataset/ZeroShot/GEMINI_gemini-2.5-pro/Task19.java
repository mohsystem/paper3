import java.util.stream.Collectors;
import java.util.Arrays;

public class Task19 {

    /**
     * Reverses words in a sentence that have five or more letters.
     *
     * @param sentence The input string containing words and spaces.
     * @return The modified string with long words reversed.
     */
    public static String spinWords(String sentence) {
        if (sentence == null) {
            return "";
        }
        // Split by one or more whitespace characters
        return Arrays.stream(sentence.split("\\s+"))
                .map(word -> {
                    if (word.length() >= 5) {
                        return new StringBuilder(word).reverse().toString();
                    } else {
                        return word;
                    }
                })
                .collect(Collectors.joining(" "));
    }

    public static void main(String[] args) {
        String[] testCases = {
            "Hey fellow warriors",
            "This is a test",
            "This is another test",
            "Welcome to the jungle",
            "Supercalifragilisticexpialidocious"
        };
        
        for (String test : testCases) {
            System.out.println(spinWords(test));
        }
    }
}