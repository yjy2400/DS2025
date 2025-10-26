#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <cstdio>
#include <cmath>
using namespace std;

/* 复数结构体 */
struct Complex {
    double re, im;
    Complex(double r = 0, double i = 0) : re(r), im(i) {}
    double norm() const { return sqrt(re * re + im * im); }         // 模长
    bool operator==(const Complex& rhs) const {       // 相等比较
        return re == rhs.re && im == rhs.im;
    }
    bool operator<(const Complex& rhs) const {        // 字典序比较
        if (re != rhs.re) return re < rhs.re; 
        return im < rhs.im;
    }
    friend ostream& operator<<(ostream& os, const Complex& c) {
        os << "(" << c.re << "," << c.im << ")";
        return os;
    }
};
typedef vector<Complex> CVec;

/* 辅助函数 */
void shuffleVec(CVec& v) {         //随机打乱
    random_shuffle(v.begin(), v.end());  
}
void printVec(const CVec& v, const string& info = "") {
    cout << info << " size=" << v.size() << ":";
    for (size_t i = 0; i < v.size(); ++i) cout << " " << v[i];
    cout << "\n";
}
bool eraseFirst(CVec& v, const Complex& key) {           //删除首个匹配项
    CVec::iterator it = find(v.begin(), v.end(), key);
    if (it == v.end()) return false;
    v.erase(it);
    return true;
}
void uniquify(CVec& v) {           //先排序再去重
    sort(v.begin(), v.end());
    v.erase(unique(v.begin(), v.end()), v.end());
}
bool cmpNorm(const Complex& a, const Complex& b) {         //按模长排序
    double na = a.norm(), nb = b.norm();
    if (na != nb) return na < nb;
    return a.re < b.re;
}

/* 排序算法 */
void bubbleSort(CVec& v) {               //冒泡排序
    for (size_t i = 0; i < v.size(); ++i)
        for (size_t j = 0; j + 1 < v.size() - i; ++j)
            if (!cmpNorm(v[j], v[j + 1])) swap(v[j], v[j + 1]);
}
void mergeSort(CVec& v, int l, int r) {          //归并排序
    if (l >= r) return;
    int m = (l + r) >> 1;
    mergeSort(v, l, m);
    mergeSort(v, m + 1, r);
    CVec tmp;
    int i = l, j = m + 1;
    while (i <= m && j <= r)
        tmp.push_back(cmpNorm(v[i], v[j]) ? v[i++] : v[j++]);
    while (i <= m) tmp.push_back(v[i++]);
    while (j <= r) tmp.push_back(v[j++]);
    for (size_t k = 0; k < tmp.size(); ++k)
        v[l + k] = tmp[k];
}

/* 范围查找 */
CVec rangeFind(const CVec& v, double m1, double m2) {
    CVec res;
    for (size_t i = 0; i < v.size(); ++i) {
        double n = v[i].norm();
        if (n >= m1 && n < m2) res.push_back(v[i]);
    }
    sort(res.begin(), res.end(), cmpNorm);
    return res;
}

/* 计时函数 */
double tick(const CVec& data, void (*f)(CVec&), string name) {
    CVec cp = data;
    clock_t st = clock();
    f(cp);
    double t = (clock() - st) * 1000.0 / CLOCKS_PER_SEC;
    cout << name << "耗时: " << t << " ms\n";
    return t;
}
double tickMerge(const CVec& data) {
    CVec cp = data;
    clock_t st = clock();
    mergeSort(cp, 0, (int)cp.size() - 1);
    double t = (clock() - st) * 1000.0 / CLOCKS_PER_SEC;
    cout << "mergeSort 耗时: " << t << " ms\n";
    return t;
}

/* 主函数 */
int main() {
    const int N = 10000;
    CVec A;
    srand(time(0));
    for (int i = 0; i < N; ++i)
        A.push_back(Complex((rand() % 2000 - 1000) / 100.0, (rand() % 2000 - 1000) / 100.0));
    for (int i = 0; i < 20; ++i) A.push_back(A[i]);

    printVec(CVec(A.begin(), A.begin() + 10), "A");

    shuffleVec(A);
    printVec(CVec(A.begin(), A.begin() + 10), "随机打乱后的A");

    Complex key = A[N / 2];
    cout << "\n查找 key = " << key << "  "
         << (find(A.begin(), A.end(), key) != A.end() ? "找到\n" : "未找到\n");

    Complex ins(3.14, 2.71);
    A.push_back(ins);
    cout << "插入 " << ins << " 后 size = " << A.size() << "\n";

    bool ok = eraseFirst(A, ins);
    cout << "删除 " << ins << (ok ? " 成功" : " 失败") << "，size = " << A.size() << "\n";

    uniquify(A);
    cout << "唯一化后 size = " << A.size() << "\n";

    CVec Test = A;
    cout << "\n===== 排序性能比较 (N=" << Test.size() << ") =====\n";

    sort(Test.begin(), Test.end(), cmpNorm);
    tick(Test, bubbleSort, "顺序 bubbleSort");
    tickMerge(Test);

    shuffleVec(Test);
    tick(Test, bubbleSort, "乱序 bubbleSort");
    tickMerge(Test);

    sort(Test.rbegin(), Test.rend(), cmpNorm);
    tick(Test, bubbleSort, "逆序 bubbleSort");
    tickMerge(Test);

    sort(A.begin(), A.end(), cmpNorm);
    double m1 = 5.0, m2 = 10.0;
    CVec sub = rangeFind(A, m1, m2);
    cout << "\n范围 [" << m1 << "," << m2 << ") 内找到 "
         << sub.size() << " 个元素\n";
    printVec(CVec(sub.begin(), sub.begin() + min(10, (int)sub.size())), "sub");

    return 0;
}
