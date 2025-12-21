#include "candidateBuilder.h"
#include <algorithm>
#include <atomic>
#include <mutex>
#include <random>
#include <thread>

/// Returns a *random* maximal planar graph on n vertices.
Graph buildMaximalPlanarGraph(int n, const Graph* avoidGraph) {
    // we start with a simple cycle on n vertices and add edges to it
    Graph G = cycleGraph(n);

    set<Edge> edgesToAvoid;
    if (avoidGraph) {
        for (auto e : make_iterator_range(edges(*avoidGraph))) {
            int u = source(e, *avoidGraph);
            int v = target(e, *avoidGraph);
            if (u > v) swap(u, v);
            edgesToAvoid.insert({u, v});
        }
    }

    // We'll attempt all possible edges in random order, adding them if planarity holds.
    vector<pair<int,int>> allPossible;
    allPossible.reserve(n*(n-1)/2);
    for (int i = 0; i < n; ++i) {
        for (int j = i+1; j < n; ++j) {
            // skip edges already in the cycle, or in the avoidGraph
            if (!edge(i, j, G).second && (!avoidGraph || edgesToAvoid.count({i, j}) == 0))
                allPossible.push_back({i, j});
        }
    }

    // Shuffle edges
    thread_local mt19937 rng{ random_device{}() };
    shuffle(allPossible.begin(), allPossible.end(), rng);

    // Try adding each edge, keep it if the graph stays planar
    for (auto& e : allPossible) {
        if (canAddEdgePlanar(G, e.first, e.second)) {
            add_edge(e.first, e.second, G);
        }
    }

    return G;
}

/// Function that computes for i=[numVertLow], ...,[numVertHigh] ([numVertLow] \leq [numAttempts])
/// biplanar graphs on i vertices, and determines if they are candidates for high chromatic
/// number (\geq 9 or \geq 10).
void computeCandidateGraphs(int numVertLow, int numVertHigh, int numAttempts, bool ind, bool chr) {
    constexpr unsigned int defaultThreadCount = 2u;
    unsigned int detectedThreads = std::thread::hardware_concurrency();
    unsigned int threadCount = detectedThreads ? detectedThreads : defaultThreadCount; // small fallback keeps some parallelism when detection fails
    constexpr int progressFrequencyDivisor = 100;
    constexpr int minAttemptsForParallelization = 4;
    for (int n = numVertLow; n <= numVertHigh; n++) {
        atomic<int> nextAttempt{0};
        atomic<int> completed{0};
        mutex progressMutex;
        mutex saveMutex;
        const int reportInterval = max(1, numAttempts / progressFrequencyDivisor);
        atomic<int> nextReport{reportInterval};
        {
            lock_guard<mutex> lock(progressMutex);
            printProgressBar(0, numAttempts, 
                             "Iteration i = 0/" + to_string(numAttempts) 
                             + ", num of vertices = " + to_string(n) + "/" + to_string(numVertHigh) + ": ");
        }

        unsigned int maxWorkers = std::min(threadCount, static_cast<unsigned int>(numAttempts));
        int workerCount = numAttempts >= minAttemptsForParallelization ? static_cast<int>(maxWorkers) : 1;
        vector<thread> workers;
        workers.reserve(workerCount);

        // Captures shared counters and mutexes by reference; all live until workers are joined below.
        auto worker = [&]() {
            while (true) {
                int i = nextAttempt.fetch_add(1);
                if (i >= numAttempts) break;

                // build max planar graphs on n vertices
                Graph g1 = buildMaximalPlanarGraph(n);
                // build max planar graphs on n vertices avoiding first graph
                Graph g2 = buildMaximalPlanarGraph(n, &g1);
                // take their graph union
                Graph g = graphUnion(g1, g2);

                // save graph if chromatic number 
                // (possibly) \geq 10 or 9
                // we use: 
                //     \chi \geq n/(\alpha) \geq 10,9 
                //                <=> 
                //          \alpha \leq n/10, n/9
                //    OR
                //     \chi \geq 10,9 or over 1000 seconds 
                //     pass while coloring with 9,8 colors.
                if (ind) {
                    if (independenceNumberAtMost(g, n/10)) {
                        lock_guard<mutex> saveLock(saveMutex);
                        saveCandidateGraph(g, g1, g2, "ind", i, n, 10);
                    } else if (independenceNumberAtMost(g, n/9)) {
                        lock_guard<mutex> saveLock(saveMutex);
                        saveCandidateGraph(g, g1, g2, "ind", i, n, 9);
                    }
                }
                if (chr) {
                    // wait 1000s ~ 16.6 minutes max.
                    if (chromaticNumberAtLeast(g, 10, true, 1000)) {
                        lock_guard<mutex> saveLock(saveMutex);
                        saveCandidateGraph(g, g1, g2, "chr", i, n, 10);
                    } else if (chromaticNumberAtLeast(g, 9, true, 1000)) {
                        lock_guard<mutex> saveLock(saveMutex);
                        saveCandidateGraph(g, g1, g2, "chr", i, n, 9);
                    }
                }

                int done = completed.fetch_add(1) + 1;
                bool shouldReport = (done == numAttempts) || (done == 1);
                if (!shouldReport && reportInterval > 1) {
                    int target = nextReport.load();
                    while (done >= target) {
                        if (nextReport.compare_exchange_weak(target, target + reportInterval)) {
                            shouldReport = true;
                            break;
                        }
                    }
                }

                if (shouldReport) {
                    lock_guard<mutex> lock(progressMutex);
                    printProgressBar(done, numAttempts, 
                                     "Iteration i = " + to_string(done) + "/" + to_string(numAttempts) 
                                     + ", num of vertices = " + to_string(n) + "/" + to_string(numVertHigh) + ": ");
                }
            }
        };

        for (int t = 0; t < workerCount; ++t) {
            workers.emplace_back(worker);
        }
        for (auto& w : workers) {
            w.join();
        }
    }
}

/// Saves candidate graph [g] and partitions [g1],[g2] at `data/candidates/{txt}{x}/graph_{i}_{n}.txt`
void saveCandidateGraph(Graph g, Graph g1, Graph g2, string txt, int i, int n, int c) {
    string s = txt + to_string(c) + "/" + to_string(n) + "_" + to_string(i) + "_graph";
    outputGraph(g, "candidates/" + s);
    string t = txt + to_string(c) + "/" + to_string(n) + "_" + to_string(i) + "_partitions";
    outputPartitions(g1, g2, "candidates/" + t);
}
