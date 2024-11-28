#include "dfs.hpp"

pair<int, vector<int>> cc_dfs(vector<vector<int>> graph)
{
	vector<int> label(graph.size(), __INT32_MAX__);
	int cc_count = 0;

    for(int i = 0; i < (int)graph.size(); i++)
	{
		if(label[i] == __INT32_MAX__)
		{
			dfs(graph, i, label, cc_count);
            cc_count++;
		}
	}
    return make_pair(cc_count, label);
}

void dfs(vector<vector<int>> graph, int node, vector<int>& label, int cc_count)
{
    label[node] = cc_count;

    for(int i = 0; i < (int)graph[node].size(); i++)
    {
        if(label[graph[node][i]] == __INT32_MAX__)
        {
            dfs(graph, graph[node][i], label, cc_count);
        }
    }
}