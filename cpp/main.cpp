#include "candidateBuilder.h"
#include "biplanarSAT.h"
#include "biplanarTester.h"

#include <fstream>
#include <string>
#include <limits>

using namespace std;

static bool loadGraphFromFile(const string& path, vector<Edge>& edges, int& n) {
    ifstream in(path);
    if (!in.is_open()) {
        cerr << "Could not open file: " << path << endl;
        return false;
    }
    int u, v;
    n = 0;
    while (in >> u >> v) {
        edges.emplace_back(u, v);
        n = max(n, max(u, v) + 1);
    }
    return true;
}

int main() {
    cout << "Would you like to run biplanar tester (y/n)? ";
    char choice;
    if (!(cin >> choice)) return 0;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (choice == 'y' || choice == 'Y') {
        cout << "Path to file: ";
        string path;
        getline(cin, path);

        int n; vector<Edge> edges;
        if (!path.empty()) {
            if (!loadGraphFromFile(path, edges, n)) return 1;
        } else {
            int a = 5, b = 4;
            n = a * b;
            edges = strongProductEdge(pathGraphEdge(a), a, completeGraphEdge(b), b);
            removeVertexEdges(edges, 0);
        }

        auto start = chrono::high_resolution_clock::now();
        testBiplanarity(&edges, n);
        auto end = chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = end - start;
        cout << "Took: " << elapsed.count() << " seconds.\n";
        return 0;
    }

    cout << "Would you like to run the candidate buider (y/n)? ";
    char build;
    if (!(cin >> build)) return 0;
    if (build != 'y' && build != 'Y') return 0;

    int low, high, attempts;
    cout << "Enter lower number of vertices: ";
    cin >> low;
    cout << "Enter higher number of vertices: ";
    cin >> high;
    cout << "Enter number of candidates to build (attempts) per ammount of verticie count: ";
    cin >> attempts;

    cout << "Enable independence-number heuristic? (y/n): ";
    char indC; cin >> indC;
    cout << "Enable chromatic-number search? (y/n): ";
    char chrC; cin >> chrC;

    bool ind = (indC == 'y' || indC == 'Y');
    bool chr = (chrC == 'y' || chrC == 'Y');

    computeCandidateGraphs(low, high, attempts, ind, chr);
    return 0;
}
