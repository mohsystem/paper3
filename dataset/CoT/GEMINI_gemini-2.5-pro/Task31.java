public class Task31 {

    /**
     * Given one side of a DNA strand, returns the other complementary side.
     * 'A' and 'T' are complements, 'C' and 'G' are complements.
     *
     * @param dna The input DNA strand (e.g., "ATTGC").
     * @return The complementary DNA strand (e.g., "TAACG").
     */
    public static String makeComplement(String dna) {
        if (dna == null || dna.isEmpty()) {
            return "";
        }
        StringBuilder complement = new StringBuilder();
        for (char c : dna.toCharArray()) {
            switch (c) {
                case 'A':
                    complement.append('T');
                    break;
                case 'T':
                    complement.append('A');
                    break;
                case 'C':
                    complement.append('G');
                    break;
                case 'G':
                    complement.append('C');
                    break;
                default:
                    // In case of invalid characters, we can either ignore them,
                    // append them as is, or throw an exception.
                    // For this problem, we assume valid input based on the description.
                    break;
            }
        }
        return complement.toString();
    }

    public static void main(String[] args) {
        String[] testCases = {
            "ATTGC",
            "GTAT",
            "AAAA",
            "GATTACA",
            "CGCG"
        };

        System.out.println("Java Test Cases:");
        for (String test : testCases) {
            String result = makeComplement(test);
            System.out.println("Input: " + test + " -> Output: " + result);
        }
    }
}