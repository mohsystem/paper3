public class Task51 {

    /**
     * Encrypts a string using the Caesar cipher algorithm.
     * @param text The string to encrypt.
     * @param shift The number of positions to shift letters.
     * @return The encrypted string.
     */
    public static String encrypt(String text, int shift) {
        if (text == null) {
            return null;
        }
        
        StringBuilder result = new StringBuilder();

        for (char character : text.toCharArray()) {
            if (character >= 'A' && character <= 'Z') {
                char shiftedChar = (char) (((character - 'A' + shift) % 26) + 'A');
                result.append(shiftedChar);
            } else if (character >= 'a' && character <= 'z') {
                char shiftedChar = (char) (((character - 'a' + shift) % 26) + 'a');
                result.append(shiftedChar);
            } else {
                result.append(character);
            }
        }
        return result.toString();
    }

    public static void main(String[] args) {
        // Test Case 1
        String text1 = "Hello, World!";
        int shift1 = 3;
        System.out.println("Test Case 1:");
        System.out.println("Original:  " + text1);
        System.out.println("Shift:     " + shift1);
        System.out.println("Encrypted: " + encrypt(text1, shift1));
        System.out.println("--------------------");

        // Test Case 2
        String text2 = "Programming is fun!";
        int shift2 = 7;
        System.out.println("Test Case 2:");
        System.out.println("Original:  " + text2);
        System.out.println("Shift:     " + shift2);
        System.out.println("Encrypted: " + encrypt(text2, shift2));
        System.out.println("--------------------");

        // Test Case 3
        String text3 = "Caesar Cipher";
        int shift3 = 13; // ROT13
        System.out.println("Test Case 3:");
        System.out.println("Original:  " + text3);
        System.out.println("Shift:     " + shift3);
        System.out.println("Encrypted: " + encrypt(text3, shift3));
        System.out.println("--------------------");
        
        // Test Case 4
        String text4 = "All your base are belong to us.";
        int shift4 = 1;
        System.out.println("Test Case 4:");
        System.out.println("Original:  " + text4);
        System.out.println("Shift:     " + shift4);
        System.out.println("Encrypted: " + encrypt(text4, shift4));
        System.out.println("--------------------");

        // Test Case 5
        String text5 = "Testing with numbers 123 and symbols !@#.";
        int shift5 = 5;
        System.out.println("Test Case 5:");
        System.out.println("Original:  " + text5);
        System.out.println("Shift:     " + shift5);
        System.out.println("Encrypted: " + encrypt(text5, shift5));
        System.out.println("--------------------");
    }
}