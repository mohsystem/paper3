
public class Task31 {
    public static String makeComplement(String dna) {
        if (dna == null) {
            return null;
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
        System.out.println("Input: ATTGC, Output: " + makeComplement("ATTGC"));
        
        // Test case 2
        System.out.println("Input: GTAT, Output: " + makeComplement("GTAT"));
        
        // Test case 3
        System.out.println("Input: AAAA, Output: " + makeComplement("AAAA"));
        
        // Test case 4
        System.out.println("Input: CGCG, Output: " + makeComplement("CGCG"));
        
        // Test case 5
        System.out.println("Input: ATCGATCG, Output: " + makeComplement("ATCGATCG"));
    }
}
