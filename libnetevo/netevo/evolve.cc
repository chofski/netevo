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

#include "evolve.h"

namespace netevo {
   
   void MutateRandom::mutate (System &sys, ChangeLog &logger) {
      // For each type of change see if it should be applied. If all probabilities
      // are < 1 then there is a chance that no mutation will take place.
      
      // Mutate the system the number of trials
      for (int i=0; i<mMutateTrials; ++i) {
         if (mRnd() < mProbNewNode) { newNode(sys, logger); }
         if (mRnd() < mProbDelNode) { delNode(sys, logger); }
         if (mRnd() < mProbNewEdge) { newEdge(sys, logger); }
         if (mRnd() < mProbDelEdge) { delEdge(sys, logger); }
         if (mRnd() < mProbUpdNode) { updNode(sys, logger); }
         if (mRnd() < mProbUpdEdge) { updEdge(sys, logger); }
         if (mRnd() < mProbRewire)  { rewire(sys, logger); }
         if (mRnd() < mProbDup)     { duplicate(sys, logger); }
      }
   }

} // netevo namespace
