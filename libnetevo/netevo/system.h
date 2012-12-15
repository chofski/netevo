/*===========================================================================
 NetEvo Library
 Copyright (C) 2011 Thomas E. Gorochowski <tgorochowski@me.com>
 Bristol Centre for Complexity Sciences, University of Bristol, Bristol, UK
 ---------------------------------------------------------------------------- 
 NetEvo is a computing framework designed to allow researchers to investigate 
 evolutionary aspects of dynamical complex networks. By providing tools to 
 easily integrate each of these factors in a coherent way, it is hoped a 
 greater understanding can be gained of key attributes and features displayed 
 by complex systems.
 
 NetEvo is open-source software released under the Open Source Initiative 
 (OSI) approved Non-Profit Open Software License ("Non-Profit OSL") 3.0. 
 Detailed information about this licence can be found in the COPYING file 
 included as part of the source distribution.
 
 This library is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 ============================================================================*/

#ifndef NE_SYSTEM_H
#define NE_SYSTEM_H

#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <lemon/list_graph.h>
#include <lemon/random.h>
#include <lemon/connectivity.h>
#include <Eigen/Eigenvalues>

using namespace std;
using namespace lemon;
using namespace Eigen;

namespace netevo {
    
   /** Type for a Node */
   typedef ListDigraph::Node Node;
    /** Type for an Arc */
   typedef ListDigraph::Arc  Arc;
    /** Type for an undirected Edge */
   typedef pair<Arc, Arc> Edge;
   // Pre-define the system
   class System;
   /** State used for system dynamics (nodes and edges) */
   typedef vector<double> State;
    
   /** Virtual class defining an interface for node dynamics. */
   class NodeDynamic {
   public:
      virtual string getName   () = 0;
      virtual int    getStates () = 0;
      virtual void   setDefaultParams (Node v, System &sys) = 0;
      virtual void   fn (Node v, System &sys, const State &x, State &dx, const double t) = 0;
   };
    
   /** Virtual class defining an interface for arc dynamics */
   class ArcDynamic {
   public:
      virtual string getName   () = 0;
      virtual int    getStates () = 0;
      virtual void   setDefaultParams (Arc e, System &sys) = 0;
      virtual void   fn (Arc e, System &sys, const State &x, State &dx, const double t) = 0;
   };
    
   /** Default null node dynamics */
   class NoNodeDynamic : public NodeDynamic {
      string getName   () { return "NoNodeDynamic"; };
      int    getStates () { return 0; };
      void   setDefaultParams (Node v, System &sys) { };
      void   fn (Node v, System &sys, const State &x, State &dx, const double t) { };
   };

   /** Default null arc dynamics */
   class NoArcDynamic : public ArcDynamic {
      string getName   () { return "NoArcDynamic"; };
      int    getStates () { return 0; };
      void   setDefaultParams (Arc e, System &sys) { };
      void   fn (Arc e, System &sys, const State &x, State &dx, const double t) { };
   };

   /** 3D position structure */
   typedef struct {
      double x;
      double y;
      double z;
   } Position;

   /** Structure to hold node related data */
   struct NodeData {
      int             key;
      string          name;
      Position        position;
      vector<double>  properties;
      NodeDynamic    *dynamic;
      vector<double>  dynamicParams;
   };

   /** Structure to hold arc related data */
   struct ArcData {
      string          name;
      double          weight;
      vector<double>  properties;
      ArcDynamic     *dynamic;
      vector<double>  dynamicParams;
   };

   class System : public ListDigraph {
   private:
      /** The parent type for systems */
      typedef ListDigraph Parent;

      /** Digraphs are not copy constructible. Use copySystem instead. */
      System(const System &) : ListDigraph() { }
      /** Assignment of a digraph to another one is not allowed. Use copySystem instead. */
      void operator=(const System &) { }
      
      /** Given a correctly sized square zero matrix, populates with the laplacian matrix */
      void fillLaplacian (MatrixXd &A);
      /** Given a correctly sized square zero matrix, populates with the adjacency matrix */
      void fillAdjacency (MatrixXd &A);

      /** Number of dynamic states required per node */
      int  mNodeStates;
      /** Number of dynamic states required per arc */
      int  mArcStates;
      
      /** Mapping of Node to int ID (0..max nodes)
       *  Used to find the start index of a node in a simulation state vector. */
      NodeMap<int> *mNodeIDs;
       /** Mapping of Arc to int ID (0..max arcs)
        *  Used to find the start index of an arc in a simulation state vector. */
      ArcMap<int>  *mArcIDs;
      
