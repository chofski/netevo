#!/bin/bash

# PATH TO HEADER FILES
CXXFLAGS="-I /Users/Tom/Development/Projects/libnetevo/netevo -I /Users/Tom/Development/Library/include -I /Users/enteg/Development/Projects/libnetevo/netevo -I /Users/enteg/Development/Library/include"

# PATH TO LIBRARIES
LD_LIBRARY_PATH="-L /Users/Tom/Development/Library/lib"

################################################
# EXAMPLES (Uncomment one to compile)
################################################

# SYSTEM RELATED FUNCTIONS
#g++ $CXXFLAGS $LD_LIBRARY_PATH ../netevo/evolve_sa.cc ../netevo/evolve.cc ../netevo/gml.cc ../netevo/simulate.cc ../netevo/system.cc systems.cc -o systems -lemon

# SIMULATE NEWORK OF MAPPINGS
#g++ $CXXFLAGS $LD_LIBRARY_PATH ../netevo/evolve_sa.cc ../netevo/evolve.cc ../netevo/gml.cc ../netevo/simulate.cc ../netevo/system.cc simulate_map.cc -o simulate_map -lemon

# SIMULATE NETWORK OF ODES
#g++ $CXXFLAGS $LD_LIBRARY_PATH ../netevo/evolve_sa.cc ../netevo/evolve.cc ../netevo/gml.cc ../netevo/simulate.cc ../netevo/system.cc simulate_ode.cc -o simulate_ode -lemon

# EVOLVE SIMULATED ANNEALING - TOPOLOGY
#g++ $CXXFLAGS $LD_LIBRARY_PATH ../netevo/evolve_sa.cc ../netevo/evolve.cc ../netevo/gml.cc ../netevo/simulate.cc ../netevo/system.cc evolve_sa_top.cc -o evolve_sa_top -lemon

# EVOLVE SIMULATED ANNEALING - DYNAMICS
g++ $CXXFLAGS $LD_LIBRARY_PATH ../netevo/evolve_sa.cc ../netevo/evolve.cc ../netevo/gml.cc ../netevo/simulate.cc ../netevo/system.cc evolve_sa_dyn.cc -o evolve_sa_dyn -lemon
