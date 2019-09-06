import zmq

def main():
    """ main method """

    # Prepare our context and publisher
    context    = zmq.Context()
    subscriber = context.socket(zmq.SUB)
    subscriber.connect("tcp://10.1.1.151:7777")
    subscriber.setsockopt(zmq.SUBSCRIBE, b"YcoeStatus")

    while True:
        # Read envelope with address
        [address, contents] = subscriber.recv_multipart()
        print("[%s] %s" % (address, contents))

# We never get here but clean up anyhow
    subscriber.close()
    context.term()

if __name__ == "__main__":
    main()
