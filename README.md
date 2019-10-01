# ur_modern_driver_without_ros

This is a fork of the `ur_modern_driver`
(https://github.com/ros-industrial/ur_modern_driver) stripped
completely from ROS. It adopted the kinect_devel branch as the master.

The repo adds a minimalistic wrapper interface to the
trajectoryFollower that hides compile dependences and compiles into a shared library. A minimal example shows how to actuate a joint.

Compile with `cmake` as usual:
```
mkdir build
cd build
cmake ..
make
```
