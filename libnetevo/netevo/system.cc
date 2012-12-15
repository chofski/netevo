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

#include "system.h"
#include <iostream>
#include <fstream>
#include <ctime>
#include "gml.h"

namespace netevo {

   void System::operator() (const State &x, State &dx, const double t) {
      // Each vertex updates itself
      if (mNodeStates > 0) {
         for (System::NodeIt v(*this); v != INVALID; ++v) {
            (*mNodeData)[v].dynamic->fn(v, *this, x, dx, t);
         }
      }
      // Each edge updates itself
      if (mArcStates > 0) {
         for (System::ArcIt e(*this); e != INVALID; ++e) {
            (*mArcData)[e].dynamic->fn(e, *this, x, dx, t);
         }
      }        
   }

   void System::addNodeDynamic (NodeDynamic *nodeDynamic) {
      mNodeDynamics.insert( pair<string,NodeDynamic*>(nodeDynamic->getName(), nodeDynamic) );
      if (nodeDynamic->getStates() > mNodeStates) { mNodeStates = nodeDynamic->getStates(); }
   }

   void System::addArcDynamic (ArcDynamic *arcDynamic) {
      mArcDynamics.insert( pair<string,ArcDynamic*>(arcDynamic->getName(), arcDynamic) );
      if (arcDynamic->getStates() > mArcStates) { mArcStates = arcDynamic->getStates(); }
   }

   Node System::addNode (string dynamic) {
      NodeDynamic *dyn = mNodeDynamics.find(dynamic)->second;
      Node v = Parent::addNode();
      (*mNodeData)[v].key = mNextKey;
      mNextKey++;
      (*mNodeData)[v].position.x = 0.0;
      (*mNodeData)[v].position.y = 0.0;
      (*mNodeData)[v].position.z = 0.0;
      (*mNodeData)[v].dynamic = dyn;
      (*mNodeData)[v].dynamicParams.clear();
      dyn->setDefaultParams(v, *this);
      mValidNodeIDs = false;
      return v;
   }

   Node System::addNode (string name, string dynamic) {
      Node v = addNode(dynamic);
      (*mNodeData)[v].name = name;
      return v;
   }

   Arc System::addArc (Node u, Node v, string dynamic) {
      ArcDynamic *dyn = mArcDynamics.find(dynamic)->second;
      Arc e = Parent::addArc(u,v);
      (*mArcData)[e].weight = 1.0;
      (*mArcData)[e].dynamic = dyn;
      (*mArcData)[e].dynamicParams.clear();
      dyn->setDefaultParams(e, *this);
      mValidArcIDs = false;
      return e;
   }

   Arc System::addArc (Node u, Node v, string name, string dynamic) {
      Arc e = addArc(u, v, dynamic);
      (*mArcData)[e].name = name;
      return e;
   }

   Edge System::addEdge (Node u, Node v, string dynamic) {
      Arc a1 = addArc(v, u, dynamic);
      Arc a2 = addArc(u, v, dynamic);
      return pair<Arc,Arc>(a1,a2);
   }

   Edge System::addEdge (Node u, Node v, string name, string dynamic) {
      Arc a1 = addArc(v, u, name, dynamic);
      Arc a2 = addArc(u, v, name, dynamic);
      return pair<Arc,Arc>(a1,a2);
   }
   
   Node System::getNode (int ID) {
      // Iterate through (change to map in future)
      System::NodeIt v(*this);
      for (int i=0; i<ID; ++i) {
         ++v;
      }
      return (Node)v;
   }
   
   Arc System::getArc (int ID) {
      // Iterate through (change to map in future)
      System::ArcIt e(*this);
      for (int i=0; i<ID; ++i) {
         ++e;
      }
      return (Arc)e;
   }

   void System::resetKeys () {
      mNextKey = 0;
      for (System::NodeIt v(*this); v != INVALID; ++v) {
         (*mNodeData)[v].key = mNextKey;
         mNextKey++;
      }
   }

