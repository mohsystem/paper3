
import java.util.*;
import java.nio.charset.StandardCharsets;

public class Task162 {
    private static final int MAX_NODES = 10000;
    private static final int MAX_WEIGHT = 1000000;
    
    static class Edge {
        final int to;
        final int weight;
        
        Edge(int to, int weight) {
            if (to < 0 || weight < 0) {
                throw new IllegalArgumentException("Invalid edge parameters");
            }
            this.to = to;
            this.weight = weight;
        }
    }
    
    static class Node implements Comparable<Node> {
        final int id;
        final int distance;
        
        Node(int id, int distance) {
            this.id = id;
            this.distance = distance;
        }
        
        @Override
        public int compareTo(Node other) {
            return Integer.compare(this.distance, other.distance);
        }
    }
    
    public static Map<Integer, Integer> dijkstra(Map<Integer, List<Edge>> graph, int start, int nodeCount) {
        if (graph == null || nodeCount <= 0 || nodeCount > MAX_NODES) {
            throw new IllegalArgumentException("Invalid graph or node count");
        }
        if (start < 0 || start >= nodeCount) {
            throw new IllegalArgumentException("Invalid start node");
        }
        
        Map<Integer, Integer> distances = new HashMap<>();
        PriorityQueue<Node> pq = new PriorityQueue<>();
        Set<Integer> visited = new HashSet<>();
        
        for (int i = 0; i < nodeCount; i++) {
            distances.put(i, Integer.MAX_VALUE);
        }
        distances.put(start, 0);
        pq.offer(new Node(start, 0));
        
        while (!pq.isEmpty()) {
            Node current = pq.poll();
            
            if (visited.contains(current.id)) {
                continue;
            }
            visited.add(current.id);
            
            List<Edge> neighbors = graph.getOrDefault(current.id, Collections.emptyList());
            if (neighbors.size() > MAX_NODES) {
                throw new IllegalArgumentException("Too many edges");
            }
            
            for (Edge edge : neighbors) {
                if (edge.to < 0 || edge.to >= nodeCount) {
                    throw new IllegalArgumentException("Invalid edge destination");
                }
                if (edge.weight < 0 || edge.weight > MAX_WEIGHT) {
                    throw new IllegalArgumentException("Invalid edge weight");
                }
                
                if (!visited.contains(edge.to)) {
                    int currentDist = distances.get(current.id);
                    if (currentDist != Integer.MAX_VALUE) {
                        long newDist = (long) currentDist + edge.weight;
                        if (newDist > Integer.MAX_VALUE) {
                            newDist = Integer.MAX_VALUE;
                        }
                        
                        if (newDist < distances.get(edge.to)) {
                            distances.put(edge.to, (int) newDist);
                            pq.offer(new Node(edge.to, (int) newDist));
                        }
                    }
                }
            }
        }
        
        return distances;
    }
    
    public static void main(String[] args) {
        try {
            // Test case 1: Simple graph
            Map<Integer, List<Edge>> graph1 = new HashMap<>();
            graph1.put(0, Arrays.asList(new Edge(1, 4), new Edge(2, 1)));
            graph1.put(1, Arrays.asList(new Edge(3, 1)));
            graph1.put(2, Arrays.asList(new Edge(1, 2), new Edge(3, 5)));
            graph1.put(3, new ArrayList<>());
            Map<Integer, Integer> result1 = dijkstra(graph1, 0, 4);
            System.out.println("Test 1 - Distances from node 0: " + result1);
            
            // Test case 2: Single node
            Map<Integer, List<Edge>> graph2 = new HashMap<>();
            graph2.put(0, new ArrayList<>());
            Map<Integer, Integer> result2 = dijkstra(graph2, 0, 1);
            System.out.println("Test 2 - Distances from node 0: " + result2);
            
            // Test case 3: Disconnected graph
            Map<Integer, List<Edge>> graph3 = new HashMap<>();
            graph3.put(0, Arrays.asList(new Edge(1, 1)));
            graph3.put(1, new ArrayList<>());
            graph3.put(2, new ArrayList<>());
            Map<Integer, Integer> result3 = dijkstra(graph3, 0, 3);
            System.out.println("Test 3 - Distances from node 0: " + result3);
            
            // Test case 4: Larger graph
            Map<Integer, List<Edge>> graph4 = new HashMap<>();
            graph4.put(0, Arrays.asList(new Edge(1, 10), new Edge(2, 5)));
            graph4.put(1, Arrays.asList(new Edge(3, 1), new Edge(2, 2)));
            graph4.put(2, Arrays.asList(new Edge(1, 3), new Edge(3, 9), new Edge(4, 2)));
            graph4.put(3, Arrays.asList(new Edge(4, 4)));
            graph4.put(4, new ArrayList<>());
            Map<Integer, Integer> result4 = dijkstra(graph4, 0, 5);
            System.out.println("Test 4 - Distances from node 0: " + result4);
            
            // Test case 5: Different starting node
            Map<Integer, List<Edge>> graph5 = new HashMap<>();
            graph5.put(0, Arrays.asList(new Edge(1, 2)));
            graph5.put(1, Arrays.asList(new Edge(2, 3)));
            graph5.put(2, Arrays.asList(new Edge(0, 1)));
            Map<Integer, Integer> result5 = dijkstra(graph5, 1, 3);
            System.out.println("Test 5 - Distances from node 1: " + result5);
            
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}
