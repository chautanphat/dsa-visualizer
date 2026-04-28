#include "shortest_path.h"
#include "raylib.h"
#include "raymath.h"
#include "raygui.h"
#include "common.h"
#include "tinyfiledialogs.h"
#include <vector>
#include <sstream>
#include <tuple>
#include <algorithm>
#include <cmath>
#include <fstream>

const float graphCenterX = 900.0f;
const float graphCenterY = 420.0f;
const float graphRadius = 320.0f;
const float animationSpeed = 0.8f;
const float stepPause = 999999.0f;
const int INF = 1000000000;
static DIJKSTRA myDijkstra;
static CodePanel dijkstraCodePanel;
static const std::vector<std::string> dijkstraCodeLines =
{
    "for each vertex: dist = inf, prev = nil",
    "dist[source] = 0",
    "while there is an unfinished node:",
    "    u = unfinished node with minimum dist",
    "    for each edge (u, v, w):",
    "        if v is finalized: continue",
    "        if dist[u] + w < dist[v]:",
    "            dist[v] = dist[u] + w; prev[v] = u",
    "    finalize u"
};

static int speedActive = 2;
static const float speedValues[] = { 0.25f, 0.5f, 1.0f, 1.5f, 2.0f };

static bool sameEdge(const DIJKSTRA::Edge &edge, int a, int b)
{
    return (edge.u == a && edge.v == b) || (edge.u == b && edge.v == a);
}

static void changeSpeed(DIJKSTRA &dijkstra)
{
    dijkstra.animSpeed = (dijkstra.mode == 1) ? stepPause : animationSpeed;
}

static int findNext(const DIJKSTRA &dijkstra)
{
    int nextNode = -1, bestDistance = INF;
    for (int i = 0; i < (int)dijkstra.nodes.size(); i++)
        if (!dijkstra.finalized[i] && dijkstra.distances[i] < bestDistance)
        {
            bestDistance = dijkstra.distances[i];
            nextNode = i;
        }
    return nextNode;
}

static const char *DistanceText(int distance)
{
    return (distance == INF) ? "inf" : TextFormat("%d", distance);
}

void DIJKSTRA::clear()
{
    nodes.clear(); edges.clear(); distances.clear(); predecessor.clear();
    finalized.clear(); pendingEdges.clear(); history.clear();
    currentEdge = selectedEdge = activeNode = activeLine = draggingNode = sourceNode = -1;
    animMode = 0; animTimer = 0.0f; dijkstraCompleted = false;
    animSpeed = animationSpeed; statusText = "Ready.";
}

void DIJKSTRA::calculateNodePositions()
{
    int n = (int)nodes.size();
    if (n == 0) return;

    float radius = graphRadius - ((n >= 7) ? 40.0f : 0.0f) - ((n >= 8) ? 20.0f : 0.0f);
    for (int i = 0; i < n; i++)
    {
        float angle = 2.0f * PI * i / n;
        nodes[i].x = graphCenterX + cosf(angle) * radius;
        nodes[i].y = graphCenterY + sinf(angle) * radius;
    }

    std::vector<Vector2> disp(n);
    float k = sqrtf(radius * radius * 4.0f / n) * 1.5f;
    float temperature = radius * 0.8f;
    for (int iter = 0; iter < 200; iter++)
    {
        std::fill(disp.begin(), disp.end(), Vector2{0.0f, 0.0f});
        for (int i = 0; i < n; i++)
            for (int j = i + 1; j < n; j++)
            {
                Vector2 delta = {nodes[i].x - nodes[j].x, nodes[i].y - nodes[j].y};
                float dist = sqrtf(delta.x * delta.x + delta.y * delta.y) + 0.01f;
                float len = sqrtf(delta.x * delta.x + delta.y * delta.y);
                Vector2 dir = {delta.x / len, delta.y / len};
                float force = k * k / dist;
                disp[i].x += dir.x * force; disp[i].y += dir.y * force;
                disp[j].x -= dir.x * force; disp[j].y -= dir.y * force;
            }

        for (const Edge &edge : edges)
        {
            Vector2 delta = {nodes[edge.u].x - nodes[edge.v].x, nodes[edge.u].y - nodes[edge.v].y};
            float dist = sqrtf(delta.x * delta.x + delta.y * delta.y) + 0.01f;
            float len = sqrtf(delta.x * delta.x + delta.y * delta.y);
            Vector2 dir = {delta.x / len, delta.y / len};
            float force = dist * dist / k;
            disp[edge.u].x -= dir.x * force; disp[edge.u].y -= dir.y * force;
            disp[edge.v].x += dir.x * force; disp[edge.v].y += dir.y * force;
        }

        for (int i = 0; i < n; i++)
        {
            Vector2 centerForce = {graphCenterX - nodes[i].x, graphCenterY - nodes[i].y};
            disp[i].x += centerForce.x * 0.01f; disp[i].y += centerForce.y * 0.01f;

            float dispLen = sqrtf(disp[i].x * disp[i].x + disp[i].y * disp[i].y);
            if (dispLen > temperature)
            {
                float scale = temperature / dispLen;
                disp[i].x *= scale; disp[i].y *= scale;
            }

            nodes[i].x = Clamp(nodes[i].x + disp[i].x, graphCenterX - radius, graphCenterX + radius);
            nodes[i].y = Clamp(nodes[i].y + disp[i].y, graphCenterY - radius, graphCenterY + radius);
        }
        temperature *= 0.95f;
    }
}

