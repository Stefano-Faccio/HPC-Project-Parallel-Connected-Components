#pragma once

#include <vector>
#include <utility>

using namespace std;

pair<int, vector<int>> cc_dfs(vector<vector<int>> graph);
void dfs(vector<vector<int>> graph, int node, vector<int>& label, int cc_count);