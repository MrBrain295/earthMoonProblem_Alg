#include "candidateBuilder.h"
#include "biplanarSAT.h"
#include "biplanarTester.h"

int main() {
    auto askYesNo = [](const char* prompt) {
        cout << prompt << " (y/n): ";
        char response;
        cin >> response;
        return response == 'y' || response == 'Y';
    };

    /////////////////////////
    // Backtracking vs SAT //
    /////////////////////////
    int n; vector<Edge> edges;

    int a = 5, b = 4;
    n = a * b;
    edges = strongProductEdge(pathGraphEdge(a), a, completeGraphEdge(b), b);
    removeVertexEdges(edges, 0);

    auto start = chrono::high_resolution_clock::now();
    testBiplanarity(&edges, n);
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end - start;
    cout << "Took: " << elapsed.count() << " seconds.\n";

    start = chrono::high_resolution_clock::now();
    if (isBiplanarSAT(edges, n)) {
        cout << "biplanar" << endl;
    }
    end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed2 = end - start;
    cout << "Took: " << elapsed2.count() << " seconds.\n";

    // n = 8;
    // edges = completeGraphEdge(n);

    // auto start = chrono::high_resolution_clock::now();
    // testBiplanarity(&edges, n);
    // auto end = chrono::high_resolution_clock::now();
    // chrono::duration<double> elapsed = end - start;
    // cout << "Took: " << elapsed.count() << " seconds.\n";

    // start = chrono::high_resolution_clock::now();
    // if (isBiplanarSAT(edges, n)) {
    //     cout << "biplanar" << endl;
    // }
    // end = chrono::high_resolution_clock::now();
    // chrono::duration<double> elapsed2 = end - start;
    // cout << "Took: " << elapsed2.count() << " seconds.\n";

    //////////////////////
    // Candidate finder //
    //////////////////////
    // (numVertLow, numVertHigh, numAttempts, bool independenceNumber, bool chromaticNumber)
    cout << "\nCandidate builder can search for biplanar graphs with chromatic number >= 9 or 10.\n";
    cout << "The two final options control whether to use the independence-number heuristic and\n";
    cout << "whether to run the chromatic-number search (slower, may run up to 1000s per attempt).\n";
    if (askYesNo("Would you like to run computeCandidateGraphs now?")) {
        int numVertLow, numVertHigh, numAttempts;
        cout << "Enter lower number of vertices: ";
        cin >> numVertLow;
        cout << "Enter upper number of vertices: ";
        cin >> numVertHigh;
        cout << "Enter number of attempts per vertex count: ";
        cin >> numAttempts;
        cout << "Independence-number heuristic saves graphs with alpha <= n/10 or n/9\n";
        bool useIndependence = askYesNo("Enable independence-number heuristic");
        cout << "Chromatic-number search tries to certify chi >= 10 or 9 (up to 1000s per attempt)\n";
        bool useChromatic = askYesNo("Enable chromatic-number search");
        computeCandidateGraphs(numVertLow, numVertHigh, numAttempts, useIndependence, useChromatic);
    }

    ////////////////////////
    // Blow-up of biwheel //
    ////////////////////////
    // int m = 5;
    // n = m+2; // spokes: 5,6
    // edges = wheelGraphEdge(n, 2);
    // printEdges(edges);
    // cout << endl;
    // edges = blowup(edges, n);
    // printEdges(edges);

    return 0;
}