   int System::saveToGML (string filename) {
      int i, j;
      std::map<Node,int> nodeMap;
      std::ofstream fileOut;
      fileOut.open(filename.c_str());

      // Check that the file has opened successfully
      if (!fileOut.is_open()) {
         return 1;
      }

      // Generate a formatted time
      time_t rawtime;
      struct tm * timeinfo;
      char buffer [80];
      time(&rawtime);
      timeinfo = localtime(&rawtime);
      strftime(buffer, 80, " on %c", timeinfo);

      // Write the general header and start graph
      fileOut << "Creator \"NetEvo 2.0.0" << buffer << "\"" << endl;
      fileOut << "graph [" << endl;

      // Write directed flag
      fileOut << " directed 1" << endl;

      // Write the nodes
      i = 0;
      for (System::NodeIt v(*this); v != INVALID; ++v) {
         nodeMap[v] = i;
         fileOut << " node [" << endl;
         fileOut << "  id " << i << endl;
         fileOut << "  key " << (*mNodeData)[v].key << endl;
         fileOut << "  label \"" << (*mNodeData)[v].name << "\"" << endl;
         fileOut << "  graphics [" << " x " << (*mNodeData)[v].position.x << " y " << 
         (*mNodeData)[v].position.y << " z " << (*mNodeData)[v].position.z << " ]" << endl;
         
         // Build properties list
         fileOut << "  properties \"";
         for (j=0; j<(*mNodeData)[v].properties.size(); j++) {
            if (j>0) { fileOut << ","; }
            fileOut << (*mNodeData)[v].properties[j];
         }
         fileOut << "\"" << endl;
         fileOut << "  dynName \"" << (*mNodeData)[v].dynamic->getName() << "\"" << endl;
         
         // Build list of params
         fileOut << "  dynParams \"";
         for (j=0; j<(*mNodeData)[v].dynamicParams.size(); j++) {
            if (j>0) { fileOut << ","; }
            fileOut << (*mNodeData)[v].dynamicParams[j];
         }
         fileOut << "\"" << endl;
         fileOut << " ]" << endl;
         i++;
      }

      // Write the edges
      for (System::ArcIt e(*this); e != INVALID; ++e) {
         fileOut << " edge [" << endl;
         fileOut << "  source " << nodeMap[source(e)] << endl;
         fileOut << "  target " << nodeMap[target(e)] << endl;
         fileOut << "  label \"" << (*mArcData)[e].name << "\"" << endl;
         fileOut << "  weight " << (*mArcData)[e].weight << endl;
         
         // Build properties list
         fileOut << "  properties \"";
         for (j=0; j<(*mArcData)[e].properties.size(); j++) {
            if (j>0) { fileOut << ","; }
            fileOut << (*mArcData)[e].properties[j];
         }
         fileOut << "\"" << endl;
         fileOut << "  dynName \"" << (*mArcData)[e].dynamic->getName() << "\"" << endl;
         
         // Build list of params
         fileOut << "  dynParams \"";
         for (j=0; j<(*mArcData)[e].dynamicParams.size(); j++) {
            if (j>0) { fileOut << ","; }
            fileOut << (*mArcData)[e].dynamicParams[j];
         }
         fileOut << "\"" << endl;
         fileOut << " ]" << endl;
      }

      // End the file and close
      fileOut << "]" << endl;
      fileOut.close();

      return 0;
   }

