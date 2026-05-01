import astrall as al


def test_smoke():
    rt = al.from_config("astrall_core/configs/robot.yaml")

    img = rt.camera().get_frame()
    cloud = rt.radar().get_pointcloud()  # mock/demo point cloud only

    assert img.shape == (480, 640, 3)
    assert img.dtype.name == "uint8"
    assert cloud.shape == (1024, 4)
    assert cloud.dtype.name == "float32"

    controller = al.Controller(rt.backend(), 1.0, 1.0, 0.1, 0.1)
    controller.stop()

    sm = rt.state_machine()
    sm.start_mission([
        al.Point2D(1.0, 0.0),
        al.Point2D(2.0, 1.0),
    ])

    for _ in range(500):
        if not sm.running():
            break
        sm.update()

    assert sm.state() in (al.RobotState.Idle, al.RobotState.Navigate)
