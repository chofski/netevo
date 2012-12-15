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

#include "evolve_sa.h"
#include <lemon/random.h>

namespace netevo {
   
   System * EvolveSA::evolve (System &sys, Simulate &sim, EvoInitialStates &initial, EvoObserver &obs, ChangeLog &logger) {
      
      // Declare variables
      int iteration, i, accepts;
      double temp, minQ, maxQ, initialPerf, tempQ;
      bool noChange;
      evolve_sa_result_t result;
      
      // Systems to hold the current and trail systems
      System *curSys = new System();
      System *newSys, *tempSys;
      
      // Copy the current system
      curSys->copySystem(sys);
      tempSys = curSys;
      
      // Initise the results structure
      result.Q1 = 0.0;
      result.Q2 = 0.0;
      result.dQ = 0.0;
      result.a  = false;
      
      // Initialise simulated annealing process
      iteration = 0;
      temp = 1000000000000.0; // Start at high temp and ad
      
      // Calculate the initial performance of the System
      initialPerf = performance(*curSys, sim, initial);
      result.Q1 = initialPerf;
      
      // Record the initial iteration
      obs(*curSys, initialPerf, iteration);
      
      // Run the initial trials to estimate starting temperature (these are not observed)
      minQ = initialPerf;
      maxQ = initialPerf;
      for (i=0; i<mParams.initialTrials; ++i) {
         
         cout << "Initial Trail: " << i+1 << endl;
         
         // Generate a new System
         newSys = trial(temp, *tempSys, sim, initial, result, logger);
         
         // Free memory
         if (i > 0) { delete(tempSys); }
         
         // Keep track of max and min performances
         if (result.Q2 < minQ) { minQ = result.Q2; }
         if (result.Q2 > maxQ) { maxQ = result.Q2; }
         
         // Swap System pointers
         tempSys = newSys;
      }
      
      // Free used memory
      delete(newSys);
      
      // Set the initial temperature
      temp = mParams.initialTemperature(minQ, maxQ);
      
      // Start the SA process properly
      noChange = false;
      
      // Ensure the temperature does not start at 0
      if ( temp > 0.0 ) {
         
         // Check the conditions to continue the search
         while (noChange <= mParams.acceptRunsNoChange && 
                temp > mParams.minTemp && 
                iteration <= mParams.maxIterations) {
            
            /* Run for mainTrials more trials or acceptTrials accepting trials */
            accepts = 0;
            for ( i=0; i<mParams.mainTrials; i++ ) {
               
               iteration++;
               
               /* Check if we have reached the maximum number of iterations */
               if (iteration > mParams.maxIterations) {
                  break;
               }				
               
               /* Run a trail */
               newSys = trial(temp, *curSys, sim, initial, result, logger);
               
               /* Check to see if accepted and output result */
               if ( result.a == true ){
                  delete(curSys);
                  curSys = newSys;
                  tempQ = result.Q1;
                  result.Q1 = result.Q2;
                  result.Q2 = tempQ;
               }
               else{
                  delete(newSys);
               }
               
               // Observe the current System
               obs(*curSys, result.Q1, iteration);
               
               /* Update accepting counters */
               if ( result.a == true ){ accepts++; }
               if ( accepts >= mParams.acceptTrials ) { break; }
            }
            
            /* Update the counter to check if no changes are made at lower temps */
            if ( accepts == 0 ) { noChange++; }
            else { noChange = 0; }
            
            /* Reduce temperature */
            temp = mParams.newTemperature(temp, result.Q1, result.Q2);
         }
      }
      
      // Return the evolved System
      return curSys;
   }
   
   System * EvolveSA::trial (double temp, System &sys, Simulate &sim, EvoInitialStates &initial, evolve_sa_result_t &result, ChangeLog &logger) {
      
      // Don't accept by default
      result.a = false;
      
      // Make a copy of the system for the trial
      System *newSys = new System();
      newSys->copySystem(sys);
      
      // Mutate the System
      mMut.mutate(*newSys, logger);
      
      // Check if network needs to be connected
      if (mParams.ensureWeaklyConnected) {
         // We just want to check there are no isolated nodes (weakly connected)
         if (newSys->weaklyConnectedComponents() != 1) {
            return newSys;
         }
      }
      
      // Estimate the new performance
      result.Q2 = performance(sys, sim, initial);
      
      // Decide if this should be selected
      result.dQ =  result.Q1 - result.Q2;
      if (result.dQ > 0.0) {
         // Accept as improvement
			result.a = true;
		}
		else {
			// To ensure no divide by zero
			if (temp != 0.0 && temp > 0.0) {
				// Calculate probability of still accepting
				if (mParams.rnd() <= mParams.acceptProb(result.dQ, temp)) {
					// Make change even though worse performance
					result.a = true;
				}
			}
         else {
            // Don't accept and warn of error
            cerr << "Divide by zero avoided (EvolveSA::trial)" << endl;
         }
		}
      
      // Return the new System (caller will decide whether to use or not)
      return newSys;
   }
   
   double EvolveSA::performance (System &sys, Simulate &sim, EvoInitialStates &initial) {
      
      // Start with a bad performance (smaller is better)
      double perf = 100000000000.0;
      
      // Declare other variables
      int numOfSims, qSum;
      vector<State> initialConds;
      vector<double> tOut;
      vector<State> xOut;
      SimObserverToVectors simObs(xOut, tOut);
      ChangeLog chLog;
      
      // Find the performance type and simulate dynamics if necessary
      switch (mQ.getType()) {
         case TOPOLOGY_ONLY:
            // No need to simulate the dynamics
            perf = mQ.performance(sys, NULL);
            break;
         
         case DYNAMICS_ONLY:
         case TOPOLOGY_AND_DYNAMICS:
            // Have to simulate the dynamics to estimate performance
            qSum = 0.0;
            initialConds = initial.initialStates(sys);

            numOfSims = initialConds.size();
            if (numOfSims > 0) {
               for (int i=0; i<numOfSims; ++i) {
                  State &runInit = initialConds[i];
                  sim.simulate(sys, mParams.simTMax, initialConds[i], simObs, chLog);
                  pair<vector<State>*,vector<double>*> dyn(&xOut,&tOut);
                  qSum += mQ.performance(sys, &dyn);
                  tOut.clear();
                  xOut.clear();
               }
               perf = qSum/initialConds.size();
            }
            break;
         
         default:
            // Do nothing
            break;
      }
   
      // Return the estimated performance
      return perf;
   }
   
} // netevo namespace
