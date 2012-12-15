/*===========================================================================
 NetEvo Simulation Using ODEs Example
 Copyright (C) 2011 Thomas E. Gorochowski <tgorochowski@me.com>
 Bristol Centre for Complexity Sciences, University of Bristol, Bristol, UK
 ----------------------------------------------------------------------------
 In this example we simulate the dynamics of a network with ODE descriptions
 for both node and edge states. Nodes are Lorenz chaotic oscillators and edges
 use an adaptive law to govern their strength. This is taken from the paper:
 
 P. De Lellis et al. "Synchronization of complex networks through local
 adaptive coupling". CHAOS 18, 037110 (2008).
 
 Starting from a random initial state for both nodes and edges. We simulate
 the dynamics to find a final topology achieving synchronisation of the entire
 network. Finally, we copy the resting edge states to the weight variable
 of edges in our system and export to file. This can then be viewed within
 tools such as Cytoscape - http://www.cytoscape.org
 ============================================================================*/

// All simulators are included as part of the core of NetEvo
#include <netevo.h>
#include <cmath>

// Simplify our code such that we don't need to specify full namespaces
using namespace lemon;
using namespace netevo;
using namespace std;


// ---------- Defining Our Dynamics ----------

// Define our own node ODE based on the Lorenz chaotic oscillator
class LorenzChaoticOscillator : public NodeDynamic {
public:
   string getName   () { return "LorenzChaoticOscillator"; }
   int getStates () { return 3; } // Single state - the phase
   
   // The default parameters for each node.
   void setDefaultParams (Node v, System &sys) { 
      sys.nodeData(v).dynamicParams.push_back(28.0);    // sigma
      sys.nodeData(v).dynamicParams.push_back(10.0);    // rho
      sys.nodeData(v).dynamicParams.push_back(8.0/3.0); // beta
   }
   
   // Function to calculate the update state for a given node
   void fn (Node v, System &sys, const State &x, State &dx, const double t) {
      
      // Collect information about this node
      int vID = sys.stateID(v);
      vector<double> &vParams = sys.nodeData(v).dynamicParams;
      
      // Coupling sums for each state
      double c1, c2, c3;
      c1 = 0.0; 
      c2 = 0.0;
      c3 = 0.0;
      
      // Calculate the coupling terms for all incoming edges
      for (System::InArcIt e(sys, v); e != INVALID; ++e) {
         int curEdgeID = sys.stateID(e);
         int curSourceID = sys.stateID(sys.source(e));
         c1 += -x[curEdgeID] * (x[curSourceID]   - x[vID] );
         c2 += -x[curEdgeID] * (x[curSourceID+1] - x[vID+1]);
         c3 += -x[curEdgeID] * (x[curSourceID+2] - x[vID+2]);
      }
      
      // Update the node states with coupling (Lorenz equations - coupling)
      dx[vID]   = (vParams[0]*(x[(vID+1)] - x[vID]))                - c1;
      dx[(vID+1)] = (x[vID]*(vParams[1] - x[(vID+2)]) - x[(vID+1)]) - c2;
      dx[(vID+2)] = (x[vID]*x[(vID+1)] - vParams[2]*x[(vID+2)])     - c3;
   }
};

// Define our own edge ODE based on the adaptive law from De Lellis et al.
class AdaptiveEdgeLaw : public ArcDynamic {
public:
   string getName   () { return "AdaptiveEdgeLaw"; }
   int getStates () { return 1; } // Single state - the coupling strength
   
   // The default parameters for each node.
   void setDefaultParams (Arc e, System &sys) { 
      sys.arcData(e).dynamicParams.push_back(0.1); // Coupling Parameter (alpha)
   }
   
   // Function to calculate the update state for a given node
   void fn (Arc e, System &sys, const State &x, State &dx, const double t) {
      
      // Update the state edge strength using the adaptive law
      dx[sys.stateID(e)] = sys.arcData(e).dynamicParams[0] * 
         sqrt(pow(x[sys.stateID(sys.source(e))] - x[sys.stateID(sys.target(e))], 2.0));
   }
};


// Main function
int main () {
   
   // ---------- Create a System Using our Dynamics ----------
   
   // Create an empty system
   System sys;
   
   // Load our new dynamics into the system
   LorenzChaoticOscillator vDyn;
   sys.addNodeDynamic(&vDyn);
   AdaptiveEdgeLaw eDyn;
   sys.addArcDynamic(&eDyn);
   
   // Generate a random undirected topology using Kuramoto dynamics for each node
   sys.randomGraph(0.2, 50, false, "LorenzChaoticOscillator", "AdaptiveEdgeLaw", true);
   
   
   // ---------- Generate an Initial Random State ----------
   
   // Generate a random initial state for the simulation
   State initial = State(sys.totalStates(), 0.0); 
   
   // Update the node states
   for (System::NodeIt n(sys); n != INVALID; ++n) {
      initial[sys.stateID(n)] = sys.rnd() * 1600.0; 
   }
   
   // Update the edge states
   for (System::ArcIt e(sys); e != INVALID; ++e) {
      initial[sys.stateID(e)] = 0.000001; // Start with all arcs at a small strength 
   }
   
   
   // ---------- Create a Suitable Simulator (for Maps) and Observers/Loggers ----------
   
   // Create several simulators for dynamics (illustrate the 3 types of simulation)
   //SimulateOdeFixed simOdeFixed = SimulateOdeFixed(RK_4, 0.01); // Fixed Step
   //SimulateOdeAdaptive simOdeAdaptive = SimulateOdeAdaptive(RK_CASH_KARP_54, 10e-6, 10e-6, 0.1); // Adaptive Step
   SimulateOdeConst simOdeConst = SimulateOdeConst(RK_CASH_KARP_54, 10e-6, 10e-6, 1.0); // Adaptive Step with Fixed Output
   
   // To observe the simulated output on the screen we use in built in streaming observer.
   // We will use this for the fixed and adpative simulators.
   SimObserver nullObserver;
   //SimObserverToStream coutObserver(cout);
   
   // For the constant time step simulator we will save the state to vectors
   vector<double> tOut;
   vector<State>  xOut;
   SimObserverToVectors vectorObserver(xOut, tOut);
   
   // We don't need to log changes so use the default change logger that does nothing
   ChangeLog nullLogger;
   
   
   // ---------- Simulate the Dynamics ----------

   // Simulate the system. The following 3 lines show the different available simulators. (Uncomment one)
   // simOdeFixed.simulate(sys, 20.0, initial, vectorObserver, nullLogger);
   // simOdeAdaptive.simulate(sys, 20.0, initial, vectorObserver, nullLogger);
   simOdeConst.simulate(sys, 20.0, initial, vectorObserver, nullLogger);
   
   // Print the dynamics to the screen
   cout << endl;
   cout << "---------------------------------------" << endl;
   cout << "Simulated output " << endl;
   cout << "---------------------------------------" << endl;
   for (int i=0; i<tOut.size(); ++i) {
      State curState = xOut[i];
      cout << "t = " << tOut[i] << ", state = (" << curState[0];
      for (int j=1; j<9; ++j) { // Output the first 3 oscillator states
         cout << ", " << curState[j];
      }
      cout << ")" << endl;
   }
   
   
   // ---------- Add Final Coupling Strengths to System and Export ----------
   
   // For each edge grab the associated state and update the weight property
   for (System::ArcIt e(sys); e != INVALID; ++e) {
      sys.arcData(e).weight = (xOut[xOut.size()-1])[sys.stateID(e)];
   }
   sys.saveToGML("simulate_ode.gml");
      
   return 0;
}
