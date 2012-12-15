/*===========================================================================
 NetEvo Simulation Using Discrete Time Maps Example
 Copyright (C) 2011 Thomas E. Gorochowski <tgorochowski@me.com>
 Bristol Centre for Complexity Sciences, University of Bristol, Bristol, UK
 ----------------------------------------------------------------------------
 This example creates a randomly connected network that has user specified
 dynamics in the form of a Kuromoto mapping. Node dynamics are coupled via
 edges and simulation of the dynamics shows eventual full synchronisation of
 the system.
 ============================================================================*/

// All simulators are included as part of the core of NetEvo
#include <netevo.h>

// Simplify our code such that we don't need to specify full namespaces
using namespace lemon;
using namespace netevo;


// ---------- Defining Our Dynamics ----------

// Define our own dynamical mapping based on the Kuramoto oscillator
class KuramotoNodeMap : public NodeDynamic {
public:
   string getName   () { return "KuramotoNodeMap"; }
   int getStates () { return 1; } // Single state - the phase
   
   // The default parameters for each node.
   void setDefaultParams (Node v, System &sys) { 
      sys.nodeData(v).dynamicParams.push_back(0.2); // Natural Freq
      sys.nodeData(v).dynamicParams.push_back(0.1); // Coupling Parameter
   }
   
   // Function to calculate the update state for a given node
   void fn (Node v, System &sys, const State &x, State &dx, const double t) {
      double sumCoupling = 0.0;
      
      // Collect information about this node
      double vState = x[sys.stateID(v)];
      vector<double> &vParams = sys.nodeData(v).dynamicParams;
      
      // Update the state vector for this node
      for (System::InArcIt e(sys, v); e != INVALID; ++e) {
         sumCoupling += sin(x[sys.stateID(sys.source(e))] - vState);
      }
      dx[sys.stateID(v)] = fmod(vState + vParams[0] + (vParams[1]*sumCoupling), 6.283);
   }
};

// Main function
int main () {
   
   // ---------- Create a System Using our Dynamics ----------
   
   // Create an empty system
   System sys;
   
   // Load our new dynamics into the system
   KuramotoNodeMap vDyn;
   sys.addNodeDynamic(&vDyn);
   
   // Generate a random undirected topology using Kuramoto dynamics for each node
   sys.randomGraph(0.5, 5, false, "KuramotoNodeMap", "NoArcDynamic", true);
   
   
   // ---------- Generate an Initial Random State ----------
   
   // Generate a random initial state for the simulation
   State initial = State(sys.totalStates(), 0.0);
   for (int i=0; i<initial.size(); ++i) { 
      initial[i] = sys.rnd() * 6.283; 
   }
   
   
   // ---------- Create a Suitable Simulator (for Maps) and Observers/Loggers ----------
   
   // Create a simulator for mapping dynamics
   SimulateMap simMap;
   // To observe the simulated output on the screen we use in built in streaming observer
   SimObserverToStream coutObserver(cout);
   // We don't need to log changes so use the default change logger that does nothing
   ChangeLog nullLogger;
   
   
   // ---------- Simulate the Dynamics ----------
   
   // Simulate the dynamics for our initial state and using the observer and logger
   simMap.simulate(sys, 50, initial, coutObserver, nullLogger); 
   
   // Output the initial vector (should be the final result)
   cout << "Initial vector = [";
   for (int j=0; j<initial.size()-1; ++j) { 
      cout << initial[j] << ", ";
   }
   cout << initial[initial.size()-1] << "]" << endl;
   
   return 0;
}