   int System::openFromGML (string filename) {
      GML_stat stat;
      stat.key_list = NULL;
      GML_pair* key_list;
      GML_pair* orig_list;
      bool directed;
      
      FILE* file = fopen(filename.c_str(), "r");
      
      // Check that file opened successfully
      if (!file) {
         return 1;
      } 

      GML_init();
      key_list = GML_parser (file, &stat, 0);
      fclose(file);

      if (stat.err.err_num != GML_OK) {
         GML_free_list (key_list, stat.key_list);
         return 2;
      }

      //
      // This file is a valid GML-file, let's build the graph.
      // 

      // TODO: clear the graph of existing contents
      //clear();
      orig_list = key_list;


      //
      // get the first entry with key "graph" in the list
      // 

      while (key_list) {
         if (!strcmp ( "graph", key_list->key)) { break; }
         key_list = key_list->next;
      }

      assert (key_list);

      key_list = key_list->value.list;
      GML_pair* graph_list = key_list;

      GML_pair* tmp_list;
      GML_pair* tmp_prev;

      list<pair<int,GML_pair*> > node_entries;
      list<pair<pair<int,int>,GML_pair*> > edge_entries; 
      
      int num_nodes = 0; 
      bool target_found;
      bool source_found;

      //
      // Node and edge keys may come in arbitrary order, so sort them such
      // that all nodes come before all edges.
      //

      while (key_list) {
         if (!strcmp (key_list->key, "node")) {

            //
            // Search the list associated with this node for the id
            //

            assert (key_list->kind == GML_LIST);
            tmp_list = key_list->value.list;
            tmp_prev = 0;
            pair<int,GML_pair*> n;
            n.second = tmp_list;

            while (tmp_list) {
               if (!strcmp (tmp_list->key, "id")) {
                  assert (tmp_list->kind == GML_INT);
                  n.first = tmp_list->value.integer;
                  break;
               }
               tmp_list = tmp_list->next;
            }

            assert (tmp_list);
            node_entries.push_back(n);
            ++num_nodes;

         } else if (!strcmp (key_list->key, "edge")) {

            //
            // Search for source and target entries
            //

            assert (key_list->kind == GML_LIST);
            tmp_list = key_list->value.list;
            tmp_prev = 0;
            source_found = false;
            target_found = false;
            pair<pair<int,int>,GML_pair*> e;
            e.second = tmp_list;

            while (tmp_list) {
               if (!strcmp (tmp_list->key, "source")) {
                  assert (tmp_list->kind == GML_INT);
                  source_found = true;
                  e.first.first = tmp_list->value.integer;
                  if (target_found) break;

               } else if (!strcmp (tmp_list->key, "target")) {
                  assert (tmp_list->kind == GML_INT);
                  target_found = true;
                  e.first.second = tmp_list->value.integer;
                  if (source_found) break;
               }
               tmp_list = tmp_list->next;
            }

            assert (source_found && target_found);
            edge_entries.push_back (e);

         } else if (!strcmp (key_list->key, "directed")) {

         // At present we ignore any directed flag (all graphs are directed)
         directed = (key_list->value.integer != 0);
         }	

         key_list = key_list->next;
      }

      //
      // make this graph the graph decribed in list
      //

      map<int, Node> id_2_node;
      Node source, target;
      Node tmp_node;
      Arc tmp_edge;
      list<pair<int,GML_pair*> >::iterator it, end;
      vector<int> node_ids;
      node_ids.reserve(num_nodes);

      for (it = node_entries.begin(), end = node_entries.end();
      it != end; ++it) {

         tmp_node = addNode();
         id_2_node[(*it).first] = tmp_node;
         NodeData &nData = (*mNodeData)[tmp_node];
         
         key_list = (*it).second;
         while (key_list) {
            if (!strcmp (key_list->key, "label")) {
               assert (key_list->kind == GML_STRING);
               nData.name = string(key_list->value.string);
            }
            else if (!strcmp (key_list->key, "key")) {
               if (key_list->kind == GML_DOUBLE) {
                  nData.key = (int)(key_list->value.floating);
               } else if (key_list->kind == GML_INT) {
               nData.key = (int)(key_list->value.integer);
               }
            }
            else if (!strcmp (key_list->key, "graphics")) {
               assert (key_list->kind == GML_LIST);
               tmp_list = key_list->value.list;
               while (tmp_list) {
                  if (!strcmp (tmp_list->key, "x")) {
                     if (tmp_list->kind == GML_DOUBLE) {
                        nData.position.x = (double)(tmp_list->value.floating);
                     } else if (tmp_list->kind == GML_INT) {
                        nData.position.x = (double)(tmp_list->value.integer);
                     }
                  }
                  if (!strcmp (tmp_list->key, "y")) {
                     if (tmp_list->kind == GML_DOUBLE) {
                        nData.position.y = (double)(tmp_list->value.floating);
                     } else if (tmp_list->kind == GML_INT) {
                        nData.position.y = (double)(tmp_list->value.integer);
                     }
                  }
                  if (!strcmp (tmp_list->key, "z")) {
                     if (tmp_list->kind == GML_DOUBLE) {
                        nData.position.z = (double)(tmp_list->value.floating);
                     } else if (tmp_list->kind == GML_INT) {
                        nData.position.z = (double)(tmp_list->value.integer);
                     }
                  }
                  tmp_list = tmp_list->next;
               }
            }
            else if (!strcmp (key_list->key, "properties")) {
               nData.properties.clear();
               assert (key_list->kind == GML_STRING);
               // Tokenize the string
               stringstream ss(key_list->value.string);
               string s;
               while (getline(ss, s, ',')) {
                  // Convert the entry to a double and add to properties
                  istringstream stm;
                  stm.str(s);
                  double d;
                  stm >> d;
                  nData.properties.push_back(d);
               }
            }
            else if (!strcmp (key_list->key, "dynName")) {
               assert (key_list->kind == GML_STRING);
               nData.dynamic = mNodeDynamics[string(key_list->value.string)];
            }
            else if (!strcmp (key_list->key, "dynParams")) {
               assert (key_list->kind == GML_STRING);
               nData.dynamicParams.clear();
               // Tokenize the string
               stringstream ss(key_list->value.string);
               string s;
               while (getline(ss, s, ',')) {
                  // Convert the entry to a double and add to properties
                  istringstream stm;
                  stm.str(s);
                  double d;
                  stm >> d;
                  nData.dynamicParams.push_back(d);
               }
            }

            // Move to next attribute value pair
            key_list = key_list->next;
         }
      }

      // Make sure the next key is large enough.
      int curKey = 0;
      for (System::NodeIt v(*this); v != INVALID; ++v) {
         (*mNodeData)[v].key = curKey;
         if (mNextKey<curKey) { mNextKey = curKey; }
      }
      mNextKey++;
   
      list<pair<pair<int,int>,GML_pair*> >::iterator eit, eend;
      for (eit = edge_entries.begin(), eend = edge_entries.end();
      eit != eend; ++eit) {
         source = id_2_node[(*eit).first.first];
         target = id_2_node[(*eit).first.second];

         // Handle adding the edge
         tmp_edge = addArc(source, target);
         ArcData &eData = (*mArcData)[tmp_edge];
         
         key_list = (*eit).second;
         while (key_list) {
            if (!strcmp (key_list->key, "label")) {
               assert (key_list->kind == GML_STRING);
               eData.name = string(key_list->value.string);
            }
            else if (!strcmp (key_list->key, "weight")) {
               if (key_list->kind == GML_DOUBLE) {
                  eData.weight = (double)(key_list->value.floating);
               } else if (key_list->kind == GML_INT) {
                  eData.weight = (double)(key_list->value.integer);
               }
            }
            else if (!strcmp (key_list->key, "properties")) {
               eData.properties.clear();
               assert (key_list->kind == GML_STRING);
               // Tokenize the string
               stringstream ss(key_list->value.string);
               string s;
               while (getline(ss, s, ',')) {
                  // Convert the entry to a double and add to properties
                  istringstream stm;
                  stm.str(s);
                  double d;
                  stm >> d;
                  eData.properties.push_back(d);
               }
            }
            else if (!strcmp (key_list->key, "dynName")) {
               assert (key_list->kind == GML_STRING);
               eData.dynamic = mArcDynamics[string(key_list->value.string)]; 
            }
            else if (!strcmp (key_list->key, "dynParams")) {
               assert (key_list->kind == GML_STRING);
               eData.dynamicParams.clear();
               // Tokenize the string
               stringstream ss(key_list->value.string);
               string s;
               while (getline(ss, s, ',')) {
                  // Convert the entry to a double and add to properties
                  istringstream stm;
                  stm.str(s);
                  double d;
                  stm >> d;
                  eData.dynamicParams.push_back(d);
               }
            }

            // Move to next attribute value pair
            key_list = key_list->next;
         }

      }
      
      // Update the state ID mappings
      refreshStateIDs();

      GML_free_list (orig_list, stat.key_list);
      stat.err.err_num = GML_OK;
      return 0;
   }

