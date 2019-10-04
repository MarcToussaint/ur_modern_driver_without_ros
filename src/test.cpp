/*
 * Copyright 2017, 2018 Jarek Potiuk (low bandwidth trajectory follower)
 *
 * Copyright 2017, 2018 Simon Rasmussen (refactor)
 *
 * Copyright 2015, 2016 Thomas Timm Andersen (original version)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <thread>
#include <fstream>
#include <math.h>
#include <iostream>
#include "ur10driver.h"

void minimalMotionToHome(UR10Driver& driver){
  std::ofstream file("z.qref");

  //-- generate trivial homing trajectory


  std::array<double, 6> qhome = {-1, -1.7, -1.3, -1.7, 1.6, 0};
  std::array<double, 6> q0 = driver.get_q_now();
  std::array<double, 6> v0;
  for(uint i=0;i<v0.size();i++) v0[i]=0.;

  std::vector<TrajectoryPoint> trajectory;
  trajectory.resize(2);
  trajectory[0] = { q0, v0, std::chrono::microseconds((unsigned long)(0. *1000000.))};
  trajectory[1] = { qhome, v0, std::chrono::microseconds((unsigned long)(2. *1000000.))};

  std::atomic<bool> interrupt (false);
  driver.executeTrajectory(trajectory, interrupt);
}


void marcTest(UR10Driver& driver){
  //-- wait for initialization
  for(;;){
    if(driver.isInitialized()) break;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  std::cout <<"** q is initialized" <<std::endl;

  std::ofstream file("z.qref");

  //-- generate a trajectory

  std::array<double, 6> qhome = {-1, -1.7, -1.3, -1.7, 1.6, 0};
  std::array<double, 6> q0 = driver.get_q_now();

  std::array<double, 6> v0;
  for(uint i=0;i<v0.size();i++) v0[i]=0.;

  std::vector<TrajectoryPoint> trajectory;
  uint T=20;
  double duration = .5;
  double L=2;
  double amplitude = 1.;
  trajectory.resize(T*L+T); //ten tail configurations, to ensure resting
  for(uint t=0;t<trajectory.size();t++){
    std::array<double, 6> q = q0;
    std::array<double, 6> v = v0;
    if(t<T*L){
      double phase = 2.*M_PI*t/double(T);
      q[3] += amplitude * 0.5*(1.-::cos(phase));
      v[3] = amplitude * (0.5*sin(phase)) * 2.*M_PI/duration;
    }else{
      q = qhome;
    }

    double time = duration*t/double(T);
    trajectory[t] = {q, v0, std::chrono::microseconds((unsigned long)(time*1000000.))};
    file <<1.2+time;
    for(uint i=0;i<6;i++) file <<' ' <<q[i];
    for(uint i=0;i<6;i++) file <<' ' <<v[i];
    file <<std::endl;
  }

  std::atomic<bool> interrupt (false);
  driver.executeTrajectory(trajectory, interrupt);


  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}


int main(int argc, char **argv){

  UR10Driver driver("10.18.0.12");
  minimalMotionToHome(driver);
  marcTest(driver);

  return EXIT_SUCCESS;
}
