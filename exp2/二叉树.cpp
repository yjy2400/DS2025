#include <cstdio>
#include <cstring>
#include <string>
#include <queue>
#include <vector>
#include <algorithm>
#include <iostream>
using namespace std;

typedef unsigned int Rank;
class Bitmap {             //位图 Bitmap 类
private:
    unsigned char* M;
    Rank N, _sz;           //位图空间 M[]，N*sizeof(char)*8 个比特中含_sz 个有效位
protected:
    void init(Rank n) {
        M = new unsigned char[N = (n + 7) / 8];
        memset(M, 0, N);
        _sz = 0;
    }
    void expand(Rank k) const {          //若被访问的 Bitmap[k]已出界，则需扩容
        if (k < 8 * N) return;          //仍在界内，无需扩容
        Bitmap* self = const_cast<Bitmap*>(this);
        Rank oldN = N;
        unsigned char* oldM = M;
        self->init(2 * k);             //与向量类似，加倍策略
        memcpy(self->M, oldM, oldN);
        delete[] oldM;                 //原数据转移至新空间
    }
public:
    Bitmap(Rank n = 8) { init(n); }         //按指定容量创建位图（为测试暂时选用较小的默认值）
    Bitmap( char* file, Rank n = 8 ) {     //按指定或默认规模，从指定文件中读取位图
	    init( n );
		FILE* fp = fopen( file, "r" ); fread( M, sizeof( char ), N, fp ); fclose( fp );
		for ( Rank k = 0, _sz = 0; k < n; k++ ) _sz += test(k);
	}
    ~Bitmap() { delete[] M; M = NULL; _sz = 0; }       //析构时释放位图空间
    Rank size() const { return _sz; }
    void set(Rank k) { expand(k); _sz++; M[k >> 3] |= (0x80 >> (k & 0x07)); }
    void clear(Rank k) { expand(k); _sz--; M[k >> 3] &= ~(0x80 >> (k & 0x07)); }
    bool test(Rank k) const { expand(k); return M[k >> 3] & (0x80 >> (k & 0x07)); }
    void dump(char* file) {              //将位图整体导出至指定的文件，以便对此后的新位图批量初始化
        FILE* fp = fopen(file, "w");
        fwrite(M, sizeof(char), N, fp);
        fclose(fp);
    }
    char* bits2string(Rank n) {          //将前 n 位转换为字符串
        expand(n - 1);                  //此时可能被访问的最高位为 bitmap[n - 1]
        char* s = new char[n + 1];s[n] = '\0';          //字符串所占空间，由上层调用者负责释放
        for (Rank i = 0; i < n; i++) s[i] = test(i) ? '1' : '0';
        return s;     //返回字符串位置
    }
};

/* 二叉树 BinTree */
struct BinNode {
    char ch;
    int freq;
    BinNode *left, *right;
    BinNode(char c = 0, int f = 0)
        : ch(c), freq(f), left(NULL), right(NULL) {}
};

struct BinTree {
    static void destroy(BinNode* x) {
        if (!x) return;
        destroy(x->left);
        destroy(x->right);
        delete x;
    }
};

/* HuffTree */
struct HuffTree {
    struct Cmp {
        bool operator()(const BinNode* a, const BinNode* b) const {
            return a->freq > b->freq;
        }
    };
    static BinNode* build(const vector<int>& freq) {
        priority_queue<BinNode*, vector<BinNode*>, Cmp> pq;
        for (int i = 0; i < 26; i++)
            if (freq[i])
                pq.push(new BinNode('a' + i, freq[i]));
        if (pq.empty()) return NULL;
        while (pq.size() > 1) {
            BinNode *l = pq.top(); pq.pop();
            BinNode *r = pq.top(); pq.pop();
            BinNode *m = new BinNode(0, l->freq + r->freq);
            m->left = l; m->right = r;
            pq.push(m);
        }
        return pq.top();
    }
};

/* 基借助 Bitmap 的 Huffman 编码串 HuffCode */
class HuffCode {
    Bitmap bm;
    int len;
public:
    HuffCode() : len(0) {}
    void append(bool bit) {
        if (bit) bm.set(len); else bm.clear(len);
        ++len;
    }
    string str() const {
        string s(len, '0');
        for (int i = 0; i < len; i++)
            if (bm.test(i)) s[i] = '1';
        return s;
    }
};

/* 生成编码表 */
void buildCodeMap(BinNode* x, string prefix, vector<string>& map) {
    if (!x) return;
    if (x->ch) { map[x->ch - 'a'] = prefix; return; }
    buildCodeMap(x->left,  prefix + "0", map);
    buildCodeMap(x->right, prefix + "1", map);
}

/* 编码一个单词 */
string encodeWord(const string& word, const vector<string>& codeMap) {
    HuffCode hc;
    for (string::size_type i = 0; i < word.size(); i++) {
        char c = word[i];
        if (c >= 'A' && c <= 'Z') c += 32;
        if (c >= 'a' && c <= 'z') {
            const string& bits = codeMap[c - 'a'];
            for (string::size_type j = 0; j < bits.size(); j++)
                hc.append(bits[j] == '1');
        }
    }
    return hc.str();
}
/* 主程序 */
int main() {
    const char* raw =
        "I have a dream that one day every valley shall be exalted "
        "and every hill and mountain shall be made low the rough places will be made plain "
        "and the crooked places will be made straight and the glory of the Lord shall be revealed "
        "and all flesh shall see it together";

    vector<int> freq(26, 0);
    for (const char* p = raw; *p; ++p) {
        char c = *p;
        if (c >= 'A' && c <= 'Z') c += 32;
        if (c >= 'a' && c <= 'z') freq[c - 'a']++;
    }

    BinNode* root = HuffTree::build(freq);
    if (!root) { puts("empty text"); return 0; }

    vector<string> codeMap(26);
    buildCodeMap(root, "", codeMap);

    puts("Huffman code table (a-z):");
    for (int i = 0; i < 26; i++) if (freq[i])
        printf("%c : %s\n", 'a' + i, codeMap[i].c_str());

    string testWords[] = { "dream", "freedom", "equality" };
    for (int i = 0; i < 3; i++)
        cout << "Encode \"" << testWords[i]
             << "\": " << encodeWord(testWords[i], codeMap) << endl;

    BinTree::destroy(root);
    return 0;
}