   void System::randomGraph (double edgeProb, int numOfNodes, bool selfLoops, bool undirected) {
      randomGraph(edgeProb, numOfNodes, selfLoops, "NoNodeDynamic", "NoArcDynamic", undirected);
   }

   void System::randomGraph (double edgeProb, int numOfNodes, bool selfLoops, string defNodeDyn, string defEdgeDyn, bool undirected) {
      // Clear any existing structure
      clear();
      
      // Create the required number of nodes 
      for (int i=0; i<numOfNodes; ++i) {
         addNode(defNodeDyn);
      }
      
      // Loop through all possible edges and add with given probability
      for (System::NodeIt n1(*this); n1 != INVALID; ++n1) {
         for (System::NodeIt n2(*this); n2 != INVALID; ++n2) {
            if ((n1 == n2 && selfLoops) || n1 != n2) {
               if (mRnd() <= edgeProb) {
                  if (undirected) {
                     // Check to make sure arc in other direction does not already exist
                     if (findArc(*this, n1, n2) == INVALID) {
                        addArc(n1, n2, defEdgeDyn);
                        addArc(n2, n1, defEdgeDyn);
                     }
                  }
                  else {
                     addArc(n1, n2, defEdgeDyn);
                  }
               }
            }
         }
      }
      
      // Update the state ID mapping
      refreshStateIDs();
   }
   
