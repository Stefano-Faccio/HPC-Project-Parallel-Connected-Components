#pragma once

#include <vector>
#include <utility>
#include <iostream>
#include <string>
#include <fstream>

using namespace std;

vector<vector<int>> parse_input(string filename);
void print_graph(vector<vector<int>> graph);