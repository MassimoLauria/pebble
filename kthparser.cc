#include <iostream>
#include <vector>
#include <cassert>
#include <sstream>
using namespace std;

typedef vector<int> VI;
typedef vector<VI> VVI;

#define FOR(i,l,t) for(int (i)=(l);(i)<(t);++(i))
#define FORE(i,l,t) for(int (i)=(l);(i)<=(t);++(i))
#define FORS(i,v) FOR((i),0,(v).size())

/* Parse a graph in kth format
   in: istream
   g: result, in adjacency list format
   rg: transposed result
 */
void parsekth(istream& in, VVI& g, VVI& rg) {
  int n=0;
  while (!n) {
    string s;
    getline(cin,s);
    if (s.empty() || s[0]=='c') continue;
    stringstream ss(s);
    ss >> n;
    assert(n);
  }
  g = VVI(n);
  rg = VVI(n);
  FOR(i,0,n) {
    string s;
    getline(cin,s);
    stringstream ss(s);
    int u;
    char c;
    ss >> u >> c;
    u--;
    assert (u==i);
    assert (c==':');
    int v;
    while(ss >> v) {
      v--;
      assert(v>=0);
      assert(v<n);
      g[u].push_back(v);
      rg[v].push_back(u);
    }
  }
}

int main() {
  VVI g,rg;
  parsekth(cin,g,rg);
  int n=g.size();
  int m=0;
  FOR (i,0,n) m+= g[i].size();
  printf("Parsed a graph with %d vertexes and %d edges\n", n, m);
}
