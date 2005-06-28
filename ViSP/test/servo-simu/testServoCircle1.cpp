

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Copyright Projet Lagadic / IRISA-INRIA Rennes, 2005
 * www  : http://www.irisa.fr/lagadic
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * File:      testServoSphere1.cpp
 * Project:   ViSP 2.0
 * Author:    Eric Marchand
 *
 * Version control
 * ===============
 *
 *  $Id: testServoCircle1.cpp,v 1.2 2005-06-28 09:04:05 marchand Exp $
 *
 * Description
 * ============
 *   tests the control law
 *   eye-in-hand control
 *   velocity computed in the camera frame
 *
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


/*!
  \example testServoCircle1.cpp
  \brief servo a circle
*/


#include <visp/vpMath.h>
#include <visp/vpHomogeneousMatrix.h>
#include <visp/vpFeatureEllipse.h>
#include <visp/vpCircle.h>
#include <visp/vpServo.h>
#include <visp/vpRobotCamera.h>
#include <visp/vpFeatureBuilder.h>


// Exception
#include <visp/vpException.h>
#include <visp/vpMatrixException.h>

// Debug trace
#include <visp/vpDebug.h>


int
main()
{
  vpServo task ;
  vpRobotCamera robot ;

  cout << endl ;
  cout << "-------------------------------------------------------" << endl ;
  cout << " Test program for vpServo "  <<endl ;
  cout << " Simulation " << endl ;
  cout << " task : servo a circle " << endl ;
  cout << "-------------------------------------------------------" << endl ;
  cout << endl ;


  TRACE("sets the initial camera location " ) ;
  vpHomogeneousMatrix cMo ;
  cMo[0][3] = 0.1 ;
  cMo[1][3] = 0.2 ;
  cMo[2][3] = 2 ;
  robot.setPosition(cMo) ;

  vpHomogeneousMatrix cMod ;
  cMod[0][3] = 0 ;
  cMod[1][3] = 0 ;
  cMod[2][3] = 1 ;



  TRACE("sets the circle coordinates in the world frame "  ) ;
  vpCircle circle ;
  circle.setWorldCoordinates(0,0,1,0,0,0,0.1) ;

  TRACE("sets the desired position of the visual feature ") ;
  vpFeatureEllipse pd ;
  circle.track(cMod) ;
  vpFeatureBuilder::create(pd,circle)  ;

  TRACE("project : computes  the circle coordinates in the camera frame and its 2D coordinates"  ) ;

  TRACE("sets the current position of the visual feature ") ;
  vpFeatureEllipse p ;
  circle.track(cMo) ;
  vpFeatureBuilder::create(p,circle)  ;

  TRACE("define the task") ;
  TRACE("\t we want an eye-in-hand control law") ;
  TRACE("\t robot is controlled in the camera frame") ;
  task.setServo(vpServo::EYEINHAND_CAMERA) ;

  TRACE("\t we want to see a circle on a circle..") ;
  cout << endl ;
  task.addFeature(p,pd) ;

  TRACE("\t set the gain") ;
  task.setLambda(0.1) ;


  TRACE("Display task information " ) ;
  task.print() ;

  int iter=0 ;
  TRACE("\t loop") ;
  while(iter++<500)
  {
    cout << "---------------------------------------------" << iter <<endl ;
    vpColVector v ;

    if (iter==1) TRACE("\t\t get the robot position ") ;
    robot.getPosition(cMo) ;
    if (iter==1) TRACE("\t\t new circle position ") ;
    //retrieve x,y and Z of the vpCircle structure

    circle.track(cMo) ;
    vpFeatureBuilder::create(p,circle);

    if (iter==1) TRACE("\t\t compute the control law ") ;
    v = task.computeControlLaw() ;
    //  TRACE("computeControlLaw" ) ;
    cout << task.rankJ1 <<endl ;
    if (iter==1) TRACE("\t\t send the camera velocity to the controller ") ;
    robot.setVelocity(vpRobot::CAMERA_FRAME, v) ;
 

    //  TRACE("\t\t || s - s* || ") ;
    //  cout << task.error.sumSquare() <<endl ; ;
  }

  TRACE("Display task information " ) ;
  task.print() ;
}