      /** Flag specifying if node IDs mapping (mNodeIDs) is up to date */
      bool mValidNodeIDs;
      /** Flag specifying if arc IDs mapping (mArcIDs) is up to date */
      bool mValidArcIDs;
      
      /** Node map holding all node properties (name, properties, dynamics) */
      NodeMap<NodeData> *mNodeData;
      /** Arc map holding all arc properties (name, weight, properties, dynamics) */
      ArcMap<ArcData>   *mArcData;

      /** Internal node dynamics library */
      std::map<string, NodeDynamic*> mNodeDynamics;
      /** Internal arc dynamics library */
      std::map<string, ArcDynamic*>  mArcDynamics;
      
      /** Null node dynamics (used as default when dynamics not specified) */
      NoNodeDynamic *noNodeDyn;
      /** Null arc dynamics (used as default when dynamics not specified) */
      NoArcDynamic *noArcDyn;
      
      int mNextKey;
      
      Random mRnd;
      
   public:
      /** System constructor
       *  Creates an empty System with no node or arcs. Initialises internal mappings and adds
       *  the null node and arc dynamics to the internal dynamics library. */
      System () : Parent () {
         mNodeStates = 0;
         mArcStates = 0;
         mNodeData = new NodeMap<NodeData>(*this);
         mArcData  = new ArcMap<ArcData>(*this);
         mNodeIDs = new NodeMap<int>(*this);
         mArcIDs  = new ArcMap<int>(*this);
         mValidNodeIDs = true;
         mValidArcIDs = true;
         // We include no dynamics as default types for all systems
         noNodeDyn = new NoNodeDynamic();
         noArcDyn = new NoArcDynamic();
         addNodeDynamic(noNodeDyn);
         addArcDynamic(noArcDyn);
         mNextKey = 0;
         mRnd.seed();
      }
      
      /** System destructor */
      ~System () {
         delete noNodeDyn;
         delete noArcDyn;
         delete mNodeData;
         delete mArcData;
         delete mNodeIDs;
         delete mArcIDs;
      }
      
      void clear () {
         Parent::clear();
         
         // State IDs are now invalid
         mValidNodeIDs = false;
         mValidArcIDs = false;
         
         // Reset our node keys
         mNextKey = 0;
      }
      
      void copyDigraph (const Digraph &from) {
         // Clear the existing System
         clear();

         // Copy the graph and initialise dynamics to none
         digraphCopy(from, *this).run();
      }

      void copyDigraph (const Digraph &from, string defNodeDyn, string defArcDyn) {         
         // Copy the structure
         copyDigraph(from);
         
         // Set the dynamics for every node
         for (NodeIt v(*this); v != INVALID; ++v) {
            NodeDynamic *nDyn = mNodeDynamics[defNodeDyn];
            (*mNodeData)[v].dynamic = nDyn;
            (*mNodeData)[v].dynamicParams.clear();
            nDyn->setDefaultParams(v, *this);
         }
         
         // Set the dynamics for every arc
         for (ArcIt e(*this); e != INVALID; ++e) {
            ArcDynamic *eDyn = mArcDynamics[defArcDyn];
            (*mArcData)[e].dynamic = eDyn;
            (*mArcData)[e].dynamicParams.clear();
            eDyn->setDefaultParams(e, *this);
         }
         mValidNodeIDs = false;
         mValidArcIDs = false;
      }
      
      void copyDigraph (const Digraph &from, string defNodeDyn, string defArcDyn, 
                        Digraph::NodeMap< Digraph::Node >  &refNodeMap,
                        Digraph::ArcMap< Digraph::Arc >    &refArcMap
                        ) {
         // Clear the existing System
         clear();
         
         // Copy the structure
         digraphCopy(from, *this)
         .nodeCrossRef(refNodeMap)
         .arcCrossRef(refArcMap)
         .run();
         
         // Set the dynamics for every node
         for (NodeIt v(*this); v != INVALID; ++v) {
            NodeDynamic *nDyn = mNodeDynamics[defNodeDyn];
            (*mNodeData)[v].dynamic = nDyn;
            (*mNodeData)[v].dynamicParams.clear();
            nDyn->setDefaultParams(v, *this);
         }
         
         // Set the dynamics for every arc
         for (ArcIt e(*this); e != INVALID; ++e) {
            ArcDynamic *eDyn = mArcDynamics[defArcDyn];
            (*mArcData)[e].dynamic = eDyn;
            (*mArcData)[e].dynamicParams.clear();
            eDyn->setDefaultParams(e, *this);
         }
         mValidNodeIDs = false;
         mValidArcIDs = false;
      }

