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

#ifndef NE_VISUALISE_H
#define NE_VISUALISE_H

#include "system.h"
#include <cinder/gl/Fbo.h>
#include <cinder/qtime/MovieWriter.h>
#include <cinder/Utilities.h>

// We use Cinder for visualisation
using namespace ci;

namespace netevo {
   
   /** Updates the positions of a systems nodes (the layout). */
   class Layout {
   public:
      /** Create an initial layout. */
      virtual void setInitialLayout (System &sys) { };
      /** New node has been added update initial position. */
      virtual void setInitialPosition (System &sys, Node v) { };
      /** Method called to interate the layout for a length of time. */
      virtual void iterate (System &sys, double dt) { };
   };
   
   /** Maps properties of a system to a visual representation. */
   class Drawer {
   public:
      virtual void draw (System &sys, gl::Fbo &framebuffer) { };		
   };
   
   /** Visualisation of a System. User must call appropriate generate frame methods and all
    *  content is rendered to an internal OpenGL framebuffer that can then be exported to the
    *  screen or written to file. */
   class Visualisation {
   protected:
      /** Time step between frames. */
      double  mDt;
      /** Layout for the visualisation. */
		Layout  *mLayout;
      /** Drawer for the visualisation. */
		Drawer  *mDrawer;
      /** Internal framebuffer for partial updates. */
      gl::Fbo *mFramebuffer;
      /** Internal framebuffer holding current rendered display. */
		gl::Fbo *mFramebufferDone;
      /** Width in pixels of framebuffer. */
      int mWidth;
      /** Height in pixels of framebuffer. */
      int mHeight;
      
      /** Utility function to swap the internal buffers. */
      void swapBuffers () {
			gl::Fbo *tempFb;
			tempFb = mFramebufferDone;
			mFramebufferDone = mFramebuffer;
			mFramebuffer = tempFb;
		}    

   public:
      /** Constructor for visualisation that will render directly to the screen. */
      Visualisation (System &sys, double dt, Layout *layout, Drawer *drawer, int width, int height, gl::Fbo::Format format) {
         mDt = dt;
			mLayout = layout;
			mDrawer = drawer;
         mWidth = width;
         mHeight = height;
         mState = new State(sys.totalStates(), 0.0);
         mFramebuffer = new gl::Fbo(width, height, format);
			mFramebufferDone = new gl::Fbo(width, height, format);
         // Create initial state
         mState = new State(sys.totalStates(), 0.0);         
         // Generate an initial layout
         mLayout->setInitialLayout(sys);
         // Render the initial layout to the framebuffer
         generateFrame();
      }
      
      /** Destructor for visualisation. Free internal framebuffers. */
      ~Visualisation () {
         delete(mFramebuffer);
			delete(mFramebufferDone);
      }
      
      /** Width in pixels of framebuffer. */
      int width () { return mWidth; }
      
      /** Height in pixels of framebuffer. */
      int height () { return mHeight; }
      
      /** Perform single iteration of layout and draw cycle. */
		void generateFrame (System &sys) {
			mLayout->iterate(mSys,  mDt);
			mDrawer->draw(*mSys, *mFramebuffer);
			swapBuffers();
		}
      
      /** Grabs the current finished framebuffer. */
		gl::Fbo * getFramebuffer () { return mFramebufferDone; }
   };
   
   /** Renders a visualisation to a movie file. */
   class VisualisationToMovie {
   protected:
      string mFilename;
      qtime::MovieWriter::Format mFormat;
      qtime::MovieWriter *mMovie;
      
   public:
      /** Constructor that uses a default codec (PNG). */
      VisualisationToMovie (string filename) : mFilename(filename) {
         // Default codec and quality
         mFormat.setCodec(qtime::MovieWriter::CODEC_PNG);
         mFormat.setQuality(0.8f);
      }
      
      /** Constructor to use a specified codec. */
      VisualisationToMovie (string filename, qtime::MovieWriter::Format format) : mFilename(filename), mFormat(format) { }
      
      /** Create the movie file. */
      void start (Visualisation &vis) {
         mMovie = new qtime::MovieWriter(mFilename, vis.width(), vis.height(), mFormat);
      }
      
      /** Record a single frame. */
      void output (Visualisation &vis) {
         movie.addFrame(vis.getFramebuffer().getTexture());
      }
      
      /** Finish the movie. */
      void finish (Visualisation &vis) { mMovie->finish(); }
   };
   
   /** Change log for saving output to a movie file. */
   class ChangeLogToMovie : public ChangeLog {
   private:
      double mDt;
      double mSimDt;
      double mEvoDt;
      double mInitDt;
      double mCurDt;
      VisualisationToMovie *mOut;
      Visualisation        *mVis;
   public:
      ChangeLogToMovie (System &sys, string filename, Layout *layout, Drawer *drawer, double simDt, double evoDt, double initDt) {
         mDt = 1.0/30.0;
         mSimDt = simDt;
         mEvoDt = evoDt;
         mInitDt = initDt;
         mOut = new VisualisationToMovie(filename);
         mVis = new Visualisation(sys, mDt, layout, drawer, 800, 600, gl::Fbo::Format());
      }
      
      void start  () { mOut->start(); }
      void finish () { mOut->finish(); }
      
      void addNode  (System &sys, Node n) { }
      void addArc   (System &sys, Node source, Node target) { }
      void erase    (System &sys, Node n) { }
      void erase    (System &sys, Arc e) { }
      void update   (System &sys, Node n) { }
      void update   (System &sys, Arc e) { }
      void newState (System &sys, const State &newState) { }
      
      void endStep  (System &sys, step_type_e stepType) {
         switch (stepType) {
            case INIT_STEP:
               mCurDt = mInitDt;
               break;
            case SIM_STEP:
               mCurDt = mSimDt;
               break;
            case EVO_STEP:
               mCurDt = mEvoDt;
               break;
            default:
               // Do nothing
               break;
         }
      }
      
      void rollback (System &sys) { mCurDt = mDt; }
      
      void commit   (System &sys) { 
         // Generate the number of frames required
         for (int i=0; i<(int)(mCurDt/mDt); ++i) {
            // Generate a frame and output to movie
            mVis->generateFrame(sys);
            mOut->output(*mVis);
         }
      }
   };
   
} // netevo namespace

#endif // NE_VISUALISE_H
