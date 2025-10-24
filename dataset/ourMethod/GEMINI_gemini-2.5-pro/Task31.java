public class Task31 {

    /**
     * Given one side of a DNA strand, this function returns the other complementary side.
     * 'A' and 'T' are complements of each other, as are 'C' and 'G'.
     *
     * @param dna A string representing one side of the DNA. It's assumed to contain
     *            only 'A', 'T', 'C', 'G', and will not be null or empty based on prompt.
     *            A null check is included for general robustness.
     * @return The complementary DNA strand.
     */
    public static String makeComplement(String dna) {
        if (dna == null) {
            return null;
        }

        StringBuilder complement = new StringBuilder(dna.length());
        for (char nucleotide : dna.toCharArray()) {
            switch (nucleotide) {
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
                    // For robustness against unexpected characters, append them as is.
                    complement.append(nucleotide);
                    break;
            }
        }
        return complement.toString();
    }

    public static void main(String[] args) {
        String[] testInputs = {"ATTGC", "GTAT", "AAAA", "GCGC", "GATTACA"};
        String[] expectedOutputs = {"TAACG", "CATA", "TTTT", "CGCG", "CTAATGT"};

        for (int i = 0; i < testInputs.length; i++) {
            String input = testInputs[i];
            String expected = expectedOutputs[i];
            String result = makeComplement(input);
            System.out.println("Test Case " + (i + 1) + ":");
            System.out.println("Input:    " + input);
            System.out.println("Output:   " + result);
            System.out.println("Expected: " + expected);
            System.out.println("Result:   " + (expected.equals(result) ? "Passed" : "Failed"));
            System.out.println("--------------------");
        }
    }
}