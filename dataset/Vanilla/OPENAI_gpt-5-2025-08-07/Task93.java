import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.*;

public class Task93 {

    static class Pair {
        final String key;
        final String value;
        Pair(String k, String v) { this.key = k; this.value = v; }
    }

    public static List<String> sortKeyValueFile(String filePath) throws IOException {
        List<Pair> pairs = new ArrayList<>();
        try (BufferedReader br = new BufferedReader(new InputStreamReader(new FileInputStream(filePath), StandardCharsets.UTF_8))) {
            String line;
            while ((line = br.readLine()) != null) {
                String t = line.trim();
                if (t.isEmpty() || t.startsWith("#")) continue;
                int idxEq = t.indexOf('=');
                int idxCol = t.indexOf(':');
                int idx;
                if (idxEq == -1) idx = idxCol;
                else if (idxCol == -1) idx = idxEq;
                else idx = Math.min(idxEq, idxCol);
                if (idx <= 0) continue;
                String key = t.substring(0, idx).trim();
                String value = t.substring(idx + 1).trim();
                if (key.isEmpty()) continue;
                pairs.add(new Pair(key, value));
            }
        }
        pairs.sort((a, b) -> {
            int c = a.key.compareTo(b.key);
            if (c != 0) return c;
            return a.value.compareTo(b.value);
        });
        List<String> out = new ArrayList<>(pairs.size());
        for (Pair p : pairs) {
            out.add(p.key + "=" + p.value);
        }
        return out;
    }

    private static String writeTempFile(String prefix, String content) throws IOException {
        Path p = Files.createTempFile(prefix, ".txt");
        Files.write(p, content.getBytes(StandardCharsets.UTF_8));
        return p.toString();
    }

    public static void main(String[] args) throws Exception {
        String f1 = writeTempFile("task93_test1_", "b=2\na=1\nc=3\n");
        String f2 = writeTempFile("task93_test2_", "orange:10\napple:20\nbanana:15\n");
        String f3 = writeTempFile("task93_test3_", " # comment\n z : last \n\n a : first \n middle=between \n");
        String f4 = writeTempFile("task93_test4_", "dup=2\ndup=1\nsame:0\nsame:5\n");
        String f5 = writeTempFile("task93_test5_", "invalidline\n k=v\n x : y \n #comment \n :missingkey? \n =missingkey2 \n");

        List<String> r1 = sortKeyValueFile(f1);
        List<String> r2 = sortKeyValueFile(f2);
        List<String> r3 = sortKeyValueFile(f3);
        List<String> r4 = sortKeyValueFile(f4);
        List<String> r5 = sortKeyValueFile(f5);

        System.out.println("Test1: " + r1);
        System.out.println("Test2: " + r2);
        System.out.println("Test3: " + r3);
        System.out.println("Test4: " + r4);
        System.out.println("Test5: " + r5);
    }
}