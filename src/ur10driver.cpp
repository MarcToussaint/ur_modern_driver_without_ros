#include <chrono>
#include <cstdlib>
#include <string>
#include <thread>
#include <math.h>
#include <fstream>
#include <iostream>

#include <ur_modern_driver/pipeline.h>
#include <ur_modern_driver/ur/lowbandwidth_trajectory_follower.h>
#include <ur_modern_driver/ur/trajectory_follower.h>
#include <ur_modern_driver/ur/factory.h>
#include <ur_modern_driver/ur/parser.h>
#include <ur_modern_driver/ur/producer.h>
#include <ur_modern_driver/ur/rt_state.h>
#include <ur_modern_driver/ur/state.h>

#include "ur10driver.h"

//=============================================================================

class UR10Driver_RTdata_Consumer : public URRTPacketConsumer {
public:
  std::atomic<bool> q_isInitialized;
  std::atomic<std::array<double, 6>> q_real;
  std::atomic<std::array<double, 6>> qdot_real;

  std::shared_ptr<LogFile> logFile;
  std::chrono::high_resolution_clock::time_point startTime;

  UR10Driver_RTdata_Consumer() : q_isInitialized(false) {
    startTime = std::chrono::high_resolution_clock::now();
  }
  ~UR10Driver_RTdata_Consumer() {}

  virtual bool consume(RTState_V1_6__7& state){ return publish(state); }
  virtual bool consume(RTState_V1_8& state){ return publish(state); }
  virtual bool consume(RTState_V3_0__1& state){ return publish(state); }
  virtual bool consume(RTState_V3_2__3& state){ return publish(state); }

  bool publish(RTShared& packet){
    q_real = packet.q_actual;
    qdot_real = packet.qd_actual;
    if(!q_isInitialized){
      std::cout <<"** initial q: ";
      for( int i=0;i<6;i++) cout <<' ' <<packet.q_actual[i];
      cout <<endl;
    }
    q_isInitialized=true;

    auto some_time = std::chrono::high_resolution_clock::now();
    if(logFile){
      logFile->lock();
      logFile->out() <<"REAL ";
      logFile->out() <<std::chrono::duration_cast<std::chrono::duration<double>>(some_time-startTime).count();
      for(int i=0;i<6;i++) logFile->out() <<' ' <<packet.q_actual[i];
      for(int i=0;i<6;i++) logFile->out() <<' ' <<packet.qd_actual[i];
      logFile->out() <<endl;
      logFile->unlock();
    }
    return true;
  }
};

//=============================================================================

struct UR10Driver_private {
  std::string host;
  int UR_SECONDARY_PORT = 30002;
  int UR_RT_PORT = 30003;

  URFactory factory;
  UR10Driver_RTdata_Consumer rt_con;

  // log file
  std::shared_ptr<LogFile> logFile;

  // RT packets consumer (MarcRTConsumer)
  std::unique_ptr<URParser<RTPacket>> rt_parser;
  std::shared_ptr<URStream> rt_stream;
  std::shared_ptr<URProducer<RTPacket>> rt_prod;
  std::shared_ptr<MultiConsumer<RTPacket>> rt_cons;
  std::shared_ptr<Pipeline<RTPacket>> rt_pl;

  // STATE consumer (none right now!)
  std::unique_ptr<URParser<StatePacket>> state_parser;
  std::shared_ptr<URStream> state_stream;
  std::shared_ptr<URProducer<StatePacket>> state_prod;
  std::shared_ptr<MultiConsumer<StatePacket>> state_cons;
  std::shared_ptr<Pipeline<StatePacket>> state_pl;

  // Trajectory Follower
  std::unique_ptr<URCommander> rt_commander;
  std::shared_ptr<TrajectoryFollowerInterface> traj_follower;

  UR10Driver_private(std::string _host, bool useLogFile);
  ~UR10Driver_private();
};

//=============================================================================

std::string getLocalIPAccessibleFromHost(std::string &host, int port) {
  URStream stream(host, port);
  return stream.connect() ? stream.getIP() : std::string();
}

//=============================================================================

UR10Driver_private::UR10Driver_private(std::string _host, bool useLogFile) : host(_host), factory(host) {

  std::string local_ip = getLocalIPAccessibleFromHost(host, UR_RT_PORT);
  INotifier *notifier(nullptr);

  // RT packets consumer (MarcRTConsumer)
  rt_parser = factory.getRTParser();
  rt_stream = make_shared<URStream>(host, UR_RT_PORT);
  rt_prod = make_shared<URProducer<RTPacket>>(*rt_stream, *rt_parser);
  rt_cons = make_shared<MultiConsumer<RTPacket>>( vector<IConsumer<RTPacket> *>({ &rt_con }) );
  rt_pl = make_shared<Pipeline<RTPacket>>(*rt_prod, *rt_cons, "RTPacket", *notifier);

  // STATE consumer (none right now!)
  state_parser = factory.getStateParser();
  state_stream = make_shared<URStream>(host, UR_SECONDARY_PORT);
  state_prod = make_shared<URProducer<StatePacket>>(*state_stream, *state_parser);
  state_cons = make_shared<MultiConsumer<StatePacket>>( vector<IConsumer<StatePacket> *>({}) );
  state_pl = make_shared<Pipeline<StatePacket>>(*state_prod, *state_cons, "StatePacket", *notifier);

  // Trajectory Follower
  int reverse_port = 50001;
  rt_commander = factory.getCommander(*rt_stream);
  traj_follower = make_shared<LowBandwidthTrajectoryFollower>(*rt_commander, local_ip, reverse_port, factory.isVersion3());
  //        //traj_follower = new TrajectoryFollower(*rt_commander, local_ip, reverse_port, factory.isVersion3());

  // log file
  if(useLogFile){
    logFile = make_shared<LogFile>("z.ur10.log");
    dynamic_pointer_cast<LowBandwidthTrajectoryFollower>(traj_follower)->logFile = logFile;
    dynamic_pointer_cast<LowBandwidthTrajectoryFollower>(traj_follower)->launchTime = rt_con.startTime;
    rt_con.logFile = logFile;
  }

  rt_pl->run();
  state_pl->run();

  // wait for initialization
  for(;;){
    if(rt_con.q_isInitialized) break;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

UR10Driver_private::~UR10Driver_private(){
  rt_pl->stop();
  state_pl->stop();
}

//=============================================================================

UR10Driver::UR10Driver(std::string host, bool useLogFile){
  self = make_shared<UR10Driver_private>(host, useLogFile);
}

UR10Driver::~UR10Driver(){
}

bool UR10Driver::isInitialized(){
  return self->rt_con.q_isInitialized;
}

std::array<double, 6> UR10Driver::get_q_now(){
  return self->rt_con.q_real;
}

std::array<double, 6> UR10Driver::get_qdot_now(){
  return self->rt_con.qdot_real;
}

void UR10Driver::executeTrajectory(std::vector<TrajectoryPoint> &trajectory, std::atomic<bool> &interrupt){
  std::cout <<"** START" <<endl;
  self->traj_follower->start();
  std::cout <<"** EXECUTE" <<endl;
  self->traj_follower->execute(trajectory, interrupt);
  std::cout <<"** STOP" <<endl;
  self->traj_follower->stop();
  std::cout <<"** DONE" <<endl;
}
