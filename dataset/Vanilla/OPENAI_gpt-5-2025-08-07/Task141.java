public class Task141 {
    public static String reverseString(String s) {
        return new StringBuilder(s).reverse().toString();
    }

    public static void main(String[] args) {
        String[] tests = {"", "a", "abcde", "Hello, World!", "12345 racecar"};
        for (String t : tests) {
            System.out.println(reverseString(t));
        }
    }
}