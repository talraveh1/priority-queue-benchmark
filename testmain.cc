#include <iostream>
#include <climits>
#include <math.h>
#include <cassert>
#include <cstdint>
#include <chrono>
#include <algorithm>
#include <random>
#include <queue>
#include <vector>

// Intel VTune ITT markers
#ifdef USE_ITT
#include <ittnotify.h>
static __itt_domain* g_itt_domain = __itt_domain_create("priqueue.benchmark");
static __itt_string_handle* g_itt_workload = __itt_string_handle_create("workload");
inline void itt_task_begin() { __itt_task_begin(g_itt_domain, __itt_null, __itt_null, g_itt_workload); }
inline void itt_task_end() { __itt_task_end(g_itt_domain); }
inline void itt_resume() { __itt_resume(); }
inline void itt_pause() { __itt_pause(); }
#else
inline void itt_task_begin() {}
inline void itt_task_end() {}
inline void itt_resume() {}
inline void itt_pause() {}
#endif

// include for test type II
#include "graph.h"

// include the priority queues
#include "boost/heap/priority_queue.hpp"
#include "boost/heap/pairing_heap.hpp"
#include "pairing_heap_priqueue.h"
#include "skiplist_priqueue.h"
#include "arch_aware_heap.h"

#define _STR(x) #x
#define STR(x) _STR(x)
#define MAX_BATCH_SIZE 1000

#ifndef TCAND
#define TCAND A
#define TCAND_A
#endif

#ifndef TTYPE
#define TTYPE I
#define TTYPE_I
#endif

#ifndef TID
#define TID i
#define TID_i
#endif

enum TEST_CAND
{
    A,B,C,D,E
};

enum TEST_ID
{
    i,ii,iii,iv,v,vi
};

using namespace std;


#if defined(TTYPE_I)
typedef int value_type;
#elif defined(TTYPE_II)
typedef NODE_M value_type;
#endif

#if defined(TCAND_A)
using chosen_pri_queue = std::priority_queue<value_type,vector<value_type>,greater<value_type>>;
#elif defined(TCAND_B)
using chosen_pri_queue = boost::heap::pairing_heap<value_type,boost::heap::compare<compare_item<value_type>>>;
#elif defined(TCAND_C)
using chosen_pri_queue = pairing_heap_priqueue<value_type>;
#elif defined(TCAND_D)
using chosen_pri_queue = skip_list_priqueue<value_type>;
#elif defined(TCAND_E)
#if !defined(TTYPE_I)
#error "Arch-aware heap only supports TTYPE_I (int)"
#endif
using chosen_pri_queue = MinHeap;
#endif


//calculate test size
const size_t sample_size(TEST_ID id) {
    return pow(10,(3+static_cast<int>(id)));
}


int main()
{
    //// test preparation
    srand(123);

    printf("[TEST] {\"cand\": [\"%s\"], \"type\": [\"%s\"], \"id\": [\"%s\"]}\n", STR(TCAND), STR(TTYPE), STR(TID));

    const size_t N = sample_size(TID);
    bool skip_test = false;
#if defined(TCAND_E)
    const int depth = MinHeap::minDepthForSize(static_cast<uint32_t>(N));
    chosen_pri_queue pq(depth);
#else
    chosen_pri_queue pq;
#endif
    printf("[TEST] {\"size\": [%lu]}\n",N);


//// random add and pop elements
#if defined(TTYPE_I)
    uint32_t total_cnt = N;
    uint32_t total_rm_cnt = 0;
    
    std::vector<int> v = std::vector<int>(N);
    for (int i=0;i<N;i++) v[i]=i;
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(v.begin(), v.end(), g);

    //// start timer
    itt_resume();
    itt_task_begin();
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    while (total_cnt!=0 || !pq.empty()) {
        // random add some elements
        if (total_cnt>0){
            auto push_batch_size = 1 + rand() % total_cnt;
            for(uint32_t n=0;n<push_batch_size;n++) {
                pq.push(v.at(total_cnt-1));
                total_cnt -= 1;
            }
        }
        auto pop_batch_size = 1 + rand() % pq.size();
        for(uint32_t n=0;n<pop_batch_size;n++) {
            pq.pop();
            total_rm_cnt++;
        }
    }
    assert(N==total_rm_cnt);
    itt_task_end();
    itt_pause();

#endif // TTYPE_I

//// find shortest path in graph
#if defined(TTYPE_II)

    // define graph
    Graph g;

    static uint32_t NODE_NUM = N;
    static uint32_t EDGE_NUM = 2*N;
    // rand gen data
    int edge_cnt = 0;
    srand(123);

    auto rd_double = [] (double a=0.0, double b=10.0) {
        return a + (double) rand() / RAND_MAX * (b-a);
    };

    auto rd_node = [] (int a, int b) {
        return a + (int) ((double) rand() / RAND_MAX * (double) (b-a));
    };

    for (int _=0; _<EDGE_NUM; _++){
        auto e = Edge(rd_node(0,NODE_NUM),rd_node(0,NODE_NUM),ceil(rd_double()));
        if(e.u!=e.v && !g.has_edge(e)) { //do not add edge if u==v
            g.add_edge(e);
            // cout << "[dij] added edge : " << e.repr() << endl;
            edge_cnt++;
        }
    }

    cout << "[dij] prepared edge size : " << edge_cnt << endl;

    // define source node s
    const int s = rd_node(0,NODE_NUM);

    // store all distances from source s
    vector<double> dist_info_vec (NODE_NUM,DBL_MAX); //store distance s to key node
    dist_info_vec[s] = 0.0;

    cout << "[dij] source node : " << s << endl;
    cout << "[dij] prepared dist info of size : " << dist_info_vec.size() << endl;
 
    
    //// start timer
    itt_resume();
    itt_task_begin();
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    pq.push(NODE_M(s,0.0));

    // find the shortest path from source s to each node
    map<int,int> prev_info_map; //store the prev node to key node

    while(pq.size()>0){
        auto min_node_m = pq.top(); 
        pq.pop();
        // cout << "popped min_node:" << min_node_m.repr() << "|" << g.get_list(min_node_m.node).size() << endl;
        for (const auto&neighbour_node : g.get_list(min_node_m.node)){
            double new_dist = dist_info_vec[min_node_m.node] + g.get_dist(min_node_m.node,neighbour_node);
            // cout << "new_dist" << new_dist << "| nn:" << neighbour_node << "," << dist_info_vec[neighbour_node] << endl;
            if (new_dist<dist_info_vec[neighbour_node]) {
                dist_info_vec[neighbour_node] = new_dist;
                prev_info_map[neighbour_node] = min_node_m.node;
                pq.push(NODE_M(neighbour_node,new_dist));
                // cout << ">> pushed:" << neighbour_node << "|" << new_dist << std::endl;
            }
        }
    }

    //// list all dist and prev info
    // cout << "[dij] min dist to each node and their prev node : ";
    // for (int i=0; i < NODE_NUM; i++){
    //     cout << "node : " << i << " | ";
    //     cout << "dist : " << dist_info_vec[i] << " | ";
    //     cout << "prev : " << ((prev_info_map.count(i)) ? prev_info_map[i] : -1) << endl;
    // }


#endif // TTYPE_II

    assert(pq.size()==0);
    itt_task_end();
    itt_pause();

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    uint64_t elapsed_time_us = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
    if (!skip_test)
        printf("[TEST] {\"elapsed_time_us\": [%lu]}\n",elapsed_time_us);
    else
        printf("[TEST] {\"elapsed_time_us\": [\"nan\"]}\n");
    return 0;
}