void DIJKSTRA::sync()
{
    for (auto &edge : edges) edge.state = 0;
    for (int node = 0; node < (int)predecessor.size(); node++)
        if (predecessor[node] >= 0)
            for (auto &edge : edges)
                if (sameEdge(edge, node, predecessor[node]))
                {
                    edge.state = 2;
                    break;
                }
    if (selectedEdge >= 0 && selectedEdge < (int)edges.size()) edges[selectedEdge].state = 1;
}

void DIJKSTRA::resetDistances()
{
    distances.assign(nodes.size(), INF);
    predecessor.assign(nodes.size(), -1);
    finalized.assign(nodes.size(), false);
    pendingEdges.clear();
    currentEdge = selectedEdge = activeNode = -1;
    sync();
    activeLine = (sourceNode >= 0 && sourceNode < (int)nodes.size()) ? 1 : 0;
    statusText = (sourceNode >= 0 && sourceNode < (int)nodes.size())
        ? (distances[sourceNode] = 0, TextFormat("Source node: %d", nodes[sourceNode].label))
        : "Click a node to choose\nthe source.";
}

void DIJKSTRA::randomize()
{
    clear();
    int nodeCount = GetRandomValue(5, 8), nextEdgeId = 0;
    nodes.reserve(nodeCount);
    for (int i = 0; i < nodeCount; i++) nodes.push_back(Node(i, i + 1));
    for (int i = 1; i < nodeCount; i++)
        edges.push_back({ nextEdgeId++, GetRandomValue(0, i - 1), i, GetRandomValue(1, 99), 0 });

    for (int i = 0; i < nodeCount; i++)
        for (int j = i + 1; j < nodeCount; j++)
        {
            bool exists = false;
            for (const auto &edge : edges) if (sameEdge(edge, i, j)) { exists = true; break; }
            if (!exists && GetRandomValue(0, 1)) edges.push_back({ nextEdgeId++, i, j, GetRandomValue(1, 99), 0 });
        }

    calculateNodePositions();
    resetDistances();
}

bool DIJKSTRA::manualUpload(const std::string &input)
{
    std::istringstream iss(input);
    std::vector<std::tuple<int, int, int>> parsedEdges;
    int maxNode = 0, u, v, w;

    while (iss >> u >> v >> w)
    {
        if (u == v || w < 1 || u < 1 || v < 1) continue;
        parsedEdges.emplace_back(u, v, w);
        maxNode = std::max(maxNode, std::max(u, v));
    }
    if (parsedEdges.empty()) return false;

    clear();
    nodes.resize(maxNode);
    for (int i = 0; i < maxNode; i++) nodes[i] = Node(i, i + 1);

    int nextId = 0;
    for (const auto &edge : parsedEdges)
    {
        int uu, vv, ww;
        std::tie(uu, vv, ww) = edge;
        uu--, vv--;
        bool dup = false;
        for (const auto &e : edges) if (sameEdge(e, uu, vv)) { dup = true; break; }
        if (!dup) edges.push_back({ nextId++, uu, vv, ww, 0 });
    }
    if (edges.empty()) return false;

    calculateNodePositions();
    resetDistances();
    return true;
}

void DIJKSTRA::startDijkstraAnimation()
{
    if (nodes.empty() || edges.empty()) return;
    history.clear();
    resetDistances();
    animMode = 1;
    activeLine = 2;
    dijkstraCompleted = false;
    changeSpeed(*this);
}

