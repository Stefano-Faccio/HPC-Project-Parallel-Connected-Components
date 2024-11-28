#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <deque>    
#include <list>         
#include <queue>   

using namespace std;

pair<int, vector<int>> cc_bfs(vector<vector<int>> graph);
void bfs(vector<vector<int>> graph, int start, vector<int>& label, int cc_count);