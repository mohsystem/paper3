public class Task31 {

    /**
     * Finds the complementary strand of a DNA sequence.
     * 'A' is complement of 'T', 'C' is complement of 'G'.
     *
     * @param dna A string representing one side of the DNA.
     * @return The complementary DNA strand.
     */
    public static String dnaStrand(String dna) {
        if (dna == null) {
            return ""; // Or throw an exception, depending on contract.
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
                    // As per prompt, input is always valid. A more robust solution
                    // might handle or log invalid characters.
                    complement.append(c);
                    break;
            }
        }
        return complement.toString();
    }

    public static void main(String[] args) {
        String[] testCases = {"ATTGC", "GTAT", "AAAA", "CGCG", "TAACG"};
        
        for (String testCase : testCases) {
            String result = dnaStrand(testCase);
            System.out.println("\"" + testCase + "\" --> \"" + result + "\"");
        }
    }
}