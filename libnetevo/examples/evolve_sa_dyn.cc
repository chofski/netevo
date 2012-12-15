/*===========================================================================
 NetEvo Evolution Using Topological Measure Example
 Copyright (C) 2011 Thomas E. Gorochowski <tgorochowski@me.com>
 Bristol Centre for Complexity Sciences, University of Bristol, Bristol, UK
 ----------------------------------------------------------------------------
 Illustrates how a the topology of a System can evolve, based upon a
 performance measure that it calculated from simulated dynamics. In
 this case we choose a standard order parameter, where minimisation of the 
 average percentage of unsynchonised node will result in highly synchronisable 
 networks.
 ============================================================================*/

#include <netevo.h>
#include <lemon/maps.h>
#include <algorithm>
#include <complex>

using namespace netevo;
using namespace lemon;

// ---------- Defining Our Dynamics ----------

// Define our own node ODE based on the Rossler chaotic oscillator
class RosslerChaoticOscillator : public NodeDynamic {
public:
   string getName   () { return "RosslerChaoticOscillator"; }
   int getStates () { return 3; } // Three states for each oscillator
   
   // The default parameters for each node.
   void setDefaultParams (Node v, System &sys) { 
      //sys.nodeData(v).dynamicParams.push_back(0.165); // a
      //sys.nodeData(v).dynamicParams.push_back(0.2);   // b
      //sys.nodeData(v).dynamicParams.push_back(10);    // c
   }
   
   // Function to calculate the update state for a given node
   void fn (Node v, System &sys, const State &x, State &dx, const double t) {
      
      // Collect information about this node
      int vID = sys.stateID(v);
      vector<double> &vParams = sys.nodeData(v).dynamicParams;
      
      // Coupling sums for each state
      double c1, c3;
      c1 = 0.0; 
      c3 = 0.0;
      
      // Calculate the coupling terms for all incoming edges
      for (System::InArcIt e(sys, v); e != INVALID; ++e) {
         int curEdgeID = sys.stateID(e);
         int curSourceID = sys.stateID(sys.source(e));
         c1 += x[curSourceID]   - x[vID];
         c3 += x[curSourceID+2] - x[vID+2];
      }
      
      // Update the node states with coupling (Rossler equations - coupling on x and z)
      dx[vID]   =  -x[(vID+1)] - x[(vID+2)] + (0.5 * c1);
      dx[(vID+1)] = x[vID] + 0.165*x[(vID+1)];
      dx[(vID+2)] = 0.2 + (x[vID] - 10.0)*x[vID+2] + (0.5 * c3);
   }
};


class MyMutate : public Mutate {
public:
   void mutate (System &sys, ChangeLog &logger) {
      int numOfRewires = 1, ns, es;
      Arc toDelete, toAdd;
      Node u, v;
      Random R;
      bool success;
      
      ns = countNodes(sys);
      es = countArcs(sys);
      R = sys.getRandom();
      numOfRewires = R.exponential(1);
      if (numOfRewires < 1) { numOfRewires = 1; }
      if (numOfRewires > 10) { numOfRewires = 10; }
      
      for (int i=0; i<numOfRewires; i++) {
         
         // Randomly pick an arc to delete and don't forget partner
         toDelete = sys.getArc(R[es]);
         sys.erase(findArc(sys,sys.target(toDelete),sys.source(toDelete)));
         sys.erase(findArc(sys,sys.source(toDelete),sys.target(toDelete)));
         
         // Randomly select two nodes and add undirected arc if none exist
         success = false;
         do {
            u = sys.getNode(R[ns]);
            do {
               v = sys.getNode(R[ns]);
            } while (u == v);
            toAdd = findArc(sys, u, v);
            if (toAdd == INVALID) {
               sys.addEdge(u,v);
               success = true;
            }
         } while (!success);
      }
   }
};

class SyncPerf : public Performance {
public:
   /** The type of the performance measure (whether or not dynamics is required). */
   performance_type_e getType () { return DYNAMICS_ONLY; }
   /** Calculates the performance measure. lambda_N / lambda_2 */
   double performance (System &sys, pair<vector<State>*,vector<double>*> *dyn) {
      int i, j, e, N, t;
      double mu, d, delta;
      
      vector<State> &sim = *(dyn->first);
      t = sim.size() -1;
      mu = 0;
      delta = 0.01;
      N  = countNodes(sys);
      
      for ( i=0; i<N; i++ ) {
         for ( j=0; j<N; j++ ) {
            if ( i != j ) {
               
               // Calculate the euclidean distance between the two nodes
               d = 0;
               for ( e=0; e<3; e++ ) {
                  if ( isnan(sim[t][(i*3)+e+1]) || 
                      isnan(sim[t][(j*3)+e+1]) ) {
                     return 1.0;
                  }

                  d += pow(sim[t][(i*3)+e+1] - sim[t][(j*3)+e+1], 2.0);
               }
               d = sqrt(d);
               // Calculate the heavyside function with delta error range 
               if ( d - delta >= 0 )
                  mu += 100;
               else
                  mu += 0;
            }
         }
      }
      
      // Normalise
      mu = mu * (1.0 / (N * (N - 1)));
      
      return mu;
   }
};

class RandomInit : public EvoInitialStates {
public:
   /** A vector of initial states to be used during the evolutionary process. */
   vector<State> initialStates (System &sys) {
      vector<State> init;
      State initial = State(sys.totalStates(), 0.0);
      for (int n=0; n < sys.totalStates(); ++n) {
         initial[n] = sys.rnd() * 10.0;
      }
      init.push_back(initial);
      return init;
   };
};

class MyEvoObserver : public EvoObserver {
public:
   void operator() (System &sys, double perf, int t) {
      // We will just output the system performance to screen.
      cout << "At step " << t << ", performance = " << perf << endl;
   }
};

// Main function
int main () {
   
   System sys, *sysOut;
   
   // Load our new dynamics into the system
   RosslerChaoticOscillator vDyn;
   sys.addNodeDynamic(&vDyn);

   // Create an undirected ring topology with our node dynamics (this includes diffuse coupling)
   sys.ringGraph(100, 2, "RosslerChaoticOscillator", "NoArcDynamic", true);
   
   EvolveSAParams evoParams; // Use the default parameters   
   MyMutate mut;
   SyncPerf per;
   EvolveSA evo(evoParams, per, mut);
   
   SimulateOdeConst simOdeConst = SimulateOdeConst(RK_CASH_KARP_54, 10e-5, 10e-5, 100.0);  
   RandomInit initial;
   
   ChangeLog nullLogger;
   MyEvoObserver obs;
   
   sys.saveToGML("evolve_sa_dyn_in.gml");
   sysOut = evo.evolve(sys, simOdeConst, initial, obs, nullLogger);
   sysOut->saveToGML("evolve_sa_dyn_out.gml");
   
   return 0;
}
