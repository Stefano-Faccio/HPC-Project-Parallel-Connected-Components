#include "bfs.hpp"

pair<int, vector<int>> cc_bfs(vector<vector<int>> graph)
{
	vector<int> label(graph.size(), __INT32_MAX__);
	int cc_count = 0;

	for(int i = 0; i < (int)graph.size(); i++)
	{
		if(label[i] == __INT32_MAX__)
		{
			bfs(graph, i, label, cc_count);
			cc_count++;
		}
	}

	return make_pair(cc_count, label);
}

void bfs(vector<vector<int>> graph, int start, vector<int>& label, int cc_count)
{
	queue<int> q;
	q.push(start);
	label[start] = cc_count;

	while(!q.empty())
	{
		int node = q.front();
		q.pop();

		for(int i = 0; i < (int)graph[node].size(); i++)
		{
			if(label[graph[node][i]] == __INT32_MAX__)
			{
				q.push(graph[node][i]);
				label[graph[node][i]] = cc_count;
			}
		}
	}
}
