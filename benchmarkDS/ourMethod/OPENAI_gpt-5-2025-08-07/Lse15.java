import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;

public class Lse15 {

    // Returns the value at the given index if valid; otherwise returns null.
    public static Integer getValueAtIndex(int[] id_sequence, int index) {
        if (id_sequence == null) {
            return null;
        }
        if (index < 0 || index >= id_sequence.length) {
            return null;
        }
        return id_sequence[index];
    }

    // Prompts the user for an index and returns the value at that index if valid; otherwise returns null.
    public static Integer promptAndGetValue(int[] id_sequence, BufferedReader reader) throws IOException {
        if (id_sequence == null || reader == null) {
            return null;
        }
        System.out.print("Enter index (0.." + (id_sequence.length - 1) + "): ");
        String line = reader.readLine();
        if (line == null) {
            System.out.println("No input.");
            return null;
        }
        line = line.trim();
        if (line.length() == 0 || line.length() > 18) {
            System.out.println("Invalid input.");
            return null;
        }
        int idx;
        try {
            idx = Integer.parseInt(line);
        } catch (NumberFormatException e) {
            System.out.println("Invalid input.");
            return null;
        }
        Integer val = getValueAtIndex(id_sequence, idx);
        if (val == null) {
            System.out.println("Invalid index.");
            return null;
        }
        return val;
    }

    public static void main(String[] args) throws IOException {
        int[] id_sequence = new int[] {42, 7, 13, 99, 5, 81, 23, 64, 12, 30};
        try (BufferedReader reader = new BufferedReader(new InputStreamReader(System.in, StandardCharsets.UTF_8))) {
            for (int t = 1; t <= 5; t++) {
                Integer result = promptAndGetValue(id_sequence, reader);
                if (result != null) {
                    System.out.println("Value: " + result);
                } else {
                    System.out.println("No value returned.");
                }
            }
        }
    }
}