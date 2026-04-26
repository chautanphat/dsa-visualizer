#include "mst.h"
#include "raylib.h"
#include "raymath.h"
#include "raygui.h"
#include "common.h"
#include <vector>
#include <sstream>
#include <map>
#include <numeric>
#include <algorithm>
#include <cmath>

static const float graphCenterX = 900.0f;
static const float graphCenterY = 420.0f;
static const float graphRadius = 320.0f;
static MST myMST;

MST::MST() {}
MST::~MST() {}

void MST::clear()
{
    nodes.clear();
    edges.clear();
    sortedEdgeIds.clear();
    parent.clear();
    rank.clear();
    history.clear();
    currentEdge = -1;
    selectedEdge = -1;
    edgesAccepted = 0;
    totalWeight = 0;
    animMode = 0;
    animTimer = 0.0f;
    animSpeed = 0.8f;
    mstCompleted = false;
    statusText = "Ready.";
}

void MST::calculateNodePositions()
{
    int n = (int)nodes.size();
    if (n == 0) return;

    float radius = graphRadius;
    if (n >= 7) radius -= 40.0f;
    if (n >= 8) radius -= 20.0f;

    for (int i = 0; i < n; i++)
    {
        float angle = 2.0f * PI * i / n;
        nodes[i].x = graphCenterX + cosf(angle) * radius;
        nodes[i].y = graphCenterY + sinf(angle) * radius;
    }

    std::vector<Vector2> disp(n);
    float area = radius * radius * 4.0f;
    float k = sqrtf(area / n) * 1.5;
    float temperature = radius * 0.8f;
    int iterations = 200;

    for (int iter = 0; iter < iterations; iter++)
    {
        std::fill(disp.begin(), disp.end(), Vector2{0.0f, 0.0f});

        for (int i = 0; i < n; i++)
            for (int j = i + 1; j < n; j++)
            {
                Vector2 delta = {nodes[i].x - nodes[j].x, nodes[i].y - nodes[j].y};
                float dist = sqrtf(delta.x * delta.x + delta.y * delta.y) + 0.01f;
                float force = k * k / dist;
                float len = sqrtf(delta.x * delta.x + delta.y * delta.y);
                Vector2 dir = {delta.x / len, delta.y / len};
                disp[i].x += dir.x * force;
                disp[i].y += dir.y * force;
                disp[j].x -= dir.x * force;
                disp[j].y -= dir.y * force;
            }

        for (const Edge &edge : edges)
        {
            Vector2 delta = {nodes[edge.u].x - nodes[edge.v].x, nodes[edge.u].y - nodes[edge.v].y};
            float dist = sqrtf(delta.x * delta.x + delta.y * delta.y) + 0.01f;
            float force = dist * dist / k;
            float len = sqrtf(delta.x * delta.x + delta.y * delta.y);
            Vector2 dir = {delta.x / len, delta.y / len};
            disp[edge.u].x -= dir.x * force;
            disp[edge.u].y -= dir.y * force;
            disp[edge.v].x += dir.x * force;
            disp[edge.v].y += dir.y * force;
        }

        for (int i = 0; i < n; i++)
        {
            Vector2 centerForce = {graphCenterX - nodes[i].x, graphCenterY - nodes[i].y};
            disp[i].x += centerForce.x * 0.01f;
            disp[i].y += centerForce.y * 0.01f;

            float dispLen = sqrtf(disp[i].x * disp[i].x + disp[i].y * disp[i].y);
            if (dispLen > temperature)
            {
                float scale = temperature / dispLen;
                disp[i].x *= scale;
                disp[i].y *= scale;
            }

            nodes[i].x += disp[i].x;
            nodes[i].y += disp[i].y;

            nodes[i].x = Clamp(nodes[i].x, graphCenterX - radius, graphCenterX + radius);
            nodes[i].y = Clamp(nodes[i].y, graphCenterY - radius, graphCenterY + radius);
        }

        temperature *= 0.95f;
    }
}

int MST::findSet(int v)
{
    if (v < 0 || v >= (int)parent.size()) return v;
    if (parent[v] == v) return v;
    return parent[v] = findSet(parent[v]);
}

void MST::unionSets(int a, int b)
{
    a = findSet(a);
    b = findSet(b);
    if (a == b) return;
    if (rank[a] < rank[b]) std::swap(a, b);
    parent[b] = a;
    if (rank[a] == rank[b]) rank[a]++;
}

