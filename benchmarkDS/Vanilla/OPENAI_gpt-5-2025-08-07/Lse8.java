public class Lse8 {
    public static String valueAtIndexFromArg1(String[] argv) {
        if (argv == null || argv.length < 2) return null;
        try {
            int idx = Integer.parseInt(argv[1]);
            if (idx >= 0 && idx < argv.length) {
                return argv[idx];
            }
            return null;
        } catch (NumberFormatException e) {
            return null;
        }
    }

    public static void main(String[] args) {
        String[][] tests = new String[][] {
            new String[] {"prog","2","a","b","c"},
            new String[] {"prog","0","X"},
            new String[] {"prog","4","w","x","y","z"},
            new String[] {"prog","1","hello"},
            new String[] {"prog","3","p","q","r","s"}
        };
        for (String[] t : tests) {
            String res = valueAtIndexFromArg1(t);
            System.out.println(res);
        }
    }
}