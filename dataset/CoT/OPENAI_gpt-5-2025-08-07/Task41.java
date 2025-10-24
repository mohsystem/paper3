// Chain-of-Through process in code generation:
// 1) Problem understanding: Read a string, process it by trimming, collapsing whitespace to single spaces, and converting to uppercase.
// 2) Security requirements: Avoid buffer overflow by using safe Java APIs and immutable Strings; avoid unbounded memory operations.
// 3) Secure coding generation: Use BufferedReader.readLine for input, regex for whitespace collapsing, Locale.ROOT for deterministic uppercase.
// 4) Code review: No manual buffers, no unsafe operations, null checks, straightforward processing.
// 5) Secure code output: Final code reflects the above steps.

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.util.Locale;

public class Task41 {

    // Accepts input as a parameter and returns processed output.
    public static String processInput(String input) {
        if (input == null) return "";
        String collapsed = input.replaceAll("\\s+", " ").trim();
        return collapsed.toUpperCase(Locale.ROOT);
    }

    public static void main(String[] args) throws Exception {
        BufferedReader br = new BufferedReader(new InputStreamReader(System.in, StandardCharsets.UTF_8));
        String userInput = br.readLine();
        if (userInput == null) {
            userInput = "";
        }
        String processed = processInput(userInput);
        System.out.println(processed);

        // 5 test cases
        String[] tests = new String[] {
            "  Hello,   world!  ",
            "",
            "Tabs\tand\nnewlines",
            "1234 abc DEF",
            "   multiple    spaces   here   "
        };
        for (String t : tests) {
            System.out.println(processInput(t));
        }
    }
}