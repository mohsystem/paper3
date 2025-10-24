public class Task13 {

    public static boolean isValidWalk(char[] walk) {
        if (walk.length != 10) {
            return false;
        }

        int x = 0;
        int y = 0;

        for (char direction : walk) {
            switch (direction) {
                case 'n': y++; break;
                case 's': y--; break;
                case 'e': x++; break;
                case 'w': x--; break;
            }
        }

        return x == 0 && y == 0;
    }

    public static void main(String[] args) {
        char[] walk1 = {'n', 's', 'n', 's', 'n', 's', 'n', 's', 'n', 's'};
        System.out.println("Test 1: " + isValidWalk(walk1));

        char[] walk2 = {'w', 'e', 'w', 'e', 'w', 'e', 'w', 'e', 'w', 'e', 'w', 'e'};
        System.out.println("Test 2: " + isValidWalk(walk2));

        char[] walk3 = {'w'};
        System.out.println("Test 3: " + isValidWalk(walk3));

        char[] walk4 = {'n', 'n', 'n', 's', 'n', 's', 'n', 's', 'n', 's'};
        System.out.println("Test 4: " + isValidWalk(walk4));
        
        char[] walk5 = {'e', 'w', 'e', 'w', 'n', 's', 'n', 's', 'e', 'w'};
        System.out.println("Test 5: " + isValidWalk(walk5));
    }
}