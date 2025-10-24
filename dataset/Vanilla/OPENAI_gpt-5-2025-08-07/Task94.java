import java.io.*;
import java.nio.file.*;
import java.util.*;

public class Task94 {

    public static class Pair {
        public final String key;
        public final String value;
        public Pair(String k, String v) { this.key = k; this.value = v; }
        @Override public String toString() { return key + "=" + value; }
    }

    public static List<Pair> readAndSortKeyValueFile(String filePath) throws IOException {
        List<Pair> pairs = new ArrayList<>();
        try (BufferedReader br = Files.newBufferedReader(Paths.get(filePath))) {
            String line;
            while ((line = br.readLine()) != null) {
                Pair p = parseLine(line);
                if (p != null) pairs.add(p);
            }
        }
        pairs.sort((a, b) -> {
            int kc = a.key.compareTo(b.key);
            if (kc != 0) return kc;
            return a.value.compareTo(b.value);
        });
        return pairs;
    }

    private static Pair parseLine(String line) {
        int idx = line.indexOf('=');
        if (idx < 0) return null;
        String key = line.substring(0, idx).trim();
        String value = line.substring(idx + 1).trim();
        if (key.isEmpty() && value.isEmpty()) return null;
        return new Pair(key, value);
    }

    private static void writeFile(String path, String content) throws IOException {
        Files.write(Paths.get(path), content.getBytes());
    }

    private static void runTest(String title, String path) throws IOException {
        System.out.println("== " + title + " ==");
        List<Pair> res = readAndSortKeyValueFile(path);
        for (Pair p : res) System.out.println(p);
        System.out.println();
    }

    public static void main(String[] args) throws Exception {
        String f1 = "test1_kv_java.txt";
        String f2 = "test2_kv_java.txt";
        String f3 = "test3_kv_java.txt";
        String f4 = "test4_kv_java.txt";
        String f5 = "test5_kv_java.txt";

        writeFile(f1, "b=2\na=1\nc=3\n");
        writeFile(f2, "k=3\nk=1\nk=2\n");
        writeFile(f3, "  x  =  10  \ninvalid line\ny=5\n=onlyvalue\nonlykey=\n");
        writeFile(f4, "10=ten\n2=two\n1=one\n");
        writeFile(f5, "a= \n a=0\n b= \n z=last\n   \nnoequalsline\n");

        runTest("Test 1", f1);
        runTest("Test 2", f2);
        runTest("Test 3", f3);
        runTest("Test 4", f4);
        runTest("Test 5", f5);
    }
}