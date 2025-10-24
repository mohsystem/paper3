public class Task31 {

    public static String dnaStrand(String dna) {
        StringBuilder result = new StringBuilder();
        for (char c : dna.toCharArray()) {
            switch (c) {
                case 'A':
                    result.append('T');
                    break;
                case 'T':
                    result.append('A');
                    break;
                case 'C':
                    result.append('G');
                    break;
                case 'G':
                    result.append('C');
                    break;
                default:
                    result.append(c);
                    break;
            }
        }
        return result.toString();
    }

    public static void main(String[] args) {
        String[] testCases = {"ATTGC", "GTAT", "AAAA", "CGCG", "TAGCTAGC"};
        for (String testCase : testCases) {
            System.out.println("Input: " + testCase + " --> Output: " + dnaStrand(testCase));
        }
    }
}