void MST::rebuildSortedEdges()
{
    sortedEdgeIds.resize(edges.size());
    std::iota(sortedEdgeIds.begin(), sortedEdgeIds.end(), 0);

    std::sort(sortedEdgeIds.begin(), sortedEdgeIds.end(), [this](int a, int b)
    {
        if (edges[a].weight != edges[b].weight) return edges[a].weight < edges[b].weight;
        if (edges[a].u != edges[b].u) return edges[a].u < edges[b].u;
        return edges[a].v < edges[b].v;
    });
}

void MST::randomize()
{
    clear();

    int nodeCount = GetRandomValue(5, 8);
    nodes.reserve(nodeCount);
    for (int i = 0; i < nodeCount; i++)
        nodes.push_back(Node(i, i + 1));

    int nextEdgeId = 0;
    for (int i = 1; i < nodeCount; i++)
    {
        int j = GetRandomValue(0, i - 1);
        edges.push_back({ nextEdgeId++, j, i, GetRandomValue(1, 99), 0 });
    }

    for (int i = 0; i < nodeCount; i++)
        for (int j = i + 1; j < nodeCount; j++)
        {
            bool exists = false;
            for (const auto &e : edges)
                if ((e.u == i && e.v == j) || (e.u == j && e.v == i))
                {
                    exists = true;
                    break;
                }
            if (!exists && GetRandomValue(0, 1))
                edges.push_back({ nextEdgeId++, i, j, GetRandomValue(1, 99), 0 });
        }

    calculateNodePositions();
    rebuildSortedEdges();
    parent.resize(nodeCount);
    std::fill(parent.begin(), parent.end(), 0);
    std::iota(parent.begin(), parent.end(), 0);
    rank.assign(nodeCount, 0);
}

bool MST::manualUpload(const std::string &input)
{
    std::istringstream iss(input);
    std::vector<std::tuple<int, int, int>> parsedEdges;
    int maxNode = 0;

    int u, v, w;
    while (iss >> u >> v >> w)
    {
        if (u == v || w < 1) continue;
        if (u < 1 || v < 1) continue;
        parsedEdges.emplace_back(u, v, w);
        if (u > maxNode) maxNode = u;
        if (v > maxNode) maxNode = v;
    }

    if (parsedEdges.empty()) return false;

    clear();
    nodes.resize(maxNode);
    for (int i = 0; i < maxNode; i++) nodes[i] = Node(i, i + 1);

    int nextId = 0;
    for (auto &[lu, lv, ww] : parsedEdges)
    {
        int uu = lu - 1;
        int vv = lv - 1;
        bool dup = false;
        for (auto &e : edges) if ((e.u == uu && e.v == vv) || (e.u == vv && e.v == uu)) { dup = true; break; }
        if (!dup) edges.push_back({ nextId++, uu, vv, ww, 0 });
    }

    if (edges.empty()) return false;

    calculateNodePositions();
    rebuildSortedEdges();
    parent.resize(nodes.size());
    std::iota(parent.begin(), parent.end(), 0);
    rank.assign(nodes.size(), 0);

    return true;
}

void MST::startMSTAnimation()
{
    if (nodes.empty() || edges.empty()) return;

    history.clear();
    for (Edge &edge : edges)
        edge.state = 0;

    parent.resize(nodes.size());
    rank.assign(nodes.size(), 0);
    for (int i = 0; i < (int)nodes.size(); i++)
        parent[i] = i;

    currentEdge = 0;
    selectedEdge = -1;
    edgesAccepted = 0;
    totalWeight = 0;
    animMode = 1;
    mstCompleted = false;
    statusText = "Ready.";

    if (mode == 1)
        animSpeed = 999999.0f;
    else
        animSpeed = 0.8f;
}