   void System::ringGraph (int numOfNodes, int neighbours, bool undirected) {
      ringGraph(numOfNodes, neighbours, "NoNodeDynamic", "NoArcDynamic", undirected);
   }
   
   void System::ringGraph (int numOfNodes, int neighbours, string defNodeDyn, string defEdgeDyn, bool undirected) {
      int i, j;
      
      // Clear any existing structure
      clear();
      
      // Create the required number of nodes 
      for (int i=0; i<numOfNodes; ++i) {
         addNode(defNodeDyn);
      }
      
      i = 0;
      for (System::NodeIt v(*this); v != INVALID; ++v) {
         for (j=i+1; j<=i+neighbours; ++j) {
            if (undirected) {
               addEdge(v, getNode(j%numOfNodes), defEdgeDyn);
            }
            else {
               addArc(v, getNode(j%numOfNodes), defEdgeDyn);
            }
         }
         ++i;
      }
      
      // Update the state ID mapping
      refreshStateIDs();
   }

   void System::makeUndirected () {
      for (System::ArcIt e(*this); e != INVALID; ++e) {
         // Check to make sure arc in other direction does not already exist
         if (findArc(*this, target(e), source(e)) == INVALID) {
            // Create the arc and copy the data
            Arc eNew = addArc(target(e), source(e));
            ArcData &eData = (*mArcData)[e];
            ArcData &eNewData = (*mArcData)[eNew];
            eNewData.name = eData.name;
            eNewData.weight = eData.weight;
            eNewData.properties = eData.properties;
            eNewData.dynamic = eData.dynamic;
            eNewData.dynamicParams = eData.dynamicParams;
         }
      }
   }
   
   int System::weaklyConnectedComponents () {
      Undirector<System> ug(*this);
      return countConnectedComponents(ug);
   }
   
