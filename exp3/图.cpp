#include <iostream>
#include <vector>
#include <queue>
#include <stack>
#include <map>
#include <set>
#include <algorithm>
#include <iomanip>
#include <sstream>
using namespace std;

const int INF = 0x3f3f3f3f;

/* 图类 */
class Graph {
public:
    int n;
    vector<string> id2name;
    map<string, int> name2id;
    vector< vector< pair<int, int> > > adj;

    Graph() : n(0) {}

    void addEdge(const string& u, const string& v, int w = 1) {
        if (name2id.find(u) == name2id.end()) {
            name2id[u] = n++;
            id2name.push_back(u);
        }
        if (name2id.find(v) == name2id.end()) {
            name2id[v] = n++;
            id2name.push_back(v);
        }
        int a = name2id[u], b = name2id[v];
        adj.resize(n);
        adj[a].push_back(make_pair(b, w));
        adj[b].push_back(make_pair(a, w));
    }

    /* 邻接矩阵 */
    void printAdjMatrix() const {
        vector< vector<int> > mat(n, vector<int>(n, 0));
        for (int u = 0; u < n; ++u)
            for (vector< pair<int,int> >::const_iterator it = adj[u].begin(); it != adj[u].end(); ++it)
                mat[u][it->first] = it->second;

        cout << "---- 邻接矩阵 ----\n     ";
        for (int i = 0; i < n; ++i) cout << setw(3) << id2name[i];
        cout << "\n";
        for (int i = 0; i < n; ++i) {
            cout << setw(3) << id2name[i] << " ";
            for (int j = 0; j < n; ++j) cout << setw(3) << mat[i][j];
            cout << "\n";
        }
    }

    /* BFS */
    vector<int> bfs(int src) const {
        vector<int> order;
        vector<bool> vis(n, false);
        queue<int> q;
        q.push(src);
        vis[src] = true;
        while (!q.empty()) {
            int u = q.front(); q.pop();
            order.push_back(u);
            for (vector< pair<int,int> >::const_iterator it = adj[u].begin(); it != adj[u].end(); ++it)
                if (!vis[it->first]) {
                    vis[it->first] = true;
                    q.push(it->first);
                }
        }
        return order;
    }

    /* DFS */
    vector<int> dfs(int src) const {
        vector<int> order;
        vector<bool> vis(n, false);
        stack<int> st;
        st.push(src);
        vis[src] = true;
        while (!st.empty()) {
            int u = st.top(); st.pop();
            order.push_back(u);
            for (vector< pair<int,int> >::const_reverse_iterator it = adj[u].rbegin(); it != adj[u].rend(); ++it)
                if (!vis[it->first]) {
                    vis[it->first] = true;
                    st.push(it->first);
                }
        }
        return order;
    }

    /* Dijkstra */
    vector<int> dijkstra(int src) const {
        vector<int> dist(n, INF);
        priority_queue< pair<int,int>, vector< pair<int,int> >, greater< pair<int,int> > > pq;
        dist[src] = 0;
        pq.push(make_pair(0, src));
        while (!pq.empty()) {
            pair<int,int> top = pq.top(); pq.pop();
            int d = top.first, u = top.second;
            if (d > dist[u]) continue;
            for (vector< pair<int,int> >::const_iterator it = adj[u].begin(); it != adj[u].end(); ++it) {
                int v = it->first, w = it->second;
                if (dist[v] > dist[u] + w) {
                    dist[v] = dist[u] + w;
                    pq.push(make_pair(dist[v], v));
                }
            }
        }
        return dist;
    }

    /* Prim */
    vector< pair<int,int> > prim(int src = 0) const {
        vector<bool> inMST(n, false);
        vector<int> lowCost(n, INF), parent(n, -1);
        lowCost[src] = 0;
        priority_queue< pair<int,int>, vector< pair<int,int> >, greater< pair<int,int> > > pq;
        pq.push(make_pair(0, src));
        while (!pq.empty()) {
            pair<int,int> top = pq.top(); pq.pop();
            int u = top.second;
            if (inMST[u]) continue;
            inMST[u] = true;
            for (vector< pair<int,int> >::const_iterator it = adj[u].begin(); it != adj[u].end(); ++it) {
                int v = it->first, w = it->second;
                if (!inMST[v] && w < lowCost[v]) {
                    lowCost[v] = w;
                    parent[v] = u;
                    pq.push(make_pair(w, v));
                }
            }
        }
        vector< pair<int,int> > edges;
        for (int v = 0; v < n; ++v)
            if (parent[v] != -1) edges.push_back(make_pair(parent[v], v));
        return edges;
    }

