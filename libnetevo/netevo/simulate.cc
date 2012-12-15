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

#include "simulate.h"
#include <boost/numeric/odeint.hpp>
#include <boost/numeric/odeint/stepper/adams_bashforth_moulton.hpp>

using namespace boost::numeric::odeint;

namespace netevo {

   void SimulateMap::simulate (System &sys, double tMax, State &initial, SimObserver &obs, ChangeLog &logger) {
      
      // We are in discrete time so use integers for time
      int t = 0, tEnd = (int)tMax;
      int states = (countNodes(sys)*sys.nodeStates()) + (countArcs(sys)*sys.arcStates());

      // Check to ensure that initial conditions are correct size
      if (initial.size() < states || initial.size() > states) {
         cerr << "Incorrect number of states for initial conditions (SimulateMap::simulate)" << endl;
         return;
      }

      // Vector to hold the output. Copy the initial conditions and add to the output
      State y1 = State(initial);
      State y2 = State(initial);

      // Check that the state IDs are correct, if not refresh
      if (!sys.validStateIDs()) { sys.refreshStateIDs(); }

      // Send the observer the initial conditions
      logger.newState (sys, y1);
      logger.endStep(SIM_STEP);
      logger.commit();
      obs(y1, 0.0);
      
      // Loop through all time steps and calculate new states
      for (t = 1; t <= tEnd; t++) {
         if (t%2 == 0) {
            // Use y1 as old and y2 as new
            // Simulate the dynamics
            sys(y1, y2, (double)t);
            // Log the state change
            logger.newState (sys, y2);
            logger.endStep(SIM_STEP);
            logger.commit();
            // Send result to the observer
            obs(y2, (double)t);
         }
         else {
            // Use y2 as old and y1 as new
            // Simulate the dynamics
            sys(y2, y1, (double)t);
            // Log the state change
            logger.newState (sys, y1);
            logger.endStep(SIM_STEP);
            logger.commit();
            // Send result to the observer
            obs(y1, (double)t);
         }
      }
      
      // Ensure the initial vector is updated to the final result
      initial = ( t%2 == 0 )  ? y1 : y2;
   }

   void SimulateOdeFixed::simulate (System &sys, double tMax, State &initial, SimObserver &obs, ChangeLog &logger) {
      
      // Check to ensure that initial conditions are correct size
      int states = (countNodes(sys)*sys.nodeStates()) + (countArcs(sys)*sys.arcStates());
      if (initial.size() < states || initial.size() > states) {
         cerr << "Incorrect number of states for initial conditions (SimulateOdeFixed::simulate)" << endl;
         return;
      }	 

      // Check that the state IDs are correct, if not refresh
      if (!sys.validStateIDs()) { sys.refreshStateIDs(); }

      // Create the required steppers
      typedef runge_kutta4<State> rk4_stepper_type;
      typedef adams_bashforth_moulton<5,State> adams_bash_moul_stepper_type;
      
      // Solve the system
      switch (mStepper) {
         case RK_4:
            integrate_const(rk4_stepper_type(),
                                    Simulator(&sys), initial, 0.0, tMax, mStepSize, ObserverPassThrough(sys, obs, logger));
            break;
         case ADAM_BASH_MOUL:
            integrate_const(adams_bash_moul_stepper_type(), 
                                    Simulator(&sys), initial, 0.0, tMax, mStepSize, ObserverPassThrough(sys, obs, logger));
            break;
         default:
            // Do nothing
            break;
      }
   }


   void SimulateOdeConst::simulate (System &sys, double tMax, State &initial, SimObserver &obs, ChangeLog &logger) {
      
      // Check to ensure that initial conditions are correct size
      int states = (countNodes(sys)*sys.nodeStates()) + (countArcs(sys)*sys.arcStates());
      if (initial.size() < states || initial.size() > states) {
         cerr << "Incorrect number of states for initial conditions (SimulateOdeConst::simulate)" << endl;
         return;
      }	

      // Check that the state IDs are correct, if not refresh
      if (!sys.validStateIDs()) { sys.refreshStateIDs(); }
      
      // Create the required steppers
      typedef runge_kutta_cash_karp54<State> rkck54_error_stepper_type;
      typedef runge_kutta_dopri5<State> dopri5_error_stepper_type;      
      
      // Solve the system
      switch (mStepper) {
         case RK_CASH_KARP_54:
            integrate_const(make_controlled( mEpsAbs , mEpsRel , rkck54_error_stepper_type() ), 
                                    Simulator(&sys), initial, 0.0, tMax, mOutputStep, ObserverPassThrough(sys, obs, logger));
            break;
         case RK_DOPRI_5:
            integrate_const(make_controlled( mEpsAbs , mEpsRel , dopri5_error_stepper_type() ), 
                                    Simulator(&sys), initial, 0.0, tMax, mOutputStep, ObserverPassThrough(sys, obs, logger));
            break;
         case RK_DOPRI_5_DENSE:
            integrate_const(make_dense_output( mEpsAbs , mEpsRel , dopri5_error_stepper_type() ),
                                    Simulator(&sys), initial, 0.0, tMax, mOutputStep, ObserverPassThrough(sys, obs, logger));
            break;
         default:
            // Do nothing
            break;
      }
   }

   void SimulateOdeAdaptive::simulate (System &sys, double tMax, State &initial, SimObserver &obs, ChangeLog &logger) {
      
      // Check to ensure that initial conditions are correct size
      int states = (countNodes(sys)*sys.nodeStates()) + (countArcs(sys)*sys.arcStates());
      if (initial.size() < states || initial.size() > states) {
         cerr << "Incorrect number of states for initial conditions (SimulateOdeAdaptive::simulate)" << endl;
         return;
      }	

      // Check that the state IDs are correct, if not refresh
      if (!sys.validStateIDs()) { sys.refreshStateIDs(); }
      
      // Create the required steppers
      typedef runge_kutta_cash_karp54<State> rkck54_error_stepper_type;
      typedef runge_kutta_dopri5<State> dopri5_error_stepper_type;     
      
      // Solve the system
      switch (mStepper) {
         case RK_CASH_KARP_54:
            integrate_adaptive(make_controlled( mEpsAbs , mEpsRel , rkck54_error_stepper_type() ), 
                                       Simulator(&sys), initial, 0.0, tMax, mInitialStep, ObserverPassThrough(sys, obs, logger));
            break;
         case RK_DOPRI_5:
            integrate_adaptive(make_controlled( mEpsAbs , mEpsRel , dopri5_error_stepper_type() ),
                                       Simulator(&sys), initial, 0.0, tMax, mInitialStep, ObserverPassThrough(sys, obs, logger));
            break;
         case RK_DOPRI_5_DENSE:
            integrate_adaptive(make_dense_output( mEpsAbs , mEpsRel , dopri5_error_stepper_type() ),
                                       Simulator(&sys), initial, 0.0, tMax, mInitialStep, ObserverPassThrough(sys, obs, logger));
            break;
         default:
            // Do nothing
            break;
      }
   }

} // netevo namespace
