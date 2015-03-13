//=======================================================================
// Copyright 2001 Jeremy G. Siek, Andrew Lumsdaine, Lie-Quan Lee, 
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#include <boost/config.hpp>
#include <iostream>
#include <fstream>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/property_map/property_map.hpp>

using namespace boost;

struct Position
{
    int x;
    int y;

    char name;

    Position(int _x, int _y) :
         x(_x),
         y(_y)
    { };
};

typedef std::vector<Position> GridMap;

int
main(int, char *[])
{
  /* Simulated GridMap. Number are te positions in the vector, used by naming
   * cells in the dijstra results
   */

  /*                    0            1              2            */
  GridMap grid = { Position(1,1), Position(1,2), Position(1,3),
                   Position(2,1), Position(2,2), Position(2,3) };
  /*                    3            4              5            */

  typedef adjacency_list < listS, vecS, directedS, no_property, property < edge_weight_t, int > > graph_t;
  typedef graph_traits < graph_t >::vertex_descriptor vertex_descriptor;
  typedef std::pair<int, int> Edge;

  const int num_nodes = grid.size();

  /* Example edges here are representing:            */
  /*  0 (initial)      1 (obstacle)       2 (free)   */
  /*  3 (free)         4 (free)           5 (fre)    */
  /* ------------------------------------------------ */
  Edge edge_array[] = { Edge(0, 3), Edge(2,5), Edge(3,4), Edge(4,5), Edge(5,2) };

  /* All the weights_v are 1 to move */
  std::vector<int> weights_v(sizeof(edge_array) , 1);
  int num_arcs = sizeof(edge_array) / sizeof(Edge);

  //graph created from the list of edges
  graph_t g(edge_array, edge_array + num_arcs, weights_v.data(), num_nodes);
  //create the property_map from edges to weights
  property_map<graph_t, edge_weight_t>::type weightmap = get(edge_weight, g);
  std::vector<vertex_descriptor> p(num_vertices(g));
  std::vector<int> d(num_vertices(g));

  //create a descriptor for the source node
  vertex_descriptor s = vertex(0, g);

  dijkstra_shortest_paths(g, s,
                          predecessor_map(boost::make_iterator_property_map(p.begin(), get(boost::vertex_index, g))).
                          distance_map(boost::make_iterator_property_map(d.begin(), get(boost::vertex_index, g))));

  std::cout << "distances and parents:" << std::endl;
  graph_traits < graph_t >::vertex_iterator vi, vend;
  for (boost::tie(vi, vend) = vertices(g); vi != vend; ++vi) {
    std::cout << "distance(" << *vi << ") = " << d[*vi] << ", ";
    std::cout << "parent(" <<  *vi << ") = " << p[*vi] << std::
      endl;
  }
  std::cout << std::endl;

  // Print the full path to destination
  // Example: goal is the node number 2
  vertex_descriptor goal = vertex(2, g);
  vertex_descriptor start   = s;
  vertex_descriptor current = goal;

  std::vector<vertex_descriptor> path;
 
  // p is the predecessor map
  while (current != start)
  {
    path.push_back(current);
    current = p[current];
  }

  path.push_back(start);

  // The path ha been stored from the end to the beggining. Using
  // reverse_iterator to get the path from origin to end
  std::cout << "Path from node " << start << " to node " << goal << std::endl;
  std::vector< graph_traits< graph_t >::vertex_descriptor >::reverse_iterator it;
  for (it=path.rbegin(); it != path.rend(); ++it)
  {
        if (it != path.rbegin())
          std::cout << "-> ";

        std::cout << *it << " ";
  }

  std::cout << std::endl;

  // Save file to .dot (graph format files) try open it with nautilus
  std::ofstream dot_file("dijkstra-eg.dot");

  dot_file << "digraph D {\n"
    << "  rankdir=LR\n"
    << "  size=\"4,3\"\n"
    << "  ratio=\"fill\"\n"
    << "  edge[style=\"bold\"]\n" << "  node[shape=\"circle\"]\n";

  graph_traits < graph_t >::edge_iterator ei, ei_end;
  for (boost::tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {
    graph_traits < graph_t >::edge_descriptor e = *ei;
    graph_traits < graph_t >::vertex_descriptor
      u = source(e, g), v = target(e, g);
    dot_file << u << " -> " << v
      << "[label=\"" << get(weightmap, e) << "\"";
    if (p[v] == u)
      dot_file << ", color=\"black\"";
    else
      dot_file << ", color=\"grey\"";
    dot_file << "]";
  }
  dot_file << "}";

  dot_file.close();
  return EXIT_SUCCESS;
}
