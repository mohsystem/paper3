public class Task3 {

    /**
     * Checks if a string is a pangram.
     * A pangram is a sentence containing every letter of the alphabet at least once.
     * This function is case-insensitive and ignores numbers and punctuation.
     *
     * @param sentence The string to check.
     * @return true if the string is a pangram, false otherwise.
     */
    public static boolean isPangram(String sentence) {
        // A boolean array to mark the presence of each letter of the alphabet.
        boolean[] alphabetPresent = new boolean[26];
        int uniqueLettersCount = 0;

        if (sentence == null) {
            return false;
        }

        // Convert the string to lowercase to make the check case-insensitive.
        String lowerCaseSentence = sentence.toLowerCase();

        for (int i = 0; i < lowerCaseSentence.length(); i++) {
            char ch = lowerCaseSentence.charAt(i);

            // Check if the character is a lowercase letter.
            if (ch >= 'a' && ch <= 'z') {
                int index = ch - 'a';

                // If this letter has not been seen before, mark it as seen and increment the count.
                if (!alphabetPresent[index]) {
                    alphabetPresent[index] = true;
                    uniqueLettersCount++;
                }
            }
            
            // Optimization: if all 26 letters have been found, we can exit early.
            if (uniqueLettersCount == 26) {
                return true;
            }
        }

        // The sentence is a pangram if we have found all 26 unique letters.
        return uniqueLettersCount == 26;
    }

    public static void main(String[] args) {
        String[] testCases = {
            "The quick brown fox jumps over the lazy dog",
            "Pack my box with five dozen liquor jugs.",
            "This is not a pangram",
            "Abcdefghijklmnopqrstuvwxyz123!@#",
            ""
        };

        for (int i = 0; i < testCases.length; i++) {
            System.out.println("Test Case " + (i + 1) + ": \"" + testCases[i] + "\"");
            System.out.println("Is Pangram? " + isPangram(testCases[i]));
            System.out.println();
        }
    }
}