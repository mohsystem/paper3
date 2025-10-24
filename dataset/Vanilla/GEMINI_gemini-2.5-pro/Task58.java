public class Task58 {

    public static boolean XO(String str) {
        int xCount = 0;
        int oCount = 0;
        String lowerStr = str.toLowerCase();
        for (int i = 0; i < lowerStr.length(); i++) {
            char c = lowerStr.charAt(i);
            if (c == 'x') {
                xCount++;
            } else if (c == 'o') {
                oCount++;
            }
        }
        return xCount == oCount;
    }

    public static void main(String[] args) {
        String[] testCases = {"ooxx", "xooxx", "ooxXm", "zpzpzpp", "zzoo"};
        for (String test : testCases) {
            System.out.println("XO(\"" + test + "\") => " + XO(test));
        }
    }
}