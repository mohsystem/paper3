public class Task31 {
    public static String dnaStrand(String dna) {
        StringBuilder sb = new StringBuilder(dna.length());
        for (int i = 0; i < dna.length(); i++) {
            char c = dna.charAt(i);
            switch (c) {
                case 'A': sb.append('T'); break;
                case 'T': sb.append('A'); break;
                case 'C': sb.append('G'); break;
                case 'G': sb.append('C'); break;
                default: sb.append(c); // In case of unexpected characters
            }
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        String[] tests = { "ATTGC", "GTAT", "AAAA", "CGCGTTAA", "A" };
        for (String s : tests) {
            System.out.println(dnaStrand(s));
        }
    }
}