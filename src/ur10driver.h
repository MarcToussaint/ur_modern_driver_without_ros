#include <memory>
#include <atomic>

#include <ur_modern_driver/trajectory_point.h>

class UR10Driver{
public:
  std::shared_ptr<struct UR10Driver_private> self;

  UR10Driver();
  ~UR10Driver();

  bool isInitialized();

  std::array<double, 6> get_q_now();
  std::array<double, 6> get_qdot_now();

  void executeTrajectory(std::vector<TrajectoryPoint>& trajectory, std::atomic<bool>& interrupt);
};
