# SocketProgramming
Sockets are a fascinating concept as communication endpoints between 2 or more entities running on the same host using the same filesystem or on totally different hosts and filesystem miles apart but of course connected by a network  
When I was new to socket programming, it was very difficult for me to understand various types of sockets and how they work with the resources on the internet which were scattered, lacked annotations or didn't cover enough.  
This prodded me to create a unified repo containing some examples in C with copious annotations for different types of sockets. I also created some real world applications like a network file server/sender to demonstrate their possible applications

In this project, I have created examples and sample apps for following type of sockets:
- **TCP** (Stream socket used for secure reliable connection between 2 processes running on the same or different hosts on the same network)
- **UDP** (Datagram socket used for fast data exchange between 2 processes running on the same or different hosts on the same network)
- **Unix** (Stream/Datagram socket residing on the same filesystem as the 2 processes using it to communicate)
- **CAN** (Raw sockets used to transmit and receive CAN messages using a can interface created by a CAN bus controller)
- **Netlink** (Raw sockets used mostly for kernel-userspace communication and also for interprocess communication)
- **Virtio** (Virtio sockets used for communication between host and virtual machine using the virtio-vsock driver and vhost-vsock backend)

**Note:** If you're working with SocketCAN and do not have a real CAN bus to work with, you can create a virtual CAN bus interface using can-utils package. A script `vcan-setup.sh` has been created for the same under *Misc* directory of this repo https://github.com/amoldhamale1105/CodingPlayground

## Build Instructions
This project is setup with a hierarchical cmake structure. The top level cmake adds downstream directories which in turn have their own cmake to build the sources enclosed. By default it will generate binaries for all sources within this repo under the `bin` directory  

If you working with one particular type of socket, for instance `TCP`, comment out the `add_subdirectory()` calls which are not required. The top-level cmake in our example would look something like this:
```
add_subdirectory(TCP)
#add_subdirectory(UDP)
#add_subdirectory(Unix)
```
This will save you some time if you're dedicatedly working on a specific type of socket  

Once the target directories are set in cmake, run the build script with or without options depending on your build configuration. Defaults for each option will appear between `{}` in the usage instruction. Print the script usage by running the following command
```
./build.sh -h
```
As an example, if you want to use the script to build for release mode with `Unix Makefiles` cmake generator, it can be executed as follows
```
./build.sh -a -r -g "Unix Makefiles"
```
On a successful build, required binaries will be generated under the `bin` directory at the root of the project

## Contribution
This is a repository catering to new programmers and experienced developers alike who like to tinker with sockets. I'd love to see you contribute by adding to this project any fun experiments you conducted using raw socket programming. Be sure to share this with people who are new to socket programming and wish to have working examples with explanation for all types of sockets in one place.  
Contribution in any way is welcome. Get in touch with me in case of any questions or suggestions amoldhamale1105@gmail.com