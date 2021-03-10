from in_time_encoder import test, InTimeEncoder

if __name__ == "__main__":
    print(test())
    x = InTimeEncoder("/dev/video0", "MJPG", 100000)
    print(x.getTargetSize())
    x.run()