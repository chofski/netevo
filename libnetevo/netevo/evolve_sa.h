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

#ifndef NE_EVOLVE_SA_H
#define NE_EVOLVE_SA_H

#include "system.h"
#include "simulate.h"
#include "evolve.h"
#include <lemon/random.h>

namespace netevo {

   /** Structure to pass current results between functions. */
   typedef struct {
      double Q1;            /**< Current graph */
      double Q2;            /**< New graph */
      double dQ;            /**< Q2 - Q1 */
      bool   a;             /**< Accept flag */
   } evolve_sa_result_t;
   
   /** Object encapsulating the parameters for the simulated annealing supervisor */
   class EvolveSAParams {
   public:
      /** Initial trials used to estimate initial temperature */
      int initialTrials;
      /** Number of main trails at each temp step */
      int mainTrials;
      /** Number of accepting trails before reduce temp */
      int acceptTrials;
      /** Number of accepting runs before end */
      int acceptRunsNoChange;
      /** Minimum temperature before process halts */
      double minTemp;
      /** Maximum number of iterations before process halts */
      int maxIterations;
      /** Should network always be connected */
      bool ensureWeaklyConnected;
      /** Time to simulate for */
      double simTMax;
      /** Seed for the random number generator */
      lemon::Random rnd;

      /** Constructor that generates default parameter values. */
      EvolveSAParams () {
         // Some default parameter values
         initialTrials         = 100;
         mainTrials            = 50;
         acceptTrials          = 10;
         acceptRunsNoChange    = 10;
         minTemp               = 0.01;
         maxIterations         = 100000;
         ensureWeaklyConnected = true;
         simTMax               = 100.0;
         rnd.seed();
      }
      
      /** Initial temperature calculated from a user specified number of trials */
      virtual double initialTemperature (double minQ, double maxQ) { return maxQ*4.0; }
      /** Cooling schedule for the process */
      virtual double newTemperature (double temp, double Q, double oldQ) { return temp*0.9; }
      /** Accepting probability for a new configuration (default = Boltzmann). */
      virtual double acceptProb (double dQ, double temp) { return exp(-dQ/temp); }
   };
   
   class EvolveSA {
   private:
      EvolveSAParams &mParams;
      Performance    &mQ;
      Mutate         &mMut;
      
      System * trial (double temp, System &sys, Simulate &sim, EvoInitialStates &initial, evolve_sa_result_t &result, ChangeLog &logger);
      double   performance (System &sys, Simulate &sim, EvoInitialStates &initial);
      
   public:
      EvolveSA (EvolveSAParams &params, Performance &Q, Mutate &mut) : mParams(params), mQ(Q), mMut(mut) { }
      System * evolve (System &sys, Simulate &sim, EvoInitialStates &initial, EvoObserver &obs, ChangeLog &logger);
   };

} // netevo namespace

#endif // NE_EVOLVE_SA_H
