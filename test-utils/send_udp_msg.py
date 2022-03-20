import socket
import argparse 
import logging

logger = logging.getLogger(__name__)

def get_args():
    argparser = argparse.ArgumentParser(description="A simple python script that sends a user-defined \
                                                    message to the UDP server determined by the given IP \
                                                    and port.")
    argparser.add_argument("--ip", metavar="<ip-address>", required=True, 
                           help="IP of the target UDP server.")
    argparser.add_argument("--port", metavar="<port>", type=int, required=True,
                           help="Port of the target UDP server.")
    argparser.add_argument("--message", metavar="<msg>", required=True, 
                           help="The message to send to the UDP server.")

    ip_stack_group = argparser.add_mutually_exclusive_group()
    ip_stack_group.add_argument("--use-ipv6", action="store_const", dest="ip_stack", const=socket.AF_INET6,
                                default=socket.AF_INET, help="Use IPv6 instead of IPv4 stack for the socket.")

    return argparser.parse_args()

if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(name)s - %(levelname)s >>> %(message)s',
                        handlers=[logging.StreamHandler()])
    args = get_args()

    logger.info("Creating socket using IP Stack %s", args.ip_stack)
    sock = socket.socket(args.ip_stack, socket.SOCK_DGRAM) 

    logger.info("Sending message '%s' to UDP Server (Host: %s | Port: %s)", 
                args.message, args.ip, args.port)
    bytes_sent = sock.sendto(str.encode(args.message), (args.ip, args.port))

    logger.info("%s bytes were sent to UDP server %s:%s", bytes_sent, args.ip, args.port)