void DIJKSTRA::updateAnimation()
{
    if (animMode == 0 || nodes.empty()) return;
    animTimer += GetFrameTime() * speedValues[speedActive];
    if (animTimer < animSpeed) return;

    captureSnapshot();
    animTimer = 0.0f;
    changeSpeed(*this);

    if (animMode == 1)
    {
        activeNode = findNext(*this);
        if (activeNode < 0)
        {
            animMode = 0;
            dijkstraCompleted = true;
            selectedEdge = activeNode = -1;
            sync();
            activeLine = 8;
            statusText = "Dijkstra complete.";
            return;
        }

        pendingEdges.clear();
        for (int i = 0; i < (int)edges.size(); i++)
            if (edges[i].u == activeNode || edges[i].v == activeNode)
                pendingEdges.push_back(i);
                
        currentEdge = 0;
        selectedEdge = -1;
        sync();
        activeLine = 3;
        statusText = TextFormat("Select node %d.", nodes[activeNode].label);
        animMode = 2;
        return;
    }

    if (animMode == 2)
    {
        if (currentEdge >= (int)pendingEdges.size())
        {
            finalized[activeNode] = true;
            selectedEdge = -1;
            sync();
            activeLine = 8;
            statusText = TextFormat("Finalize node %d.", nodes[activeNode].label);
            animMode = 1;
            activeNode = -1;
            return;
        }

        selectedEdge = pendingEdges[currentEdge];
        sync();
        const Edge &edge = edges[selectedEdge];
        int neighbor = (edge.u == activeNode) ? edge.v : edge.u;
        activeLine = 4;
        statusText = TextFormat("Check %d -> %d (%d).", nodes[activeNode].label, nodes[neighbor].label, edge.weight);
        animMode = 3;
        return;
    }

    const Edge &edge = edges[selectedEdge];
    int neighbor = (edge.u == activeNode) ? edge.v : edge.u;
    if (finalized[neighbor])
    {
        activeLine = 5;
        statusText = TextFormat("Skip %d. Already finalized.", nodes[neighbor].label);
    }
    else if (distances[activeNode] == INF)
    {
        activeLine = 6;
        statusText = TextFormat("Skip %d. Source is unreachable.", nodes[neighbor].label);
    }
    else
    {
        activeLine = 6;
        int candidate = distances[activeNode] + edge.weight;
        if (candidate < distances[neighbor])
        {
            distances[neighbor] = candidate;
            predecessor[neighbor] = activeNode;
            activeLine = 7;
            statusText = TextFormat("Update dist[%d] = %d.", nodes[neighbor].label, candidate);
        }
        else statusText = TextFormat("Keep dist[%d] = %s.", nodes[neighbor].label, DistanceText(distances[neighbor]));
    }

    currentEdge++;
    selectedEdge = -1;
    sync();
    animMode = 2;
}

void DIJKSTRA::captureSnapshot()
{
    Snapshot sn;
    sn.currentEdge = currentEdge;
    sn.selectedEdge = selectedEdge;
    sn.activeNode = activeNode;
    sn.activeLine = activeLine;
    sn.animMode = animMode;
    sn.dijkstraCompleted = dijkstraCompleted;
    sn.statusText = statusText;
    sn.distances = distances;
    sn.predecessor = predecessor;
    sn.finalized = finalized;
    sn.pendingEdges = pendingEdges;
    for (const Edge &edge : edges) sn.edgeStates.push_back({ edge.id, edge.state });
    history.push_back(sn);
}

void DIJKSTRA::restoreSnapshot(const Snapshot &sn)
{
    currentEdge = sn.currentEdge;
    selectedEdge = sn.selectedEdge;
    activeNode = sn.activeNode;
    activeLine = sn.activeLine;
    animMode = sn.animMode;
    dijkstraCompleted = sn.dijkstraCompleted;
    statusText = sn.statusText;
    distances = sn.distances;
    predecessor = sn.predecessor;
    finalized = sn.finalized;
    pendingEdges = sn.pendingEdges;
    for (const Snapshot::EdgeState &edgeState : sn.edgeStates)
        if (edgeState.id >= 0 && edgeState.id < (int)edges.size())
            edges[edgeState.id].state = edgeState.state;
    changeSpeed(*this);
}

