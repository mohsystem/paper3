public class Task12 {

    public static String highestScoringWord(String s) {
        if (s == null || s.isEmpty()) {
            return "";
        }

        String[] words = s.split(" ");
        String highestScoringWord = "";
        int maxScore = -1;

        for (String word : words) {
            int currentScore = 0;
            for (char c : word.toCharArray()) {
                currentScore += c - 'a' + 1;
            }

            if (currentScore > maxScore) {
                maxScore = currentScore;
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
            "aaa b",
            "b aa"
        };
        
        System.out.println("Running Java Test Cases:");
        for (int i = 0; i < testCases.length; i++) {
            System.out.println("Test Case " + (i + 1) + ": \"" + testCases[i] + "\"");
            String result = highestScoringWord(testCases[i]);
            System.out.println("Result: " + result);
            System.out.println("--------------------");
        }
    }
}