    /* Tarjan BCC + 割点 */
    vector<int> cutVertex;
    vector< vector< pair<int,int> > > bcc;
    void tarjanBCC() {
        cutVertex.clear();
        bcc.clear();
        int idx = 0;
        vector<int> dfn(n, -1), low(n);
        stack< pair<int,int> > stk;
        for (int i = 0; i < n; ++i)
            if (dfn[i] == -1)
                dfsBCC(i, -1, idx, dfn, low, stk);
    }
private:
    void dfsBCC(int u, int fa, int& idx, vector<int>& dfn, vector<int>& low, stack< pair<int,int> >& stk) {
        dfn[u] = low[u] = idx++;
        int child = 0;
        bool isCut = false;
        for (vector< pair<int,int> >::const_iterator it = adj[u].begin(); it != adj[u].end(); ++it) {
            int v = it->first;
            if (v == fa) continue;
            if (dfn[v] == -1) {
                child++;
                stk.push(make_pair(u, v));
                dfsBCC(v, u, idx, dfn, low, stk);
                low[u] = min(low[u], low[v]);
                if ((fa == -1 && child > 1) || (fa != -1 && low[v] >= dfn[u]))
                    isCut = true;
                if (low[v] >= dfn[u]) {
                    vector< pair<int,int> > comp;
                    while (true) {
                        pair<int,int> e = stk.top(); stk.pop();
                        comp.push_back(e);
                        if (e.first == u && e.second == v) break;
                    }
                    bcc.push_back(comp);
                }
            } else if (dfn[v] < dfn[u]) {
                low[u] = min(low[u], dfn[v]);
                stk.push(make_pair(u, v));
            }
        }
        if (isCut) cutVertex.push_back(u);
    }
};

/* 主函数 */
int main() {
    ios::sync_with_stdio(false);
    cin.tie(NULL);

    /* 图1 */
    Graph g1;
    const string nodes[] = {"A", "B", "C", "D", "E", "F", "G", "H"};
	for (int i = 0; i < sizeof(nodes)/sizeof(nodes[0]); ++i) {
		const string& name = nodes[i];
		g1.name2id[name] = g1.n++;
		g1.id2name.push_back(name);
		}
	g1.adj.resize(g1.n);
    // 按题目要求带权边
    g1.addEdge("A","B",4);
    g1.addEdge("A","D",6);
    g1.addEdge("A","G",7);
    g1.addEdge("B","C",12);
    g1.addEdge("C","D",9);
    g1.addEdge("C","E",1);
    g1.addEdge("C","F",2);
    g1.addEdge("C","H",10);
    g1.addEdge("D","E",13);
    g1.addEdge("D","G",2);
    g1.addEdge("E","F",5);
    g1.addEdge("E","G",11);
    g1.addEdge("E","H",8);
    g1.addEdge("F","H",3);
    g1.addEdge("G","H",14);

    /* 邻接矩阵 */
    g1.printAdjMatrix();

    /* BFS / DFS */
    int src1 = g1.name2id["A"];
    vector<int> bfsOrder = g1.bfs(src1);
    vector<int> dfsOrder = g1.dfs(src1);
    cout << "\n---- BFS from A ----\n";
    for (size_t i = 0; i < bfsOrder.size(); ++i) cout << g1.id2name[bfsOrder[i]] << " ";
    cout << "\n---- DFS from A ----\n";
    for (size_t i = 0; i < dfsOrder.size(); ++i) cout << g1.id2name[dfsOrder[i]] << " ";
    cout << "\n";

    /* Dijkstra / Prim */
    vector<int> dist = g1.dijkstra(src1);
    vector< pair<int,int> > mst = g1.prim(src1);
    cout << "\n---- Dijkstra dist from A ----\n";
    for (int i = 0; i < g1.n; ++i) {
        string ds;
        if (dist[i] == INF) ds = "INF";
        else { ostringstream oss; oss << dist[i]; ds = oss.str(); }
        cout << "到 " << g1.id2name[i] << " = " << ds << "\n";
    }
    cout << "---- Prim MST ----\n";
    for (size_t i = 0; i < mst.size(); ++i)
        cout << g1.id2name[mst[i].first] << "-" << g1.id2name[mst[i].second] << "\n";

    /* 图2 */
    Graph g2;
    // 无权边
    const char* edges2[][2] = {
        {"A","B"},{"A","E"},
        {"B","F"},
        {"C","D"},{"C","F"},{"C","H"},
        {"D","H"},
        {"E","F"},{"E","I"},
        {"F","G"},{"F","I"},{"F","J"},{"F","K"},
        {"G","K"},
        {"J","K"},
        {"K","L"}
    };
    for (size_t i = 0; i < sizeof(edges2)/sizeof(edges2[0]); ++i)
        g2.addEdge(edges2[i][0], edges2[i][1]);

    /* BCC + 割点 */
    g2.tarjanBCC();
    cout << "\n---- 图2 割点 ----\n";
    for (size_t i = 0; i < g2.cutVertex.size(); ++i) cout << g2.id2name[g2.cutVertex[i]] << " ";
    cout << "\n---- 图2 双连通分量（边集） ----\n";
    for (size_t i = 0; i < g2.bcc.size(); ++i) {
        cout << "BCC" << i + 1 << ": ";
        for (size_t j = 0; j < g2.bcc[i].size(); ++j)
            cout << "(" << g2.id2name[g2.bcc[i][j].first]
                 << "," << g2.id2name[g2.bcc[i][j].second] << ") ";
        cout << "\n";
    }

    /* 验证不同起点是否一致 */
    cout << "\n---- 验证一致性 ----\n";
    set<string> ref;
    for (size_t i = 0; i < g2.cutVertex.size(); ++i) ref.insert(g2.id2name[g2.cutVertex[i]]);
    bool same = true;
    for (int s = 0; s < g2.n; ++s) {
        Graph tmp = g2;
        tmp.tarjanBCC();
        set<string> now;
        for (size_t i = 0; i < tmp.cutVertex.size(); ++i) now.insert(tmp.id2name[tmp.cutVertex[i]]);
        if (now != ref) same = false;
    }
    cout << "不同起点得到的割点集合是否一致：" << (same ? "是" : "否") << "\n";
    return 0;
}
