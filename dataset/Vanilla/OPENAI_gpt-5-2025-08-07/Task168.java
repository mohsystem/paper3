public class Task168 {
    public static String tweakLetters(String s, int[] arr) {
        char[] chars = s.toLowerCase().toCharArray();
        for (int i = 0; i < chars.length && i < arr.length; i++) {
            char ch = chars[i];
            if (ch >= 'a' && ch <= 'z') {
                int pos = ch - 'a';
                pos = (pos + arr[i]) % 26;
                if (pos < 0) pos += 26;
                chars[i] = (char) ('a' + pos);
            }
        }
        return new String(chars);
    }

    public static void main(String[] args) {
        String[] words = {
            "apple",
            "many",
            "rhino",
            "xyz",
            "zoo"
        };
        int[][] shifts = {
            {0, 1, -1, 0, -1},
            {0, 0, 0, -1},
            {1, 1, 1, 1, 1},
            {1, 1, 1},
            {1, 0, 0}
        };
        for (int i = 0; i < words.length; i++) {
            System.out.println(tweakLetters(words[i], shifts[i]));
        }
    }
}