   void System::fillAdjacency (MatrixXd &A) {
      
      // Create map of node to int
      map<Node, int> nToID;
      int i = 0;
      for (System::NodeIt n(*this); n != INVALID; ++n) {
         nToID[n] = i;
         ++i;
      }
      
      // Fill all terms
      for (System::ArcIt a(*this); a != INVALID; ++a) {
         A(nToID[source(a)], nToID[target(a)]) = 1;
      }
   }

   void System::fillLaplacian (MatrixXd &A) {
      
      // Create map of node to int
      map<Node, int> nToID;
      int i = 0;
      for (System::NodeIt n(*this); n != INVALID; ++n) {
         nToID[n] = i;
         ++i;
      }

      // Fill the off diagonal terms
      for (System::ArcIt a(*this); a != INVALID; ++a) {
         A(nToID[source(a)], nToID[target(a)]) = 1;
      }

      // Fill the diagonal terms
      for (System::NodeIt n(*this); n != INVALID; ++n) {
         int j = nToID[n];
         A(j, j) = -countOutArcs(*this, n);
      }
   }

   VectorXcd System::eigenvalues (int mType) {
      
      // Create the matrix
      int mSize = countNodes(*this);
      MatrixXd A = MatrixXd::Zero(mSize,mSize);
      if (mType == 0) {
         fillLaplacian(A);
      }
      else {
         fillAdjacency(A);
      }

      // Solve the system and return the eigenvalues
      EigenSolver<MatrixXd> es(A, false);
      return es.eigenvalues();
   }

   pair<VectorXcd, MatrixXcd> System::eigensystem (int mType) {
      
      // Create the Laplacian matrix
      int mSize = countNodes(*this);
      MatrixXd A = MatrixXd::Zero(mSize,mSize);
      if (mType == 0) {
         fillLaplacian(A);
      }
      else {
         fillAdjacency(A);
      }

      // Solve the system and return the eigensystem
      EigenSolver<MatrixXd> es(A, true);
      return pair<VectorXcd, MatrixXcd>(es.eigenvalues(), es.eigenvectors());
   }

   bool System::validStateIDs () {
      return (mValidNodeIDs && mValidArcIDs);
   }

   void System::refreshStateIDs () {
      int i = 0;
      
      // Update node IDs
      if (!mValidNodeIDs) {
         i = 0;
         for (NodeIt v(*this); v != INVALID; ++v) {
            (*mNodeIDs)[v] = i;
            ++i;
         }
      }
      
      // Update arc IDs
      if (!mValidArcIDs) {
         i = 0;
         for (ArcIt e(*this); e != INVALID; ++e) {
            (*mArcIDs)[e] = i;
            ++i;
         }
      }
   }

   int System::stateID (Node v) {
      return (mNodeStates * (*mNodeIDs)[v]);
   }

	int System::stateID (Arc e) {
		return ((mNodeStates * countNodes(*this)) + (mArcStates * (*mArcIDs)[e]));
	}
   
   void ChangeLogSet::addChangeLog (ChangeLog *logger) {
      mLoggers.push_back(logger);
   }
   
   void ChangeLogSet::addNode (System &sys, Node n) {
      for (int i=0; mLoggers.size(); ++i) {
         mLoggers[i]->addNode(sys, n);
      }
   }
   
   void ChangeLogSet::addArc (System &sys, Node source, Node target) {
      for (int i=0; mLoggers.size(); ++i) {
         mLoggers[i]->addArc(sys, source, target);
      }
   }
   
   void ChangeLogSet::erase (System &sys, Node n) {
      for (int i=0; mLoggers.size(); ++i) {
         mLoggers[i]->erase(sys, n);
      }
   }
   
   void ChangeLogSet::erase (System &sys, Arc e) {
      for (int i=0; mLoggers.size(); ++i) {
         mLoggers[i]->erase(sys, e);
      }
   }
   
