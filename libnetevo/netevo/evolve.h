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

#ifndef NE_EVOLVE_H
#define NE_EVOLVE_H

#include "system.h"
#include <lemon/random.h>

using namespace lemon;

namespace netevo {
   
   /** Enum of the types of performance measure */
   typedef enum performance_type_e {
      TOPOLOGY_ONLY         = 0,
      DYNAMICS_ONLY         = 1,
      TOPOLOGY_AND_DYNAMICS = 2
   };

   class Mutate {
   public:
      virtual void mutate (System &sys, ChangeLog &logger) = 0;
   };
   
   class MutateRandom : public Mutate {
   private:
      double mProbNewNode;
      double mProbDelNode;
      double mProbNewEdge;
      double mProbDelEdge;
      double mProbUpdNode;
      double mProbUpdEdge;
      double mProbRewire;
      double mProbDup;
      
      int mMutateTrials;
      
      Random mRnd;
      
   public:
      MutateRandom () { 
         mProbNewNode  = 0.0;
         mProbDelNode  = 0.0;
         mProbNewEdge  = 0.0;
         mProbDelEdge  = 0.0;
         mProbUpdNode  = 0.0;
         mProbUpdEdge  = 0.0;
         mProbRewire   = 0.0;
         mProbDup      = 0.0;
         mMutateTrials = 1;
         mRnd.seed();
      }
      
      /** Seed the internal random number generator with a specific seed. */
      void seedRnd (int seed) { mRnd.seed(seed); }
      
      void setNewNodeProb   (double prob) { mProbNewNode = prob; }
      void setDelNodeProb   (double prob) { mProbDelNode = prob; }
      void setNewEdgeProb   (double prob) { mProbNewEdge = prob; }
      void setDelEdgeProb   (double prob) { mProbDelEdge = prob; }
      void setUpdNodeProb   (double prob) { mProbUpdNode = prob; }
      void setUpdEdgeProb   (double prob) { mProbUpdEdge = prob; }
      void setRewireProb    (double prob) { mProbRewire  = prob; }
      void setDuplicateProb (double prob) { mProbDup     = prob; }
      
      void setMutateTrials  (int num) { mMutateTrials = num; }

      virtual void mutate (System &sys, ChangeLog &logger);
      
      virtual void newNode   (System &sys, ChangeLog &logger) { }
      virtual void delNode   (System &sys, ChangeLog &logger) { }
      virtual void newEdge   (System &sys, ChangeLog &logger) { }
      virtual void delEdge   (System &sys, ChangeLog &logger) { }
      virtual void updNode   (System &sys, ChangeLog &logger) { }
      virtual void updEdge   (System &sys, ChangeLog &logger) { }
      virtual void rewire    (System &sys, ChangeLog &logger) { }
      virtual void duplicate (System &sys, ChangeLog &logger) { }
   };

   class Performance {
   public:
      /** The type of the performance measure (whether or not dynamics is required). */
      virtual performance_type_e getType () = 0;
      /** Calculates the performance measure. */
      virtual double performance (System &sys, pair<vector<State>*,vector<double>*> *dyn) = 0;
   };
   
   class EvoObserver {
   public:
      /** This should be overwritten by any observer. By default does nothing. */
      virtual void operator() (System &sys, double perf, int t) { };
   };
   
   class EvoInitialStates {
   public:
      /** A vector of initial states to be used during the evolutionary process. Called for each 
       *  simulation step. */
      virtual vector<State> initialStates (System &sys) { return vector<State>(); };
   };

} // netevo namespace

#endif // NE_EVOLVE_H
