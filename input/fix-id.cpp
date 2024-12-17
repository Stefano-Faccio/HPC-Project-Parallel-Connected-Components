#include <iostream>
#include <fstream>
#include <unordered_set>
#include <unordered_map>

using namespace std;

int main() {
    ifstream inputFile("step1-graph.txt");
    ofstream outputFile("step2-graph.txt");

    if (!inputFile.is_open()) {
        cerr << "Errore nell'apertura del file di input" << std::endl;
        return 1;
    }

    if (!outputFile.is_open()) {
        cerr << "Errore nell'apertura del file di output" << std::endl;
        return 1;
    }

    uint64_t numNodi, numArchi;
    inputFile >> numNodi >> numArchi;

    outputFile << numNodi << " " << numArchi << endl;

    unordered_map<uint64_t, uint64_t> nodesID;
    uint64_t id = 0;
    
    uint64_t iter = 0;
    while (!inputFile.eof()) {
        uint64_t from, formID, to, toID;
        inputFile >> from >> to;

        auto search = nodesID.find(from);
        if (search == nodesID.end()) {
            formID = id;
            nodesID[from] = formID;
            id++;
        }
        else
            formID = search->second;

        search = nodesID.find(to);
        if (search == nodesID.end()) {
            toID = id;
            nodesID[to] = toID;
            id++;
        }
        else
            toID = nodesID[to];

        iter++;
        outputFile << formID << " " << toID << "\n";
    }

    inputFile.close();
    outputFile.close();

    return 0;
}