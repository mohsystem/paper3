public class Task113 {
    public static String concatenate(String[] parts) {
        StringBuilder sb = new StringBuilder();
        if (parts != null) {
            for (String s : parts) {
                if (s != null) sb.append(s);
            }
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        String[][] tests = new String[][]{
            new String[]{"Hello", " ", "World"},
            new String[]{"", "abc", "", "123"},
            new String[]{"Join", "-", "these", "-", "together"},
            new String[]{"multi", "", "ple", " ", "strings"},
            new String[]{"Unicode:", " ", "😊", " ", "测试"}
        };

        for (String[] t : tests) {
            System.out.println(concatenate(t));
        }
    }
}