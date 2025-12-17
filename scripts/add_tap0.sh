ip tuntap add dev tap0 mode tap
ip address add 10.0.0.1/24 dev  tap0
ip link set tap0 up