void MST::updateAnimation()
{
    if (animMode == 0 || nodes.empty()) return;

    animTimer += GetFrameTime();
    if (animTimer < animSpeed) return;

    captureSnapshot();
    animTimer = 0.0f;

    if (mode == 0) animSpeed = 0.8f;
    else animSpeed = 999999.0f;

    if (animMode == 1)
    {
        selectedEdge = sortedEdgeIds[currentEdge];
        edges[selectedEdge].state = 1;
        const Edge &edge = edges[selectedEdge];
        statusText = TextFormat("Check %d-%d (%d).", nodes[edge.u].label, nodes[edge.v].label, edge.weight);
        animMode = 2;
    }
    else if (animMode == 2)
    {
        auto &e = edges[selectedEdge];
        int ur = findSet(e.u), vr = findSet(e.v);
        if (ur != vr)
        {
            e.state = 2;
            unionSets(ur, vr);
            edgesAccepted++;
            totalWeight += e.weight;
            statusText = TextFormat("Accept %d-%d.", nodes[e.u].label, nodes[e.v].label);
        }
        else
        {
            e.state = 3;
            statusText = TextFormat("Reject %d-%d. Cycle.", nodes[e.u].label, nodes[e.v].label);
        }

        currentEdge++;
        selectedEdge = -1;

        if (edgesAccepted >= (int)nodes.size() - 1 || currentEdge >= (int)sortedEdgeIds.size())
        {
            animMode = 0;
            mstCompleted = true;
            for (auto &e : edges) if (e.state == 0) e.state = 3;
            statusText = TextFormat("MST complete.");
        } else animMode = 1;
    }
}

void MST::captureSnapshot()
{
    Snapshot sn;
    sn.currentEdge = currentEdge;
    sn.selectedEdge = selectedEdge;
    sn.animMode = animMode;
    sn.edgesAccepted = edgesAccepted;
    sn.totalWeight = totalWeight;
    sn.mstCompleted = mstCompleted;
    sn.statusText = statusText;
    sn.parent = parent;
    sn.rank = rank;

    for (const Edge &edge : edges)
        sn.edgeStates.push_back({ edge.id, edge.state });

    history.push_back(sn);
}

void MST::restoreSnapshot(const Snapshot &sn)
{
    currentEdge = sn.currentEdge;
    selectedEdge = sn.selectedEdge;
    animMode = sn.animMode;
    edgesAccepted = sn.edgesAccepted;
    totalWeight = sn.totalWeight;
    mstCompleted = sn.mstCompleted;
    statusText = sn.statusText;
    parent = sn.parent;
    rank = sn.rank;

    for (const Snapshot::EdgeState &edgeState : sn.edgeStates)
    {
        if (edgeState.id >= 0 && edgeState.id < (int)edges.size())
            edges[edgeState.id].state = edgeState.state;
    }

    if (mode == 1)
        animSpeed = 999999.0f;
    else
        animSpeed = 0.8f;
}

static void DrawForwardButton(float x, float y, MST &mst)
{
    GuiSetState(STATE_NORMAL);
    if (mst.mode != 1 || mst.animMode == 0) GuiSetState(STATE_DISABLED);
    if (GuiButton((Rectangle){ x, y, 120, 30 }, "Forward >"))
        mst.animSpeed = 0.0f;
    GuiSetState(STATE_NORMAL);
}

static void DrawBackwardButton(float x, float y, MST &mst)
{
    if (mst.mode != 1 || mst.history.empty()) GuiSetState(STATE_DISABLED);
    if (GuiButton((Rectangle){ x, y, 120, 30 }, "< Backward"))
    {
        MST::Snapshot lastState = mst.history.back();
        mst.history.pop_back();
        mst.restoreSnapshot(lastState);
        if (mst.mode == 1) mst.animSpeed = 999999.0f;
    }
    GuiSetState(STATE_NORMAL);
}

static void DrawToggle(float x, float y, MST &mst)
{
    int oldMode = mst.mode;
    makeGuiLabel(x + 45, y - 30, "Animation Mode:");
    GuiToggleGroup((Rectangle){ x, y, 130, 30 }, "Run-at-once;Step-by-step", &mst.mode);

    if (mst.mode != oldMode)
    {
        if (mst.mode == 1) mst.animSpeed = 999999.0f;
        else mst.animSpeed = 0.8f;
    }
}

static void DrawInitPanel(float x, float y, MST &mst, char *inputBuf, bool &editMode)
{
    static Vector2 inputScroll = { 0.0f, 0.0f };
    DrawRectangleLinesEx((Rectangle){ x - 20, y - 25, 340, 330 }, 1, BLACK);
    makeGuiLabel(x, y, "Initialize Graph");

    if (GuiButton((Rectangle){ x, y + 35, 145, 35 }, "Random"))
        mst.randomize();

    if (GuiButton((Rectangle){ x + 155, y + 35, 145, 35 }, "Upload"))
    {
        // Upload support can be added later.
    }

    if (GuiButton((Rectangle){ x, y + 90, 300, 35 }, "Manual"))
        mst.manualUpload(inputBuf);

    makeGuiLabel(x, y + 145, "Edges (u, v w):");
    DrawMultiLineEditor((Rectangle){ x, y + 175, 300, 110 }, inputBuf, 2048, editMode, inputScroll);
}

