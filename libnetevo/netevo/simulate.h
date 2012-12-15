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

#ifndef NE_SIMULATE_H
#define NE_SIMULATE_H

#include "system.h"

using namespace std;

namespace netevo {

   /** Fixed steppers available for use with SimulateOdeFixed. */
   enum fixed_step_type_e { 
      RK_4           = 0, /** Runge Kutta (4) */
      ADAM_BASH_MOUL = 1  /** Adam Bashforth Moulton */
   };
   
   /** Adaptive steppers available for use with SimulateOdeConst and SimulateOdeAdaptive. */
   enum adaptive_step_type_e { 
      RK_CASH_KARP_54  = 0, /** Runge Kutta Cash Karp (5,4) */
      RK_DOPRI_5       = 1, /** Runge Kutta Dormand & Prince (5) */
      RK_DOPRI_5_DENSE = 2  /** Runge Kutta Dormand & Prince (5) Dense Output */
   };
   
   class SimObserver {
   public:
      /** This should be overwritten by any observer. By default does nothing. */
      virtual void operator() (const State &x, double t) { };
   };
   
   class SimObserverToVectors : public SimObserver {
   private:
      std::vector<State> &mStates;
      std::vector<double> &mTimes;
   public:
      SimObserverToVectors (std::vector< State > &states , std::vector< double > &times) : mStates(states) , mTimes( times ) { }
      SimObserverToVectors(const SimObserverToVectors &obs) : mStates(obs.mStates), mTimes(obs.mTimes) { }
      void operator() (const State &x, double t) {
         mStates.push_back(x);
         mTimes.push_back(t);
      }
   };
   
   class SimObserverToStream : public SimObserver {
   private:
      ostream &mOutStream;
   public:
      SimObserverToStream (ostream &outStream) : mOutStream(outStream) { }
      SimObserverToStream(const SimObserverToStream &obs) : mOutStream(obs.mOutStream) { }
      void operator() (const State &x, double t) {
         int i;
         mOutStream << "t = " << t << ", state = (";
         if (x.size() > 0) {
            mOutStream << x[0];
            for (i=1; i<x.size(); ++i) {
               mOutStream << ", " << x[i];
            }
         }
         mOutStream << ")" << endl;
      }
   };

   /** Virtual class to define the interface for simulation of a System. */
   class Simulate {
      public:
      virtual void simulate (System &sys, double tMax, State &inital, SimObserver &obs, ChangeLog &logger) { };
   };

   class SimulateMap : public Simulate {
   public:
      SimulateMap () { };
      void simulate (System &sys, double tMax, State &initial, SimObserver &obs, ChangeLog &logger);
   };

   class SimulateOdeFixed : public Simulate {
   public:
      SimulateOdeFixed (fixed_step_type_e stepper, double stepSize) { 
         mStepper = stepper;
         mStepSize = stepSize; 
      };
      void simulate (System &sys, double tMax, State &initial, SimObserver &obs, ChangeLog &logger);
   private:
      fixed_step_type_e mStepper;
      double mStepSize;
   };

   class SimulateOdeConst : public Simulate {
   public:
      SimulateOdeConst (adaptive_step_type_e stepper, double epsAbs, double epsRel, double outputStep) { 
         mStepper = stepper;
         mEpsAbs = epsAbs;
         mEpsRel = epsRel;
         mOutputStep = outputStep;
      };
      void simulate (System &sys, double tMax, State &initial, SimObserver &obs, ChangeLog &logger);
   private:
      adaptive_step_type_e mStepper;
      double mEpsAbs;
      double mEpsRel;
      double mOutputStep;
   };

   class SimulateOdeAdaptive : public Simulate {
   public:
      SimulateOdeAdaptive (adaptive_step_type_e stepper, double epsAbs, double epsRel, double initialStep) { 
         mStepper = stepper;
         mEpsAbs = epsAbs;
         mEpsRel = epsRel;
         mInitialStep = initialStep;
      };
      void simulate (System &sys, double tMax, State &initial, SimObserver &obs, ChangeLog &logger);
   private:
      adaptive_step_type_e mStepper;
      double mEpsAbs;
      double mEpsRel;
      double mInitialStep;
   };
   
   class SimInitialState {
   public:
      /** A vector of initial states to be used during the evolutionary process. Called for each simulation step. */
      virtual State initialState (const System &sys) = 0;
   };

   /**  Required for odeint-v2 to simulate a System correctly (requires copy constructor which is expensive). */
   class Simulator {
   private:
      System *mSys;
   public:
      Simulator (System *sys) { mSys = sys; }
      Simulator (const Simulator &sim) { mSys = sim.mSys; }
      /** Call the () operator on the system to calculate the dynamics. */
      void operator() (const State &x, State &dx, const double t) { (*mSys)(x, dx, t); }
   };
   
   /** Required for odeint-v2 to output observations correctly to SimObservers. */
   class ObserverPassThrough {
   private:
      System      &mSys;
      SimObserver &mObs;
      ChangeLog   &mLogger;
   public:
      ObserverPassThrough (System &sys, SimObserver &obs, ChangeLog &logger) : mSys(sys), mObs(obs), mLogger(logger) { }
      ObserverPassThrough (const ObserverPassThrough &obs) : mSys(obs.mSys), mObs(obs.mObs), mLogger(obs.mLogger) { }
      void operator() (const State &x, double t) { 
         // Log the state change
         mLogger.newState(mSys, x);
         mLogger.endStep(SIM_STEP);
         mLogger.commit();
         // Observe the new state
         mObs(x,t);
      }
   };

} // netevo namespace

#endif // NE_SIMULATE_H
