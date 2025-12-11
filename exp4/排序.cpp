#include <cstdio>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <ctime>
#include <cmath>

/* 基本数据结构 */
struct BBox {                   //存储边界框信息：坐标、置信度、编号、面积计算  
    float x1, y1, x2, y2;
    float score;
    int   id;
    float area() const { return (x2 - x1) * (y2 - y1); }
};

/* 四种排序底层实现 */
void quickSort(std::vector<BBox>& a, int l, int r) {     //快速排序
    if (l >= r) return;
    float pivot = a[l + rand() % (r - l + 1)].score;
    int i = l, j = r;
    while (i <= j) {
        while (a[i].score < pivot) ++i;
        while (a[j].score > pivot) --j;
        if (i <= j) std::swap(a[i++], a[j--]);
    }
    if (l < j) quickSort(a, l, j);
    if (i < r) quickSort(a, i, r);
}

void mergeSort(std::vector<BBox>& a, int l, int r, std::vector<BBox>& tmp) {       //归并排序
    if (l >= r) return;
    int m = (l + r) >> 1;
    mergeSort(a, l, m, tmp);
    mergeSort(a, m + 1, r, tmp);
    int i = l, j = m + 1, k = l;
    while (i <= m && j <= r)
        tmp[k++] = (a[i].score < a[j].score) ? a[i++] : a[j++];
    while (i <= m) tmp[k++] = a[i++];
    while (j <= r) tmp[k++] = a[j++];
    for (i = l; i <= r; ++i) a[i] = tmp[i];
}

struct CmpGreater {
    bool operator()(const BBox& a, const BBox& b) const { return a.score > b.score; }
};
void heapSort(std::vector<BBox>& a) {          //堆排序
    std::make_heap(a.begin(), a.end(), CmpGreater());
    for (int i = (int)a.size(); i > 1; --i)
        std::pop_heap(a.begin(), a.begin() + i, CmpGreater());
}

void shellSort(std::vector<BBox>& a) {        //希尔排序
    const int sedgewick[] = {1391376, 463792, 198768, 86961, 33936,
                             13776, 4592, 1968, 861, 336, 112, 48, 21, 7, 3, 1};
    int n = a.size();
    for (int k = 0; k < 16; ++k) {
        int h = sedgewick[k];
        if (h >= n) continue;
        for (int i = h; i < n; ++i) {
            BBox key = a[i];
            int j = i;
            while (j >= h && a[j - h].score > key.score) {
                a[j] = a[j - h];
                j -= h;
            }
            a[j] = key;
        }
    }
}

/* 包装函数：统一签名 void(std::vector<BBox>&)*/
void quickSortWrap(std::vector<BBox>& a) { quickSort(a, 0, a.size() - 1); }

void mergeSortWrap(std::vector<BBox>& a) {
    static std::vector<BBox> tmp;   // 静态缓存，避免反复分配
    tmp.resize(a.size());
    mergeSort(a, 0, a.size() - 1, tmp);
}

void heapSortWrap(std::vector<BBox>& a) { heapSort(a); }

void shellSortWrap(std::vector<BBox>& a) { shellSort(a); }

/* NMS */
std::vector<BBox> nms(std::vector<BBox> boxes, float iouTh = 0.5f) {
    if (boxes.empty()) return std::vector<BBox>();
    std::sort(boxes.begin(), boxes.end(), CmpGreater());
    std::vector<BBox> keep;
    std::vector<bool> supp(boxes.size(), false);
    for (int i = 0; i < (int)boxes.size(); ++i) {
        if (supp[i]) continue;
        keep.push_back(boxes[i]);
        const BBox& a = boxes[i];
        for (int j = i + 1; j < (int)boxes.size(); ++j) {
            if (supp[j]) continue;
            const BBox& b = boxes[j];
            float xx1 = std::max(a.x1, b.x1), yy1 = std::max(a.y1, b.y1);
            float xx2 = std::min(a.x2, b.x2), yy2 = std::min(a.y2, b.y2);
            float w = std::max(0.0f, xx2 - xx1), h = std::max(0.0f, yy2 - yy1);
            float inter = w * h;
            float iou   = inter / (a.area() + b.area() - inter);
            if (iou > iouTh) supp[j] = true;
        }
    }
    return keep;
}

/* 数据生成器 */
static float frand() { return rand() / (float)RAND_MAX; }

std::vector<BBox> genRandom(int n) {       //随机分布
    std::vector<BBox> b(n);
    for (int i = 0; i < n; ++i) {
        float w = 0.05f + frand() * 0.15f;
        float h = 0.05f + frand() * 0.15f;
        float x1 = frand() * (1 - w), y1 = frand() * (1 - h);
        b[i].x1 = x1; b[i].y1 = y1; b[i].x2 = x1 + w; b[i].y2 = y1 + h;
        b[i].score = frand(); b[i].id = i;
    }
    return b;
}

std::vector<BBox> genCluster(int n) {      //聚集分布
    std::vector<BBox> b(n);
    for (int i = 0; i < n; ++i) {
        float cx = frand(), cy = frand();
        float w = 0.03f + frand() * 0.05f, h = 0.03f + frand() * 0.05f;
        float x1 = cx + (frand() - 0.5f) * 0.04f - w * 0.5f;
        float y1 = cy + (frand() - 0.5f) * 0.04f - h * 0.5f;
        if (x1 < 0) x1 = 0; if (x1 + w > 1) x1 = 1 - w;
        if (y1 < 0) y1 = 0; if (y1 + h > 1) y1 = 1 - h;
        b[i].x1 = x1; b[i].y1 = y1; b[i].x2 = x1 + w; b[i].y2 = y1 + h;
        b[i].score = frand(); b[i].id = i;
    }
    return b;
}

/* 计时 */
double tick() {
    return (double)clock() / CLOCKS_PER_SEC * 1000.0;
}

/* 主实验 */
typedef void (*SortFunc)(std::vector<BBox>&);

struct SorterEntry {
    const char* name;
    SortFunc    sort;
};

int main() {
    srand((unsigned)time(NULL));
    printf("sort,dataDist,n,keep,tNMS,tSort\n");

    int sizes[] = {100, 1000, 10000};
    const char* dists[] = {"random", "cluster"};

    SorterEntry sorters[] = {
        {"quick", quickSortWrap},
        {"merge", mergeSortWrap},
        {"heap",  heapSortWrap},
        {"shell", shellSortWrap}
    };

    for (int si = 0; si < 3; ++si) {
        int n = sizes[si];
        for (int di = 0; di < 2; ++di) {
            std::vector<BBox> boxes = (di == 0) ? genRandom(n) : genCluster(n);
            for (int ki = 0; ki < 4; ++ki) {
                std::vector<BBox> tmp = boxes;
                double t0 = tick();
                sorters[ki].sort(tmp);
                double tSort = tick() - t0;

                std::reverse(tmp.begin(), tmp.end());
                t0 = tick();
                tmp = nms(tmp);
                double tNMS = tick() - t0;

                printf("%s,%s,%d,%d,%.2f,%.2f\n",
                       sorters[ki].name, dists[di], n, (int)tmp.size(), tNMS, tSort);
            }
        }
    }
    return 0;
}
