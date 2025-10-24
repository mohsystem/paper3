import java.nio.charset.StandardCharsets;

public class Task19 {
    private static final int MAX_LEN = 100000;

    public static String spinWords(String input) {
        if (input == null) {
            throw new IllegalArgumentException("Input must not be null");
        }
        if (input.length() < 1 || input.length() > MAX_LEN) {
            throw new IllegalArgumentException("Input length out of allowed range");
        }

        // Validate: only letters and spaces
        final int len = input.length();
        for (int i = 0; i < len; i++) {
            char ch = input.charAt(i);
            if (ch != ' ' && !Character.isLetter(ch)) {
                throw new IllegalArgumentException("Input contains invalid characters");
            }
        }

        StringBuilder result = new StringBuilder(len);
        StringBuilder word = new StringBuilder();

        for (int i = 0; i < len; i++) {
            char ch = input.charAt(i);
            if (ch == ' ') {
                if (word.length() >= 5) {
                    result.append(word.reverse());
                    word.setLength(0);
                } else {
                    result.append(word);
                    word.setLength(0);
                }
                result.append(' ');
            } else {
                word.append(ch);
            }
        }

        // Flush last word
        if (word.length() >= 5) {
            result.append(word.reverse());
        } else {
            result.append(word);
        }

        return result.toString();
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "Hey fellow warriors",
            "This is a test",
            "This is another test",
            "spin",
            "Spinning works right now"
        };

        for (String t : tests) {
            try {
                String out = spinWords(t);
                System.out.println(out);
            } catch (IllegalArgumentException e) {
                System.out.println("ERROR");
            }
        }
    }
}