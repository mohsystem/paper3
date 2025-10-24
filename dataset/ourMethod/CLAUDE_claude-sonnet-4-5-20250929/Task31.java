
import java.nio.charset.StandardCharsets;

public class Task31 {
    public static String getDNAComplement(String dna) {
        if (dna == null) {
            throw new IllegalArgumentException("DNA string cannot be null");
        }
        
        if (dna.isEmpty()) {
            throw new IllegalArgumentException("DNA string cannot be empty");
        }
        
        if (dna.length() > 1000000) {
            throw new IllegalArgumentException("DNA string too long");
        }
        
        StringBuilder complement = new StringBuilder(dna.length());
        
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
                    throw new IllegalArgumentException("Invalid nucleotide: " + nucleotide);
            }
        }
        
        return complement.toString();
    }
    
    public static void main(String[] args) {
        String[] testCases = {"ATTGC", "GTAT", "AAAA", "CGCG", "ATCGATCG"};
        String[] expected = {"TAACG", "CATA", "TTTT", "GCGC", "TAGCTAGC"};
        
        for (int i = 0; i < testCases.length; i++) {
            String result = getDNAComplement(testCases[i]);
            System.out.println("Input: " + testCases[i] + " -> Output: " + result + 
                             " (Expected: " + expected[i] + ", Pass: " + result.equals(expected[i]) + ")");
        }
    }
}
