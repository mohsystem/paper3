public class Task12 {

    public static String high(String s) {
        String[] words = s.split(" ");
        String highestScoringWord = "";
        int highestScore = 0;

        for (String word : words) {
            int currentScore = 0;
            for (char c : word.toCharArray()) {
                currentScore += c - 'a' + 1;
            }

            if (currentScore > highestScore) {
                highestScore = currentScore;
                highestScoringWord = word;
            }
        }
        return highestScoringWord;
    }

    public static void main(String[] args) {
        String[] testCases = {
            "man i need a taxi up to ubud",
            "what time are we climbing up the volcano",
            "take me to semynak",
            "bb d",
            "d bb"
        };

        for (String test : testCases) {
            System.out.println("Input: \"" + test + "\"");
            System.out.println("Output: \"" + high(test) + "\"\n");
        }
    }
}