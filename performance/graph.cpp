// g++ -O2 -std=gnu++1z graph.cpp -o graph.exe -lmorn -lpthread

#define USE_BOOST
#define USE_LEMON
// #define USE_CXXGRAPH

#include "morn_util.h"
#include "morn_math.h"

#ifdef USE_BOOST
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#endif

#ifdef USE_LEMON
#include <lemon/list_graph.h>
#include <lemon/dijkstra.h>
#endif

#ifdef USE_CXXGRAPH
#include <CXXGraph/CXXGraph.hpp>
#endif

void test(int node_num,int edge_num)
{
    printf("\n%d node, %d edge: \n",node_num,edge_num);
    int i,j,n;
    MMatrix *matrix=mMatrixCreate(node_num,node_num);
    float **mat=matrix->data;
    for(int j=0;j<node_num;j++)
    {
        memset(mat[j],0,node_num*sizeof(float));
        if(j+1<node_num) mat[j][j+1]=mRand(10000,100000);
    }
    mat[node_num-1][0]=mRand(10000,100000);
    for(i=node_num;i<edge_num;i++)
    {
        int n=mRand(0,node_num);
        int m=mRand(0,node_num);
        mat[m][n]=mRand(10000,100000);
    }
    
    int idx[101];for(i=0;i<101;i++) idx[i]=mRand(0,node_num);
    
    #ifdef USE_BOOST
    typedef boost::adjacency_list_traits<boost::listS,boost::listS,boost::directedS >::vertex_descriptor BoostNode;
    typedef boost::adjacency_list<boost::listS,boost::listS,boost::directedS,
                                      boost::property<boost::vertex_index_t,int,
                                      boost::property<boost::vertex_distance_t,int,
                                      boost::property<boost::vertex_predecessor_t,BoostNode>>>,
                                  boost::property<boost::edge_weight_t,int>>
            BoostGraph;
    typedef std::pair< int,int > BoostEdge;
    
    BoostEdge *edge_array=(BoostEdge *)malloc(edge_num*sizeof(BoostEdge));
    float *weight=(float *)malloc(edge_num*sizeof(float));
    n=0;
    for(j=0;j<node_num;j++)for(i=0;i<node_num;i++)
        if(mat[j][i]!=0) {edge_array[n]=BoostEdge(j,i);weight[n]=mat[j][i];n++;}
    BoostGraph boost_graph(edge_array,edge_array+n,weight,node_num);
    
    boost::property_map< BoostGraph,boost::vertex_index_t>::type indexmap=boost::get(boost::vertex_index,boost_graph);
    boost::graph_traits< BoostGraph >::vertex_iterator id,idend;
    boost::tie(id,idend) = boost::vertices(boost_graph);
    for(i=0; id != idend;id++,i++) indexmap[*id] = i;
    boost::property_map< BoostGraph,boost::vertex_distance_t>::type distance=boost::get(boost::vertex_distance,boost_graph);
    
    float boost_result[100];
    mTimerBegin("boost");
    for(i=0;i<100;i++)
    {
        boost::dijkstra_shortest_paths(boost_graph,boost::vertex(idx[i],boost_graph),boost::distance_map(distance));
        boost_result[i]=distance[boost::vertex(idx[i+1],boost_graph)];
//         printf("from %d to %d,d=%f\n",idx[i],idx[i+1],boost_result[i]);
    }
    mTimerEnd("boost");
    
    free(edge_array);
    free(weight);
    #endif
    
    #ifdef USE_LEMON
    lemon::ListGraph lemon_graph;
    lemon::ListGraph::Node *lemon_node=(lemon::ListGraph::Node *)mMalloc(node_num*sizeof(lemon::ListGraph::Node));
    for(i=0;i<node_num;i++) lemon_node[i]=lemon_graph.addNode();
    lemon::ListGraph::EdgeMap<float> lemon_length(lemon_graph);
    for(j=0;j<node_num;j++)for(i=0;i<node_num;i++)
    {
        if(mat[j][i]!=0) 
        {
            lemon::ListGraph::Edge lemon_edge=lemon_graph.addEdge(lemon_node[j],lemon_node[i]);
            lemon_length.set(lemon_edge,mat[j][i]);
        }
    }
    lemon::Dijkstra<lemon::ListGraph,lemon::ListGraph::EdgeMap<float>> dijkstra_test(lemon_graph,lemon_length);
    float lemon_result[100];
    mTimerBegin("lemon");
    for(i=0;i<100;i++)
    {
        dijkstra_test.run(lemon_node[idx[i]]);
        lemon_result[i]=dijkstra_test.dist(lemon_node[idx[i+1]]);
    }
    mTimerEnd("lemon");
    
    mFree(lemon_node);
    #endif
    
    #ifdef USE_CXXGRAPH
    std::vector<CXXGraph::Node<int>> cxx_node;
    char name[8];
    for(i=0;i<node_num;i++)
    {
        sprintf(name,"%d",i);
        CXXGraph::Node<int> node(name,i);
        cxx_node.push_back(node);
    }
    CXXGraph::T_EdgeSet<int> cxx_edge_set;
    n=0;
    for(j=0;j<node_num;j++)for(i=0;i<node_num;i++)
    {
        if(mat[j][i]!=0)
        {
            CXXGraph::DirectedWeightedEdge<int> cxx_edge(n,cxx_node[j],cxx_node[i],mat[j][i]);
            cxx_edge_set.insert(std::make_shared<CXXGraph::DirectedWeightedEdge<int>>(cxx_edge));
            n++;
        }
    }
    CXXGraph::Graph<int> cxx_graph(cxx_edge_set);
    
    float cxxgraph_result[100];
    mTimerBegin("CXXGraph");
    for(i=0;i<100;i++)
    {
        auto res =cxx_graph.dijkstra(cxx_node[idx[i]], cxx_node[idx[i+1]]);
        cxxgraph_result[i]=res.result;
    }
    mTimerEnd("CXXGraph");
    #endif
    
    #if 1
    MGraph *morn_graph=mGraphCreate();
    MGraphNode **morn_node=(MGraphNode **)mMalloc(node_num*sizeof(MGraphNode *));
    for(i=0;i<node_num;i++) morn_node[i]=mGraphNode(morn_graph);
    for(j=0;j<node_num;j++)for(i=0;i<node_num;i++)
        if(mat[j][i]!=0) mGraphLink(morn_node[j],morn_node[i],mat[j][i]);
    
    float morn_result[100];
    mTimerBegin("Morn");
    for(i=0;i<100;i++)
    {
//         printf("from %d to %d\n",idx[i],idx[i+1]);
        morn_result[i]=mGraphPath(morn_node[idx[i]],morn_node[idx[i+1]]);
    }
    mTimerEnd("Morn");
    
    mFree(morn_node);
    mGraphRelease(morn_graph);
    #endif
    
    #ifdef USE_BOOST
    for(i=0;i<100;i++)
    {
        if(ABS(boost_result[i]-morn_result[i])<0.1) continue;
        printf("error: from %d to %d,boost=%f,morn=%f\n",idx[i],idx[i+1],boost_result[i],morn_result[i]);
    }
    #endif

    mMatrixRelease(matrix);
}

int main()
{
    test(1000,10000);
    test(1000,100000);
    test(10000,100000);
    test(10000,1000000);
    return 0;
}