static void DrawAnimationControls(float centerX, float y, DIJKSTRA &dijkstra)
{
    float btnW = 60, gap = 10, startX = centerX - (4 * btnW + 3 * gap) / 2.0f;

    GuiSetState((dijkstra.mode == 1 && !dijkstra.history.empty()) ? STATE_NORMAL : STATE_DISABLED);
    if (DrawCustomButton((Rectangle){ startX, y, btnW, 30 }, "#129#"))
    {
        DIJKSTRA::Snapshot firstState = dijkstra.history.front();
        dijkstra.history.clear();
        dijkstra.restoreSnapshot(firstState);
    }

    startX += btnW + gap;
    if (DrawCustomButton((Rectangle){ startX, y, btnW, 30 }, "#130#"))
    {
        DIJKSTRA::Snapshot lastState = dijkstra.history.back();
        dijkstra.history.pop_back();
        dijkstra.restoreSnapshot(lastState);
    }

    startX += btnW + gap;
    GuiSetState((dijkstra.mode == 1 && dijkstra.animMode != 0) ? STATE_NORMAL : STATE_DISABLED);
    if (DrawCustomButton((Rectangle){ startX, y, btnW, 30 }, "#131#")) dijkstra.animSpeed = 0.0f;

    startX += btnW + gap;
    if (DrawCustomButton((Rectangle){ startX, y, btnW, 30 }, "#134#"))
    {
        while (dijkstra.animMode != 0)
        {
            dijkstra.animSpeed = 0.0f;
            dijkstra.animTimer = 99999.0f;
            dijkstra.updateAnimation();
        }
        if (dijkstra.mode == 1) dijkstra.animSpeed = 999999.0f;
    }
    GuiSetState(STATE_NORMAL);
}

static void DrawToggle(float x, float y, DIJKSTRA &dijkstra)
{
    int oldMode = dijkstra.mode;
    makeGuiLabel(x + 45, y - 30, "Animation Mode:");
    GuiToggleGroup((Rectangle){ x, y, 130, 30 }, "Run-at-once;Step-by-step", &dijkstra.mode);
    if (dijkstra.mode != oldMode) changeSpeed(dijkstra);

    makeGuiLabel(120, 25, "Speed:");
    GuiToggleGroup((Rectangle){ 190, 20, 55, 30 }, "0.25x;0.5x;1x;1.5x;2x", &speedActive);
}

static void DrawInitPanel(float x, float y, DIJKSTRA &dijkstra, char *inputBuf, bool &editMode)
{
    static Vector2 inputScroll = { 0.0f, 0.0f };
    DrawRectangleLinesEx((Rectangle){ x - 20, y - 25, 340, 330 }, 1, BLACK);
    makeGuiLabel(x, y, "Initialize Graph");
    if (DrawCustomButton((Rectangle){ x, y + 35, 145, 35 }, "Random")) dijkstra.randomize();
    if (DrawCustomButton((Rectangle){ x + 155, y + 35, 145, 35 }, "Upload"))
    {
        const char* filters[] = { "*.txt" };
        const char* filepath = tinyfd_openFileDialog("Select File", "", 1, filters, "Text Files", 0);
        if (filepath)
        {
            std::ifstream file(filepath);
            if (file.is_open())
            {
                std::stringstream buffer;
                buffer << file.rdbuf();
                dijkstra.manualUpload(buffer.str());
                file.close();
            }
        }
    }
    if (DrawCustomButton((Rectangle){ x, y + 90, 300, 35 }, "Manual")) dijkstra.manualUpload(inputBuf);
    makeGuiLabel(x, y + 145, "Edges (u, v w):");
    DrawMultiLineEditor((Rectangle){ x, y + 175, 300, 110 }, inputBuf, 2048, editMode, inputScroll);
}

static void DrawOperationPanel(float x, float y, DIJKSTRA &dijkstra)
{
    DrawRectangleLinesEx((Rectangle){ x - 20, y - 25, 340, 180 }, 1, BLACK);
    makeGuiLabel(x, y, "Operations");
    int curState = GuiGetState();
    if (dijkstra.nodes.empty() || dijkstra.edges.empty() || dijkstra.sourceNode < 0) GuiSetState(STATE_DISABLED);
    if (DrawCustomButton((Rectangle){ x, y + 35, 300, 35 }, "Start Dijkstra")) dijkstra.startDijkstraAnimation();
    GuiSetState(curState);
    if (DrawCustomButton((Rectangle){ x, y + 90, 300, 35 }, "Clear")) dijkstra.clear();
}

static void DrawStatusPanel(float x, float y, DIJKSTRA &dijkstra)
{
    const char *sourceText = (dijkstra.sourceNode >= 0 && dijkstra.sourceNode < (int)dijkstra.nodes.size())
        ? TextFormat("Selected source: %d", dijkstra.nodes[dijkstra.sourceNode].label)
        : "Selected source: none";
    DrawText(sourceText, (int)x, (int)y, 20, GREEN);
    DrawText(dijkstra.statusText.c_str(), (int)x, (int)(y + 25), 20, BLACK);
    if (dijkstra.activeNode >= 0 && dijkstra.activeNode < (int)dijkstra.nodes.size())
        DrawText(TextFormat("Current node: %d", dijkstra.nodes[dijkstra.activeNode].label), (int)x, (int)(y + 50), 20, ORANGE);
}

