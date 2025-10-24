import java.util.stream.Collectors;
import java.util.Arrays;

public class Task19 {

    public static String spinWords(String sentence) {
        return Arrays.stream(sentence.split(" "))
                     .map(word -> {
                         if (word.length() >= 5) {
                             return new StringBuilder(word).reverse().toString();
                         }
                         return word;
                     })
                     .collect(Collectors.joining(" "));
    }

    public static void main(String[] args) {
        // Test Case 1
        String input1 = "Hey fellow warriors";
        System.out.println("Input: \"" + input1 + "\"");
        System.out.println("Output: \"" + spinWords(input1) + "\"\n"); // Expected: "Hey wollef sroirraw"

        // Test Case 2
        String input2 = "This is a test";
        System.out.println("Input: \"" + input2 + "\"");
        System.out.println("Output: \"" + spinWords(input2) + "\"\n"); // Expected: "This is a test"

        // Test Case 3
        String input3 = "This is another test";
        System.out.println("Input: \"" + input3 + "\"");
        System.out.println("Output: \"" + spinWords(input3) + "\"\n"); // Expected: "This is rehtona test"

        // Test Case 4
        String input4 = "Welcome";
        System.out.println("Input: \"" + input4 + "\"");
        System.out.println("Output: \"" + spinWords(input4) + "\"\n"); // Expected: "emocleW"

        // Test Case 5
        String input5 = "Just kidding there is still one more";
        System.out.println("Input: \"" + input5 + "\"");
        System.out.println("Output: \"" + spinWords(input5) + "\"\n"); // Expected: "Just gniddik ereht is llits one more"
    }
}