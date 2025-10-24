import java.util.*;
import java.io.*;

public class Task190 {
    public static String transpose(String content) {
        if (content == null) return "";
        content = content.replace("\r", "");
        String[] lines = content.split("\n", -1);
        List<String[]> rows = new ArrayList<>();
        for (String line : lines) {
            if (line.length() == 0) continue; // skip empty lines
            // Split by whitespace to be slightly robust (though spec says single space)
            String[] parts = line.trim().split("\\s+");
            if (parts.length > 0) rows.add(parts);
        }
        if (rows.isEmpty()) return "";
        int r = rows.size();
        int c = rows.get(0).length;
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < c; i++) {
            for (int j = 0; j < r; j++) {
                sb.append(rows.get(j)[i]);
                if (j < r - 1) sb.append(' ');
            }
            if (i < c - 1) sb.append('\n');
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "name age\nalice 21\nryan 30",
            "a b c",
            "a\na\na",
            "1 2 3\n4 5 6\n7 8 9",
            "w x y z\n1 2 3 4"
        };
        for (int i = 0; i < tests.length; i++) {
            String out = transpose(tests[i]);
            System.out.println(out);
            if (i < tests.length - 1) System.out.println("-----");
        }
    }
}