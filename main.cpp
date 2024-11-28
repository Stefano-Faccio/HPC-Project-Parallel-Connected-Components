#include "input/parse_input.hpp"
#include "serial_approaches/bfs.hpp"
#include "serial_approaches/dfs.hpp"

#include <chrono>

using namespace std;
using namespace std::chrono;

void test_serial_approces(vector<vector<int>> graph)
{
    //Timer for all approaches
    milliseconds elapsed_ms;
    high_resolution_clock::time_point start, end;

    //Results for all approaches
    pair<int, vector<int>> res;

    //BFS
    start = high_resolution_clock::now();
    res = cc_bfs(graph);
    end = high_resolution_clock::now();
    elapsed_ms = duration_cast<milliseconds>(end - start);
    cout << "BFS cc: " << res.first << "\t" << elapsed_ms.count() << "ms" << endl;

    //DFS
    start = high_resolution_clock::now();
    res = cc_dfs(graph);
    end = high_resolution_clock::now();
    elapsed_ms = duration_cast<milliseconds>(end - start);
    cout << "DFS cc: " << res.first << "\t" << elapsed_ms.count() << "ms" << endl;

    /*
    cout << "Labels: ";
    for(int i = 0; i < res.second.size(); i++)
        cout << res.second[i] << " ";
    cout << endl;
    */
}

int main(int argc, char* argv[])
{
    //Read the input file
    vector<vector<int>> graph = parse_input("input/small.txt");

    //Print the graph
    //print_graph(graph);

    //Find the connected components
    test_serial_approces(graph);

    return 0;
}