void DIJKSTRA::drawGraph()
{
    updateAnimation();

    Vector2 mouse = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        for (int i = (int)nodes.size() - 1; i >= 0; i--)
        {
            Vector2 d = {mouse.x - nodes[i].x, mouse.y - nodes[i].y};
            if (d.x * d.x + d.y * d.y <= 900.0f)
            {
                draggingNode = i;
                dragOffset = {nodes[i].x - mouse.x, nodes[i].y - mouse.y};
                dragStartMouse = mouse;
                break;
            }
        }

    if (draggingNode >= 0 && IsMouseButtonDown(MOUSE_LEFT_BUTTON))
    {
        nodes[draggingNode].x = mouse.x + dragOffset.x;
        nodes[draggingNode].y = mouse.y + dragOffset.y;
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && draggingNode >= 0)
    {
        float dx = mouse.x - dragStartMouse.x, dy = mouse.y - dragStartMouse.y;
        if (dx * dx + dy * dy <= 36.0f && animMode == 0)
        {
            sourceNode = draggingNode;
            resetDistances();
        }
        draggingNode = -1;
    }

    for (const auto &edge : edges)
    {
        if (edge.u < 0 || edge.v >= (int)nodes.size()) continue;
        Vector2 a = {nodes[edge.u].x, nodes[edge.u].y}, b = {nodes[edge.v].x, nodes[edge.v].y};
        Color color = (edge.state == 1) ? ORANGE : (edge.state == 2) ? BLUE : DARKGRAY;
        float thickness = (edge.state == 0) ? 2.5f : 4.0f;
        DrawLineEx(a, b, thickness, color);

        Vector2 mid = {(a.x + b.x) * 0.5f, (a.y + b.y) * 0.5f};
        Vector2 diff = {b.x - a.x, b.y - a.y};
        float len = sqrtf(diff.x * diff.x + diff.y * diff.y) + 0.01f;
        Vector2 perp = {-diff.y / len, diff.x / len};
        Vector2 textPos = {mid.x + perp.x * 18.0f, mid.y + perp.y * 18.0f};
        DrawText(TextFormat("%d", edge.weight), textPos.x - 8, textPos.y - 10, 20, BLACK);
    }

    for (int i = 0; i < (int)nodes.size(); i++)
    {
        Vector2 p = {nodes[i].x, nodes[i].y};
        Color fillColor = WHITE, textColor = BLACK;
        if (finalized.size() == nodes.size() && finalized[i]) { fillColor = DARKGREEN; textColor = WHITE; }
        else if (i == activeNode) fillColor = ORANGE;
        else if (i == sourceNode) { fillColor = RED; textColor = WHITE; }

        DrawCircleV(p, 30, fillColor);
        DrawRing(p, 26, 30, 0.0f, 360.0f, 40, BLACK);
        const char *nodeText = TextFormat("%d", nodes[i].label);
        DrawText(nodeText, p.x - MeasureText(nodeText, 20) / 2, p.y - 10, 20, textColor);

        const char *distText = (i < (int)distances.size()) ? DistanceText(distances[i]) : "INF";
        DrawText(distText, p.x - MeasureText(distText, 18) / 2, p.y - 52, 18, DARKBLUE);
    }
}

void runDijkstra(AppState &currentState)
{
    static char inputBuffer[2048] = "1 2 10\n2 3 5\n1 3 15";
    static bool editMode = false;
    (void)currentState;

    ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
    myDijkstra.drawGraph();

    float x = 60.0f, y = 150.0f;
    DrawAnimationControls(800, 800, myDijkstra);
    DrawCodePanel(dijkstraCodePanel, code_panel, "Dijkstra Pseudocode", dijkstraCodeLines, myDijkstra.activeLine);

    bool isBusy = (myDijkstra.animMode != 0);
    if (isBusy) GuiSetState(STATE_DISABLED);

    DrawToggle(x, y, myDijkstra);
    DrawInitPanel(x, y + 125, myDijkstra, inputBuffer, editMode);
    DrawOperationPanel(x, y + 500, myDijkstra);
    DrawStatusPanel(1350.0f, 40.0f, myDijkstra);

    if (!isBusy) GuiSetState(STATE_NORMAL);
}