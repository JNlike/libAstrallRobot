# OdomNode

Placeholder for localization output ownership.

For the production robot, FAST-LIO or another validated estimator should consume `PointCloud2` plus IMU and publish odom/TF. Do not derive production odometry from the mock `Radar` interface.
