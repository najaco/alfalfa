from in_time_encoder import test, InTimeEncoder

if __name__ == "__main__":
    # print(test())
    x = InTimeEncoder("/dev/video1", "MJPG", 100000)
    x.run()