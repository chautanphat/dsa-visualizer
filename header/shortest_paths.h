#ifndef SHORTEST_PATHS_H
#define SHORTEST_PATHS_H

#include "raylib.h"
#include "common.h"
#include <vector>
#include <string>

struct DIJKSTRA
{
    struct Node
    {
        int id;
        int label;
        float x, y;
        Node(int _id = 0, int _label = 0, float _x = 0.0f, float _y = 0.0f) : id(_id), label(_label), x(_x), y(_y) {}
    };

    struct Edge
    {
        int id;
        int u, v;
        int weight;
        int state; // 0 = unexplored, 1 = selected, 2 = accepted, 3 = rejected
    };

    struct Snapshot
    {
        struct EdgeState
        {
            int id;
            int state;
        };

        std::vector<EdgeState> edgeStates;
        std::vector<int> parent;
        std::vector<int> rank;
        int currentEdge;
        int selectedEdge;
        int animMode;
        int edgesAccepted;
        int totalWeight;
        bool dijkstraCompleted;
        std::string statusText;
    };

    std::vector<Node> nodes;
    std::vector<Edge> edges;
    std::vector<int> sortedEdgeIds;

    std::vector<int> parent;
    std::vector<int> rank;

    int currentEdge = -1;
    int selectedEdge = -1;
    int edgesAccepted = 0;
    int totalWeight = 0;

    int animMode = 0;
    float animTimer = 0.0f;
    float animSpeed = 0.8f;
    bool dijkstraCompleted = false;

    std::string statusText;

    int draggingNode = -1;
    Vector2 dragOffset = { 0.0f, 0.0f };

    int mode = 0;

    std::vector<Snapshot> history;

    DIJKSTRA();
    ~DIJKSTRA();

    void clear();
    void drawGraph();
    void randomize();
    bool manualUpload(const std::string &input);
    void startDijkstraAnimation();
    void updateAnimation();
    void captureSnapshot();
    void restoreSnapshot(const Snapshot &sn);

    void calculateNodePositions();
    void rebuildSortedEdges();
    int findSet(int v);
    void unionSets(int a, int b);
};

void runDijkstra(AppState &currentState);

#endif