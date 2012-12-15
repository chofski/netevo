/*===========================================================================
 NetEvo System Example
 Copyright (C) 2011 Thomas E. Gorochowski <tgorochowski@me.com>
 Bristol Centre for Complexity Sciences, University of Bristol, Bristol, UK
 ----------------------------------------------------------------------------
 In this example we show how to create Systems that can then be used for
 simulations and evolution. We save and load a System from file. Use our own
 custom dynamics and show how to access information from a System. As Systems
 in NetEvo are also compatible Digraphs in the LEMON graph library any
 functions in LEMON can also be applied to any System. These are of specific
 use when traversing the graph, e.g. iterating through nodes and arcs. See
 the LEMON project and documentation for details.
 ============================================================================*/


// All the main functionality of NetEvo is contained within netevo.h. The only
// aspect missing is related to visualisation as this will not be required by
// all users and requires the availability of addional libraries e.g. Cinder.
#include <netevo.h>


// NetEvo makes use of the following namespaces
using namespace lemon;
using namespace netevo;


// ---------- Defining Our Dynamics ----------
// We first define the node and arc dynamics that we will be using. These extend
// the basic NodeDynamic and ArcDynamic classes overwriting the functions as 
// necessary.

// My own node dynamics that do nothing in this case.
class MyNodeDynamic : public NodeDynamic {
public:
   string getName   () { return "MyNodeDynamic"; }
   int    getStates () { return 0; }
   void   setDefaultParams (Node v, System &sys) { sys.nodeData(v).dynamicParams.push_back(0.0); }
   void   fn (Node v, System &sys, const State &x, State &dx, const double t) { }
};

// My own arc dynamics that do nothing in this case
class MyArcDynamic : public ArcDynamic {
public:
   string getName   () { return "MyArcDynamic"; }
   int    getStates () { return 0; }
   void   setDefaultParams (Arc e, System &sys) { sys.arcData(e).dynamicParams.push_back(0.0); }
   void   fn (Arc e, System &sys, const State &x, State &dx, const double t) { };
};


// Main function where we define our simulation
int main () {
   
   // ---------- Creating Systems ----------
   // It is worth noting that all Systems in NetEvo are directed. This is the 
   // reason we refer to arcs and not edges. If you are working with undirected
   // graphs we provide some helper functions to generate arcs in both directions
   // when creating an edge. Users should be careful when updating properties
   // of edges though and ensure updates are carried out for both directions.
   
   // Empty systems can be created easily like any other object.
   System sys1;
   
   // We provide the ability to generate random graphs (useful for initial conditions
   // when evolving a topology).
   System sysRandomUndirected;
   sysRandomUndirected.randomGraph(0.0001, 1000, false, true);
   System sysRandomDirected;
   sysRandomDirected.randomGraph(0.0001, 1000, false, false);
   
   // The Undirected graph should (approx) has double the number of edges
   cout << "Directed graph has " << countArcs(sysRandomDirected) << ", and Undirected graph has "
        << countArcs(sysRandomUndirected) << endl;
   
   // It is also possible to load a topology from an external GML file. This
   // will set all node and arc dynamics to none if they are not specified in
   // the file. This enables integration with other graph based tools/libraries.
   System sysFromGML;
   sysFromGML.openFromGML("ring.gml");
   
   
   // ---------- Populating a Systems Dynamic Library ----------
   
   // Once created it is necessary to populate the internal dynamics library. This
   // is the list of node and arc dynamics that can be used by the system.
   
   // Create node dynamic and add to system
   MyNodeDynamic vDyn1;
   sys1.addNodeDynamic(&vDyn1);
   
   // Create arc dynamic and add to system
   MyArcDynamic eDyn1;
   sys1.addArcDynamic(&eDyn1);
   
   
   // ---------- Adding/Erasing Nodes and Arcs ----------
   
   // When adding nodes and arcs to our system, NetEvo must associate a dynamic
   // to them. By default it will use the build in null dynamics "NoNodeDynamic"
   // and "NoArcDynamic", which are automatically available in a systems
   // dynamics library once it is created. To use this we create objects without
   // specifying a dyanmic.
   Node v1 = sys1.addNode();
   
   // If instead we would like to use a new form of dynamic we have loaded then
   // we just specify the name during creation. 
   Node v2 = sys1.addNode("MyNodeDynamic");
   Node v3 = sys1.addNode("MyNodeDynamic");
   
   Arc e1 = sys1.addArc(v1, v2);
   Arc e2 = sys1.addArc(v2, v3, "MyArcDynamic");
   Arc e3 = sys1.addArc(v1, v3);
   
   
   // ---------- Accessing Node and Arc Properties ----------
   
   // Nodes and arcs have a structure associated with them which holds information
   // related to their name, properties, and dyanmics.
   NodeData &v2Data = sys1.nodeData(v2);
   ArcData &e1Data = sys1.arcData(e1);
   
   // It is worth noting that dynamics will automatically allocate default parameters
   // for the node or arc, which can be edited by grabbing the objects data. Any 
   // properties of a node or arc can be accessed in this way.
   sys1.nodeData(v2).dynamicParams[0] = 10.0;
   
   
   // ---------- Copying Systems ----------
   
   // It is also possible to create a system from an existing system. This will
   // copy all internal properties, including the dynamics library.
   System sys2;
   sys2.copySystem(sys1);
   
   // Systems can be created from existing LEMON graphs. This will copy the nodes
   // and arcs, setting all dynamics to none.
   ListDigraph lemonGraph;
   System sys3;
   sys3.copyDigraph(lemonGraph);
   
   // If you instead would like to specify the default node and arc dynamics to
   // use, they can be specified by name during the copying. These dynamics must
   // be loaded into the new system before copying.
   System sys4;
   sys4.addNodeDynamic(&vDyn1);
   sys4.addArcDynamic(&eDyn1);
   sys4.copyDigraph(lemonGraph, "MyNodeDynamic", "MyArcDynamic");

   
   // ---------- Saving and Loading From Files ----------

   // A very simple interface is provided to save Systems to file. The format is standard
   // GML which means they can be loaded by most other graph based tools. The files
   // are human readable which makes manual editing possible.
   
   // To save a System to file you just specify the full path and file name.
   sys1.saveToGML("example.gml");
   
   // Loading a System requires a couple additional steps. Because it is not possible
   // to save the actual code that specfies any dynamics for nodes or edges, the system 
   // that will hold the loaded contents must first have any used dynamics loaded.
   // The names of the dynamics held in the file are then matched to the new systems
   // internal library and linked when loading.
   System sysFromFile;
   sysFromFile.addNodeDynamic(&vDyn1);
   sysFromFile.addArcDynamic(&eDyn1);
   sysFromFile.openFromGML("example.gml");

   return 0;
}
