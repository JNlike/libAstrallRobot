import astrall as al


def main():
    rt = al.from_config("astrall_core/configs/robot.yaml")

    img = rt.camera().get_frame()
    cloud = rt.radar().get_pointcloud()

    print("image shape:", img.shape)
    print("mock cloud shape:", cloud.shape)

    sm = rt.state_machine()
    sm.start_mission([
        al.Point2D(1.0, 0.0),
        al.Point2D(2.0, 1.0),
    ])

    for _ in range(500):
        if not sm.running():
            break
        sm.update()

    print("final state:", sm.state())
    print("final pose:", rt.backend().get_current_pose())


if __name__ == "__main__":
    main()
