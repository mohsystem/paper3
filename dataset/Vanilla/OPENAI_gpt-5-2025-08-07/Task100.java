import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardOpenOption;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Task100 {
    public static List<String> searchInFile(String pattern, String filename) {
        List<String> matches = new ArrayList<>();
        try {
            String content = Files.readString(Path.of(filename), StandardCharsets.UTF_8);
            Pattern p = Pattern.compile(pattern);
            Matcher m = p.matcher(content);
            while (m.find()) {
                matches.add(m.group());
            }
        } catch (Exception e) {
            // Return empty list on errors
        }
        return matches;
    }

    public static void main(String[] args) throws IOException {
        if (args.length >= 2) {
            List<String> res = searchInFile(args[0], args[1]);
            System.out.println("Matches: " + res.size());
            for (String s : res) {
                System.out.println(s);
            }
            return;
        }

        String filename = "test_input.txt";
        String content = ""
                + "foo bar baz 123 foo42\n"
                + "New line with Foo and BAR.\n"
                + "email: test@example.com; ip: 192.168.0.1; dates: 2025-10-04;\n"
                + "ababa bab abab ababab\n";
        Files.writeString(Path.of(filename), content, StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING, StandardOpenOption.WRITE);

        String[] patterns = new String[] {
                "foo",
                "[0-9]+",
                "(ab)+",
                "[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}",
                "192\\.168\\.[0-9]+\\.[0-9]+"
        };

        for (int i = 0; i < patterns.length; i++) {
            List<String> res = searchInFile(patterns[i], filename);
            System.out.println("Test " + (i + 1) + " Pattern: " + patterns[i]);
            System.out.println("Matches: " + res.size());
            for (String s : res) {
                System.out.println(s);
            }
            System.out.println("----");
        }
    }
}