      /** Copy one System to another
       *  We do not allow for copy consturctors due to some initialisation that needs to take place.
       *  Instead this method can be used with an initialised, but empty System. */
      void copySystem (System &from) {
         Node newN;
         Arc  newE;
         NodeMap<Node> nr(from);
         ArcMap<Arc> acr(from);
         
         // Clear the existing System
         clear();
         
         // Copy the graph structure
         digraphCopy(from, *this).nodeRef(nr).arcCrossRef(acr).run();

         // Copy the dynamics library
         mNodeDynamics = *from.getNodeDynamicsMap();
         mArcDynamics = *from.getArcDynamicsMap();

         // Copy the node data using the mapping
         for (NodeIt v(from); v != INVALID; ++v) {
            NodeData &toNodeData = nodeData(nr[v]);
            NodeData &fromNodeData = from.nodeData(v);
            // Copy fields
            toNodeData.key = fromNodeData.key;
            toNodeData.name = fromNodeData.name;
            toNodeData.position.x = fromNodeData.position.x;
            toNodeData.position.y = fromNodeData.position.y;
            toNodeData.position.z = fromNodeData.position.z;
            toNodeData.dynamic = fromNodeData.dynamic;
            toNodeData.dynamicParams = fromNodeData.dynamicParams;
            toNodeData.properties = fromNodeData.properties;
         }

         // Copy the arc data using the mapping
         for (ArcIt e(from); e != INVALID; ++e) {
            ArcData &toArcData = arcData(acr[e]);
            ArcData &fromArcData = from.arcData(e);
            // Copy fields
            toArcData.name = fromArcData.name;
            toArcData.weight = fromArcData.weight;
            toArcData.dynamic = fromArcData.dynamic;
            toArcData.dynamicParams = fromArcData.dynamicParams;
            toArcData.properties = fromArcData.properties;
         }
         
         // Invalidate IDs
         mValidNodeIDs = false;
         mValidArcIDs = false;
         
         // Copy the dynamic states
         mNodeStates = from.nodeStates();
         mArcStates = from.arcStates();
         
         // Update nextKey
         mNextKey = from.nextKey();
      }
      
      // Used for simulating the dynamics of the system (boost::odeint)
      void operator() (const State &x, State &dx, const double t);

      int nodeStates () { return mNodeStates; }
      int arcStates  () { return mArcStates; }

      int nextKey () { return mNextKey; }
      void resetKeys ();
      
      NodeData & nodeData (Node v) { return (*mNodeData)[v]; }
      ArcData &  arcData  (Arc e) { return (*mArcData)[e]; }

      // Getter methods for the node and arc dynamics library (don't think this is required)
      std::map<string, NodeDynamic*> * getNodeDynamicsMap () { return &mNodeDynamics; }
      std::map<string, ArcDynamic*> * getArcDynamicsMap () { return &mArcDynamics; }
      
      void addNodeDynamic (NodeDynamic *nodeDynamic);
      void addArcDynamic  (ArcDynamic  *arcDynamic);

      Node addNode () { return addNode("NoNodeDynamic"); }
      Node addNode (string dynamic);
      Node addNode (string name, string dynamic);

      Arc addArc (Node u, Node v) { return addArc(u, v, "NoArcDynamic"); }
      Arc addArc (Node u, Node v, string dynamic);
      Arc addArc (Node u, Node v, string name, string dynamic);

      // Add arcs in both directions (useful for undirected edges)
      Edge addEdge (Node u, Node v) { Arc a1 = addArc(v,u); Arc a2 = addArc(u,v); return pair<Arc,Arc>(a1,a2); }
      Edge addEdge (Node u, Node v, string dynamic);
      Edge addEdge (Node u, Node v, string name, string dynamic);
      
      Node getNode (int ID);
      Arc  getArc  (int ID);
      
      /** Whether the current state IDs are valid */
      bool validStateIDs ();
      /** Force a recalculation of the state IDs */
      void refreshStateIDs ();
      
