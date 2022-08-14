# SocketProgramming
Sockets are a fascinating concept as communication media between 2 processes running on the same host using the same filesystem or on totally different hosts and filesystem miles apart but of course connected by a network  
When I was new to socket programming it was very confusing for me to understand the concept. The code on the internet was either too complex or didn't cover enough    
It was hard to get hold of some lucid code which could explain the concepts I learnt which I could also play with because admittedly its impossible to learn a software concept properly without getting your hands dirty with it  
This prodded me to create this repo with some socket programming examples in C used to demonstrate how sockets work  

There are primarily 2 socket domains (Internet and Unix)  
In this project, I have laid down examples with elaborate comments for 3 most commonly used types of sockets:
- **TCP** (Stream socket used for secure reliable connection between 2 processes running on the same or different hosts on the same network)
- **UDP** (Datagram socket used for fast data exchange between 2 processes running on the same or different hosts on the same network)
- **Unix** (Stream/Datagram socket residing on the same filesystem as the 2 processes using it to communicate)

## Build Instructions
This project is setup with a hierarchical cmake structure. The top level cmake adds downstream directories which in turn have their own cmake to build the sources enclosed  
By default it will generate binaries for all sources within this repo under the `bin` directory  
If you working with one particular type of socket, for instance `TCP`, comment out the `add_subdirectory()` calls which are not required. The top-level cmake in our example would look something like this:
```
add_subdirectory(TCP)
#add_subdirectory(UDP)
#add_subdirectory(Unix)
```
This will save build time and unncessary build instructions when you're not intending to execute the output binaries  
Once the target directories are set in cmake, run `Clean Reconfigure All` or `Clean Rebuild All` in case you've loaded the project using VSCode  
Enter the following commands at the project root if you work with the command line
```
mkdir build
cd build
cmake ..
make
```
On a successful build, required binaries will be generated under the `bin` directory at the root of the project

## Contribution
This is a repository catering to new programmers and experienced developers alike who like to tinker with sockets. I'd love people to contribute by adding to this project any fun experiments they conducted using raw socket programming  
Also, any other version of code or in a different language (for instance, python) which explains socket programming in a much better manner would be great.  
Contribution in any way is appreciable. Send me an email (amoldhamale1105@gmail.com) if you happen to face issues creating branches, issues or raising pull requests 