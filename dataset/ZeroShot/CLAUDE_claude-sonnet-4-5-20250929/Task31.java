
public class Task31 {
    public static String getDNAComplement(String dna) {
        if (dna == null || dna.isEmpty()) {
            return "";
        }
        
        StringBuilder complement = new StringBuilder();
        
        for (int i = 0; i < dna.length(); i++) {
            char nucleotide = dna.charAt(i);
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
                    throw new IllegalArgumentException("Invalid DNA nucleotide: " + nucleotide);
            }
        }
        
        return complement.toString();
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println("Input: ATTGC");
        System.out.println("Output: " + getDNAComplement("ATTGC"));
        System.out.println("Expected: TAACG\\n");
        
        // Test case 2
        System.out.println("Input: GTAT");
        System.out.println("Output: " + getDNAComplement("GTAT"));
        System.out.println("Expected: CATA\\n");
        
        // Test case 3
        System.out.println("Input: AAAA");
        System.out.println("Output: " + getDNAComplement("AAAA"));
        System.out.println("Expected: TTTT\\n");
        
        // Test case 4
        System.out.println("Input: CGCG");
        System.out.println("Output: " + getDNAComplement("CGCG"));
        System.out.println("Expected: GCGC\\n");
        
        // Test case 5
        System.out.println("Input: ATCGATCG");
        System.out.println("Output: " + getDNAComplement("ATCGATCG"));
        System.out.println("Expected: TAGCTAGC\\n");
    }
}
