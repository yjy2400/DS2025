#include <iostream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cctype>
using namespace std;

/* 顺序栈 */
template <typename T>
class Stack {
public:
    explicit Stack(int sz = 200) {
        elem = new T[sz];
        top  = -1;
        capacity = sz;
    }
    ~Stack() { delete [] elem; }

    bool empty() const { return top == -1; }
    bool full()  const { return top == capacity - 1; }

    void push(const T& x) {
        if (full()) throw "栈溢出";
        elem[++top] = x;
    }
    T pop() {
        if (empty()) throw "栈下溢";
        return elem[top--];
    }
    T& peek() {
        if (empty()) throw "栈空";
        return elem[top];
    }
private:
    T* elem;
    int top;
    int capacity;
};

#define N_OPTR 9
typedef enum {ADD,SUB,MUL,DIV,POW,FAC,L_P,R_P,EOE} Operator;
const char pri[N_OPTR][N_OPTR] = {
/* 栈顶 \ 当前    +   -   *   /   ^   !   (   )   \0 */
/*      + */    {'>','>','<','<','<','<','<','>','>'},
/*      - */    {'>','>','<','<','<','<','<','>','>'},
/*      * */    {'>','>','>','>','<','<','<','>','>'},
/*      / */    {'>','>','>','>','<','<','<','>','>'},
/*      ^ */    {'>','>','>','>','>','<','<','>','>'},
/*      ! */    {'>','>','>','>','>','>',' ','>','>'},
/*      ( */    {'<','<','<','<','<','<','<','=',' '},
/*      ) */    {' ',' ',' ',' ',' ',' ',' ',' ',' '},
/*     \0 */    {'<','<','<','<','<','<','<',' ','='}
};

int op2idx(char c) {
    switch (c) {
    case '+': return ADD;
    case '-': return SUB;
    case '*': return MUL;
    case '/': return DIV;
    case '^': return POW;
    case '!': return FAC;
    case '(': return L_P;
    case ')': return R_P;
    case '\0':
    case '#': return EOE;   // 用 '\0' 或 '#' 都表示结束
    default:  return -1;
    }
}

/* 关系 → 数字 */
int relOp(char r) {
    if (r == '<') return 1;
    if (r == '=') return 2;
    if (r == '>') return 3;
    return 0;          // 错误
}

double calc(double a, char op, double b) {
    switch (op) {
    case '+': return a + b;
    case '-': return a - b;
    case '*': return a * b;
    case '/':
        if (fabs(b) < 1e-12) throw "除零错误";
        return a / b;
    }
    throw "非法双目运算符";
}

double str2double(const string& s) {
    return strtod(s.c_str(), NULL);
}

/* 预处理：去空白 + 全角转半角 */
string preprocess(const string& in) {
    string out;
    for (string::size_type i = 0; i < in.size(); ++i) {
        unsigned char c = in[i];
        /* 全角空格、TAB、换行 一律跳过 */
        if (c == 0xe3 && i + 2 < in.size() &&
            (unsigned char)in[i + 1] == 0x80 &&
            (unsigned char)in[i + 2] == 0x80) { i += 2; continue; }
        if (isspace(c)) continue;
        /* 全角括号转半角 */
        if (c == 0xe3 && i + 2 < in.size()) {
            if ((unsigned char)in[i + 1] == 0x80 && (unsigned char)in[i + 2] == 0x88) {
                out += '('; i += 2; continue;
            }
            if ((unsigned char)in[i + 1] == 0x80 && (unsigned char)in[i + 2] == 0x89) {
                out += ')'; i += 2; continue;
            }
        }
        out += c;
    }
    return out;
}

/* 表达式求值 */
double evaluate(const string& expr);

/* 处理一元函数 */
double callFunc(const string& name, double arg) {
    if (name == "sin")  return sin(arg);
    if (name == "cos")  return cos(arg);
    if (name == "tan")  return tan(arg);
    if (name == "log")  return log10(arg);
    if (name == "ln")   return log(arg);
    if (name == "sqrt") return sqrt(arg);
    throw "未知函数";
}

double evaluate(const string& expr) {
    string src = preprocess(expr);
    Stack<double> opnd;
    Stack<char>   optr;
    optr.push('#');

    string s;
    for (string::size_type i = 0; i < src.size(); ++i) s += src[i];
    s += '#';

    string::size_type i = 0;
    while (i < s.size()) {
        char c = s[i];
        if (isdigit(c) || c == '.') {
            string numStr;
            while (i < s.size() && (isdigit(s[i]) || s[i] == '.'))
                numStr += s[i++];
            opnd.push(str2double(numStr));
            continue;
        }
        /* 处理函数调用 */
        if (isalpha(c)) {
            string fname;
            while (i < s.size() && isalpha(s[i])) fname += s[i++];
            if (s[i] != '(') throw  "函数后必须跟左括号 '('";
            ++i;
            int dep = 1;
            string sub;
            while (i < s.size() && dep) {
                if (s[i] == '(') ++dep;
                else if (s[i] == ')') --dep;
                if (dep) sub += s[i++];
                else break;
            }
            if (dep) throw "函数括号不匹配";
            ++i; // skip ')'
            double arg = evaluate(sub);
            opnd.push(callFunc(fname, arg));
            continue;
        }
        /* 运算符 */
        char topOp = optr.peek();
        int idx1 = op2idx(topOp), idx2 = op2idx(c);
        if (idx1 < 0 || idx2 < 0) throw "非法运算符";
        int rel = relOp(pri[idx1][idx2]);
        if (rel == 0) {
            /* 结束符相遇，正常退出 */
            if (topOp == '#' && c == '#') {
                optr.pop();
                ++i;
                break;
            }
            throw "运算符优先级错误";
        }
        if (rel == 1) {          // <
            optr.push(c);
            ++i;
        } else if (rel == 2) {   // =
            optr.pop();
            ++i;
        } else {                 // >
            char theta = optr.pop();
            double b = opnd.pop();
            double a = opnd.pop();
            opnd.push(calc(a, theta, b));
        }
    }
    double ans = opnd.pop();
    if (!opnd.empty() || !optr.empty()) throw "表达式异常";
    return ans;
}

/*测试 */
int main() {
    const char* tests[] = {
        "1 + 2 * 3",
        "( 1 + 2 ) * 3",
        "3.5 / 2 + 1",
        "sin(0) + cos(0)",
        "ln(2.718281828)",
        "sqrt(16) * 2 + 1",
        "1 + 2 * (3 + 4) / 5 - 6",
        "tan(3.14159265/4)",
        "log(100)",
        "1 + + 2",
        "5 / 0"
    };
    int n = sizeof(tests) / sizeof(tests[0]);
    for (int i = 0; i < n; ++i) {
        cout << "表达式： " << tests[i] << endl;
        try {
            cout << "结果 = " << evaluate(tests[i]) << endl;
        } catch (const char* e) {
            cout << "错误: " << e << "  => 式子无效" << endl;
        }
        cout << endl;
    }

    /* 交互模式 */
    cout << "请输入表达式（空行退出）：" << endl;
    string line;
    while (getline(cin, line)) {
        if (line.empty()) break;
        try {
            cout << "结果 = " << evaluate(line) << endl;
        } catch (const char* e) {
            cout << "错误: " << e << endl;
        }
    }
    return 0;
}
