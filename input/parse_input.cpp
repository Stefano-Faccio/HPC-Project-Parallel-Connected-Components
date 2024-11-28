#include "parse_input.hpp"

vector<vector<int>> parse_input(string filename)
{
    //Open the file
    ifstream in(filename);
    if (in.is_open())
    {
        // Read the number of vertices and edges
        int nVertices, nEdges;
        in >> nVertices >> nEdges;

        // Create the graph
        vector<vector<int>> graph(nVertices, vector<int>());

        // Read the edges
        int from, to;
        for (int i = 0; i < nEdges; i++)
        {
            in >> from >> to;
            graph[from].push_back(to);
            //Undirected graph
            graph[to].push_back(from);
        }

        // Close the file
        in.close();

        return graph;
    }
    
    // If the file is not open, print an error message and exit the program
    exit(1);
}

void print_graph(vector<vector<int>> graph)
{
    for (int i = 0; i < (int)graph.size(); i++)
    {
        cout << i << ": ";
        for (int j = 0; j < (int)graph[i].size(); j++)
        {
            cout << graph[i][j] << " ";
        }
        cout << endl;
    }
}