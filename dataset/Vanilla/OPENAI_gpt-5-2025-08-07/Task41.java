import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.util.Locale;

public class Task41 {
    public static String process(String s) {
        if (s == null) return "";
        String collapsed = s.trim().replaceAll("\\s+", " ");
        return collapsed.toUpperCase(Locale.ROOT);
    }

    public static void main(String[] args) throws Exception {
        BufferedReader br = new BufferedReader(new InputStreamReader(System.in, StandardCharsets.UTF_8));
        String userInput = br.readLine();
        if (userInput != null) {
            System.out.println(process(userInput));
        }

        String[] tests = new String[] {
            "  hello   world  ",
            "\tMixed\tCase Input\n",
            "already CLEAN",
            "   ",
            "Symbols 123 !@#   test"
        };
        for (String t : tests) {
            System.out.println(process(t));
        }
    }
}