   void ChangeLogSet::update (System &sys, Node n) {
      for (int i=0; mLoggers.size(); ++i) {
         mLoggers[i]->update(sys, n);
      }
   }
   
   void ChangeLogSet::update (System &sys, Arc e) {
      for (int i=0; mLoggers.size(); ++i) {
         mLoggers[i]->update(sys, e);
      }
   }
   
   void ChangeLogSet::newState (System &sys, const State &newState) {
      for (int i=0; mLoggers.size(); ++i) {
         mLoggers[i]->newState(sys, newState);
      }
   }
   
   void ChangeLogSet::endStep (step_type_e stepType) {
      for (int i=0; mLoggers.size(); ++i) {
         mLoggers[i]->endStep(stepType);
      }
   }
   
   void ChangeLogSet::rollback () {
      for (int i=0; mLoggers.size(); ++i) {
         mLoggers[i]->rollback();
      }
   }
   
   void ChangeLogSet::commit () {
      for (int i=0; mLoggers.size(); ++i) {
         mLoggers[i]->commit();
      }
   }
   
   void ChangeLogToStream::addNode (System &sys, Node n) {
      buffer << "N+," << sys.nodeData(n).key << endl;
   }
   
   void ChangeLogToStream::addArc (System &sys, Node source, Node target) {
      buffer << "E+," << sys.nodeData(source).key << "," << sys.nodeData(target).key << endl;
   }
   
   void ChangeLogToStream::erase (System &sys, Node n) {
      buffer << "N-," << sys.nodeData(n).key << endl;
   }
   
   void ChangeLogToStream::erase (System &sys, Arc e) {
      Node source = sys.source(e);
      Node target = sys.target(e);
      buffer << "E-," << sys.nodeData(source).key << "," << sys.nodeData(target).key << endl;
   }
   
   void ChangeLogToStream::update (System &sys, Node n) {
      buffer << "NU," << sys.nodeData(n).key << endl;
   }
   
   void ChangeLogToStream::update (System &sys, Arc e) {
      Node source = sys.source(e);
      Node target = sys.target(e);
      buffer << "EU," << sys.nodeData(source).key << "," << sys.nodeData(target).key << endl;
   }
   
   void ChangeLogToStream::newState (System &sys, const State &newState) {
      int i, stateIndex;
      
      // Loop through each node and edge and output state
      if (sys.nodeStates() > 0) {
         for (System::NodeIt n(sys); n != INVALID; ++n) {
            buffer << "NS," << sys.nodeData(n).key;
            stateIndex = sys.stateID(n);
            for (i=0; i<sys.nodeStates(); ++i) {
               buffer << "," << newState[stateIndex + i];
            }
            buffer << endl;
         }
      }
      if (sys.arcStates() > 0) {
         for (System::ArcIt e(sys); e != INVALID; ++e) {
            Node source = sys.source(e);
            Node target = sys.target(e);
            buffer << "ES," << sys.nodeData(source).key << "," << sys.nodeData(target).key;
            stateIndex = sys.stateID(e);
            for (i=0; i<sys.arcStates(); ++i) {
               buffer << "," << newState[stateIndex + i];
            }
            buffer << endl;
         }
      }
   }
   
   void ChangeLogToStream::endStep (step_type_e stepType) { 
      switch (stepType) {
         case INIT_STEP:
            buffer << "---" << endl;
            break;
         case SIM_STEP:
            buffer << "-" << endl;
            break;
         case EVO_STEP:
            buffer <<  "--" << endl;
            break;
         default:
            // Do nothing
            break;
      }
   }
   
   void ChangeLogToStream::rollback () {
      
      // Clear the buffer
      buffer.clear(); 
      buffer.str("");
   }
   
   void ChangeLogToStream::commit () { 
      
      // Write the buffer to the output
      mOut << buffer.str(); 
      
      // Reset (clear) the buffer
      rollback();
   }
   
} // netevo namespace
