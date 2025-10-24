public class Task8 {
    public static char findMissingLetter(char[] array) {
        for (int i = 1; i < array.length; i++) {
            if (array[i] != array[i - 1] + 1) {
                return (char) (array[i - 1] + 1);
            }
        }
        return (char) (array[array.length - 1] + 1);
    }

    public static void main(String[] args) {
        char[][] tests = {
            new char[]{'a','b','c','d','f'},
            new char[]{'O','Q','R','S'},
            new char[]{'b','c','d','e','g'},
            new char[]{'A','B','D'},
            new char[]{'m','n','p','q','r'}
        };
        for (char[] t : tests) {
            System.out.println(findMissingLetter(t));
        }
    }
}