public class Task4 {
    public static boolean isIsogram(String str) {
        if (str == null) return true;
        int mask = 0;
        for (char ch : str.toLowerCase().toCharArray()) {
            if (ch < 'a' || ch > 'z') continue;
            int bit = ch - 'a';
            if ((mask & (1 << bit)) != 0) return false;
            mask |= (1 << bit);
        }
        return true;
    }

    public static void main(String[] args) {
        String[] tests = {"Dermatoglyphics", "aba", "moOse", "", "isogram"};
        for (String t : tests) {
            System.out.println(t + " -> " + isIsogram(t));
        }
    }
}