static void DrawOperationPanel(float x, float y, MST &mst)
{
    DrawRectangleLinesEx((Rectangle){ x - 20, y - 25, 340, 180 }, 1, BLACK);
    makeGuiLabel(x, y, "Operations");

    int curState = GuiGetState();
    if (mst.nodes.empty() || mst.edges.empty()) GuiSetState(STATE_DISABLED);
    if (GuiButton((Rectangle){ x, y + 35, 300, 35 }, "Generate MST"))
        mst.startMSTAnimation();
    GuiSetState(curState);

    if (GuiButton((Rectangle){ x, y + 90, 300, 35 }, "Clear"))
        mst.clear();
}

static void DrawStatusPanel(float x, float y, MST &mst)
{
    DrawText(mst.statusText.c_str(), (int)x, (int)y, 20, BLACK);
    DrawText(TextFormat("Total sum: %d", mst.totalWeight), (int)x, (int)(y + 25), 20, GREEN);
}

void MST::drawGraph()
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
                break;
            }
        }

    if (draggingNode >= 0 && IsMouseButtonDown(MOUSE_LEFT_BUTTON))
    {
        nodes[draggingNode].x = mouse.x + dragOffset.x;
        nodes[draggingNode].y = mouse.y + dragOffset.y;
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) draggingNode = -1;

    int ha = -1, hb = -1;
    for (auto &e : edges) if (e.state == 1) { ha = e.u; hb = e.v; break; }

    for (auto &e : edges)
    {
        if (e.u < 0 || e.v >= (int)nodes.size()) continue;
        Vector2 a = {nodes[e.u].x, nodes[e.u].y}, b = {nodes[e.v].x, nodes[e.v].y};
        Color c = BLACK;
        float th = 2.0f;
        unsigned char al = 255;

        if (e.state == 0) { if (animMode || mstCompleted) { c = DARKGRAY; al = 100; } }
        else if (e.state == 1) { c = ORANGE; th = 3.0f; }
        else if (e.state == 2) { c = GREEN; th = 3.0f; }
        else if (e.state == 3) { c = DARKGRAY; al = 100; }

        c.a = al;
        DrawLineEx(a, b, th, c);

        Vector2 mid = {(a.x + b.x) * 0.5f, (a.y + b.y) * 0.5f};
        Vector2 diff = {b.x - a.x, b.y - a.y};
        float len = sqrtf(diff.x * diff.x + diff.y * diff.y) + 0.01f;
        Vector2 perp = {-diff.y / len, diff.x / len};
        Vector2 textPos = {mid.x + perp.x * 18.0f, mid.y + perp.y * 18.0f};
        DrawText(TextFormat("%d", e.weight), textPos.x - 10, textPos.y - 10, 20, BLACK);
    }

    for (auto &n : nodes)
    {
        Vector2 p = {n.x, n.y};
        if (n.id == ha || n.id == hb) DrawCircleV(p, 35, ORANGE);
        DrawNode(p, n.label, 20, 30, 4);
    }
}

void runMST(AppState &currentState)
{
    static char inputBuffer[2048] = "1 2 10\n2 3 5\n1 3 15";
    static bool editMode = false;
    const float statusX = 1400.0f;
    const float statusY = 40.0f;

    ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
    myMST.drawGraph();

    float X = 60, Y = 150;
    DrawForwardButton(875, 800, myMST);
    DrawBackwardButton(725, 800, myMST);

    bool isBusy = (myMST.animMode != 0);
    if (isBusy) GuiSetState(STATE_DISABLED);
    DrawToggle(X, Y, myMST);
    DrawInitPanel(X, Y + 125, myMST, inputBuffer, editMode);
    DrawOperationPanel(X, Y + 500, myMST);
    DrawStatusPanel(statusX, statusY, myMST);
    
    if (!isBusy) GuiSetState(STATE_NORMAL);
}