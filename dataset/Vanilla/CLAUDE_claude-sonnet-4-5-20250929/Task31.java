
public class Task31 {
    public static String makeComplement(String dna) {
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
            }
        }
        return result.toString();
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
