/*===========================================================================
 NetEvo Evolution Using Topological Measure Example
 Copyright (C) 2011 Thomas E. Gorochowski <tgorochowski@me.com>
 Bristol Centre for Complexity Sciences, University of Bristol, Bristol, UK
 ----------------------------------------------------------------------------
 Illustrates how a the topology of a System can evolve, based upon a
 performance measure that it calculated from the topological structure. In
 this case we choose the Eigenratio, where minimisation of the lambda_N
 divided by lambda_2 will result in highly synchronisable networks. lambda_i
 are the eigen values of the network Laplacian.
 ============================================================================*/

#include <netevo.h>
#include <lemon/maps.h>
#include <algorithm>
#include <complex>
#include <Eigen/Eigenvalues>

using namespace netevo;
using namespace lemon;
using namespace Eigen;
using namespace std;

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

bool compareComplex (complex<double> i, complex<double> j) { return (-real(i) < -real(j)); }

class EigenratioPerf : public Performance {
public:
   /** The type of the performance measure (whether or not dynamics is required). */
   performance_type_e getType () { return TOPOLOGY_ONLY; }
   /** Calculates the performance measure. lambda_N / lambda_2 */
   double performance (System &sys, pair<vector<State>*,vector<double>*> *dyn) {
      VectorXcd L = sys.eigenvalues(0);
      sort(L.data(), L.data()+L.size(), compareComplex);
      double l2 = real(L(1));
      if (l2 < 0) { l2 = -l2; }
      double ln = real(L(L.size()-1));
      if (ln < 0) { ln = -ln; }
      return ln / l2;
   }
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
   //sys.randomGraph(0.3, 25, false, true);
   sys.ringGraph(25, 2, true);
   
   EvolveSAParams evoParams;
   MyMutate mut;
   EigenratioPerf per;
   
   EvolveSA evo(evoParams, per, mut);
   
   ChangeLog nullLogger;
   MyEvoObserver obs;
   EvoInitialStates initial;
   Simulate nullSimulator;

   sys.saveToGML("EvoTopIn.gml");
   sysOut = evo.evolve(sys, nullSimulator, initial, obs, nullLogger);
   sysOut->saveToGML("EvoTopOut.gml");
   
   cout << "Initial Perf = " << per.performance(sys, NULL) 
        << ", Final Perf = " << per.performance(*sysOut, NULL) << endl;
   
   return 0;
}
