import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.PrintStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.util.AbstractMap;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;
import java.util.Optional;

public class Task93 {

    public static List<String> readAndSortKeyValues(String filePath) throws IOException {
        Path path = Paths.get(filePath);
        List<AbstractMap.SimpleEntry<String, String>> items = new ArrayList<>();
        try (BufferedReader br = Files.newBufferedReader(path, StandardCharsets.UTF_8)) {
            String line;
            while ((line = br.readLine()) != null) {
                Optional<AbstractMap.SimpleEntry<String, String>> parsed = parseLine(line);
                parsed.ifPresent(items::add);
            }
        }
        items.sort(Comparator.<AbstractMap.SimpleEntry<String, String>, String>comparing(AbstractMap.SimpleEntry::getKey)
                .thenComparing(AbstractMap.SimpleEntry::getValue));
        List<String> out = new ArrayList<>(items.size());
        for (AbstractMap.SimpleEntry<String, String> kv : items) {
            out.add(kv.getKey() + "=" + kv.getValue());
        }
        return out;
    }

    private static Optional<AbstractMap.SimpleEntry<String, String>> parseLine(String line) {
        if (line == null) return Optional.empty();
        String trimmed = line.trim();
        if (trimmed.isEmpty()) return Optional.empty();
        if (trimmed.startsWith("#") || trimmed.startsWith(";")) return Optional.empty();
        int idx = trimmed.indexOf('=');
        if (idx <= 0) return Optional.empty(); // no '=' or empty key
        String key = trimmed.substring(0, idx).trim();
        String value = trimmed.substring(idx + 1).trim();
        if (key.isEmpty()) return Optional.empty();
        return Optional.of(new AbstractMap.SimpleEntry<>(key, value));
    }

    private static void writeFile(Path path, String content) throws IOException {
        try (BufferedWriter bw = Files.newBufferedWriter(path, StandardCharsets.UTF_8,
                StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING, StandardOpenOption.WRITE)) {
            bw.write(content);
        }
    }

    private static void printResult(PrintStream out, String header, List<String> lines) {
        out.println(header);
        for (String s : lines) {
            out.println(s);
        }
    }

    public static void main(String[] args) throws Exception {
        Path f1 = Paths.get("task93_test1.txt");
        Path f2 = Paths.get("task93_test2.txt");
        Path f3 = Paths.get("task93_test3.txt");
        Path f4 = Paths.get("task93_test4.txt");
        Path f5 = Paths.get("task93_test5.txt");

        writeFile(f1, "b=2\na=1\nc=3\n");
        writeFile(f2, "  x = 10 \n\n=bad\n y= 5  \n  # comment\n");
        writeFile(f3, "a=2\na=1\n");
        writeFile(f4, "# comment\nfoo=bar\ninvalid line\nk=v=extra\n; another comment\n");
        writeFile(f5, "Apple=1\napple=2\nß=sharp\nss=double\n A=9 \n");

        List<String> r1 = readAndSortKeyValues(f1.toString());
        List<String> r2 = readAndSortKeyValues(f2.toString());
        List<String> r3 = readAndSortKeyValues(f3.toString());
        List<String> r4 = readAndSortKeyValues(f4.toString());
        List<String> r5 = readAndSortKeyValues(f5.toString());

        printResult(System.out, "Test 1:", r1);
        printResult(System.out, "Test 2:", r2);
        printResult(System.out, "Test 3:", r3);
        printResult(System.out, "Test 4:", r4);
        printResult(System.out, "Test 5:", r5);
    }
}