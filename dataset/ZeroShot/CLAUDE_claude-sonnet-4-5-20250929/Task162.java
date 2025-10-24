
import java.util.*;

public class Task162 {
    static class Edge {
        int target;
        int weight;
        
        Edge(int target, int weight) {
            this.target = target;
            this.weight = weight;
        }
    }
    
    static class Node implements Comparable<Node> {
        int vertex;
        int distance;
        
        Node(int vertex, int distance) {
            this.vertex = vertex;
            this.distance = distance;
        }
        
        @Override
        public int compareTo(Node other) {
            return Integer.compare(this.distance, other.distance);
        }
    }
    
    public static int[] dijkstra(List<List<Edge>> graph, int start, int n) {
        int[] distances = new int[n];
        Arrays.fill(distances, Integer.MAX_VALUE);
        distances[start] = 0;
        
        PriorityQueue<Node> pq = new PriorityQueue<>();
        pq.offer(new Node(start, 0));
        
        boolean[] visited = new boolean[n];
        
        while (!pq.isEmpty()) {
            Node current = pq.poll();
            int u = current.vertex;
            
            if (visited[u]) continue;
            visited[u] = true;
            
            for (Edge edge : graph.get(u)) {
                int v = edge.target;
                int weight = edge.weight;
                
                if (!visited[v] && distances[u] != Integer.MAX_VALUE 
                    && distances[u] + weight < distances[v]) {
                    distances[v] = distances[u] + weight;
                    pq.offer(new Node(v, distances[v]));
                }
            }
        }
        
        return distances;
    }
    
    public static void main(String[] args) {
        // Test Case 1: Simple graph
        System.out.println("Test Case 1:");
        int n1 = 5;
        List<List<Edge>> graph1 = new ArrayList<>();
        for (int i = 0; i < n1; i++) {
            graph1.add(new ArrayList<>());
        }
        graph1.get(0).add(new Edge(1, 4));
        graph1.get(0).add(new Edge(2, 1));
        graph1.get(1).add(new Edge(3, 1));
        graph1.get(2).add(new Edge(1, 2));
        graph1.get(2).add(new Edge(3, 5));
        graph1.get(3).add(new Edge(4, 3));
        int[] result1 = dijkstra(graph1, 0, n1);
        System.out.println(Arrays.toString(result1));
        
        // Test Case 2: Single node
        System.out.println("\\nTest Case 2:");
        int n2 = 1;
        List<List<Edge>> graph2 = new ArrayList<>();
        graph2.add(new ArrayList<>());
        int[] result2 = dijkstra(graph2, 0, n2);
        System.out.println(Arrays.toString(result2));
        
        // Test Case 3: Disconnected graph
        System.out.println("\\nTest Case 3:");
        int n3 = 4;
        List<List<Edge>> graph3 = new ArrayList<>();
        for (int i = 0; i < n3; i++) {
            graph3.add(new ArrayList<>());
        }
        graph3.get(0).add(new Edge(1, 1));
        graph3.get(1).add(new Edge(0, 1));
        int[] result3 = dijkstra(graph3, 0, n3);
        System.out.println(Arrays.toString(result3));
        
        // Test Case 4: Dense graph
        System.out.println("\\nTest Case 4:");
        int n4 = 4;
        List<List<Edge>> graph4 = new ArrayList<>();
        for (int i = 0; i < n4; i++) {
            graph4.add(new ArrayList<>());
        }
        graph4.get(0).add(new Edge(1, 5));
        graph4.get(0).add(new Edge(2, 10));
        graph4.get(1).add(new Edge(2, 3));
        graph4.get(1).add(new Edge(3, 8));
        graph4.get(2).add(new Edge(3, 2));
        int[] result4 = dijkstra(graph4, 0, n4);
        System.out.println(Arrays.toString(result4));
        
        // Test Case 5: Linear graph
        System.out.println("\\nTest Case 5:");
        int n5 = 6;
        List<List<Edge>> graph5 = new ArrayList<>();
        for (int i = 0; i < n5; i++) {
            graph5.add(new ArrayList<>());
        }
        graph5.get(0).add(new Edge(1, 2));
        graph5.get(1).add(new Edge(2, 3));
        graph5.get(2).add(new Edge(3, 1));
        graph5.get(3).add(new Edge(4, 4));
        graph5.get(4).add(new Edge(5, 2));
        int[] result5 = dijkstra(graph5, 0, n5);
        System.out.println(Arrays.toString(result5));
    }
}