      /** Total number of states to simulate this System. */
      int totalStates () { return ((mNodeStates * countNodes(*this)) + (mArcStates * countArcs(*this))); }
      
      /** State ID for a given node
       *  Calculates the index for the node in any dynamical state vector. */
      int stateID (Node v);
      /** State ID for a given arc
       *  Calculates the index for the arc in any dynamical state vector. */
      int stateID (Arc e);

      /** Save a System to a GML file
       *  We use GML as the native file format enabling systems created with NetEvo to be used 
       *  with other network related tools. */
      int saveToGML (string filename);
      /** Open a System from a GML file
       *  Opens a given GML file and loads the network and properties into the System. If the 
       *  GML file was not saved directly from NetEvo then some features such as dynamics and
       *  properties may not be loaded and defaults will instead be used. */
      int openFromGML (string filename);
      
      /** Seed the internal random number generator with a specific seed. */
      void seedRnd (int seed) { mRnd.seed(seed); }
      
      double rnd () { return mRnd(); }
      
      Random & getRandom() { return mRnd; }
      
      /** Generate a random topology with no dynamics. */
      void randomGraph (double edgeProb, int numOfNodes, bool selfLoops, bool undirected);
      /** Generate a random topology with user specific node and edge dynamics. */
      void randomGraph (double edgeProb, int numOfNodes, bool selfLoops, string defNodeDyn, string defEdgeDyn, bool undirected);
      
      void ringGraph (int numOfNodes, int neighbours, bool undirected);
      
      void ringGraph (int numOfNodes, int neighbours, string defNodeDyn, string defEdgeDyn, bool undirected);
      
      /** Will ensure that all arc (u->v) have a matching arc (v->u) for all u and v. */
      void makeUndirected ();
      
      int weaklyConnectedComponents ();

      /** Calculate the eigenvalues for the network
       *  Allows eigenvalues of the network to be generated using the laplacian or adjacency matrix */
      VectorXcd eigenvalues (int mType = 0);
      /** Calculate the eigensystem (values and vectors) for the network
       *  Allows the eigensystem of the network to be generated using the laplacian or adjacency matrix */
      pair<VectorXcd, MatrixXcd> eigensystem (int mType = 0);
      
   };
   
   /** Types of step that can occur. */
   typedef enum step_type_e {
      INIT_STEP = 0,
      SIM_STEP  = 1,
      EVO_STEP  = 2
   };
   
   /** Logs the changes that occur to a System. Used for export and visualisation. Should be called before 
    *  an update is made the actual System. */
   class ChangeLog {
   public:
      virtual void addNode  (System &sys, Node n) { };
      virtual void addArc   (System &sys, Node source, Node target) { };
      virtual void erase    (System &sys, Node n) { };
      virtual void erase    (System &sys, Arc e) { };
      
      virtual void update   (System &sys, Node n) { };
      virtual void update   (System &sys, Arc e) { };
      
      virtual void newState (System &sys, const State &newState) { };
      
      virtual void endStep  (step_type_e stepType) { };
      
      virtual void rollback () { };
      virtual void commit   () { };
   };
   
   class ChangeLogSet : ChangeLog {
   private:
      vector<ChangeLog*> mLoggers;
   public:
      ChangeLogSet () { };
      
      void addChangeLog (ChangeLog *logger);
      
      void addNode  (System &sys, Node n);
      void addArc   (System &sys, Node source, Node target);
      void erase    (System &sys, Node n);
      void erase    (System &sys, Arc e);
      
      void update   (System &sys, Node n);
      void update   (System &sys, Arc e);
      
      void newState (System &sys, const State &newState);
      
      void endStep  (step_type_e stepType);
      
      void rollback ();
      void commit   ();
   };
   
   class ChangeLogToStream : public ChangeLog {
   private:
      ostream &mOut;
      stringstream buffer;
   public:
      ChangeLogToStream (ostream &outStream) : mOut(outStream) { }
      
      void addNode  (System &sys, Node n);
      void addArc   (System &sys, Node source, Node target);
      void erase    (System &sys, Node n);
      void erase    (System &sys, Arc e);
      
      void update   (System &sys, Node n);
      void update   (System &sys, Arc e);
      
      void newState (System &sys, const State &newState);
      
      void endStep  (step_type_e stepType);
      
      void rollback ();
      void commit   ();
   };
	
} // netevo namespace

#endif // NE_SYSTEM_H
