# AOS ASSIGNMENT 2

### Execution :
1) Create a tracker file that contains atleast 1 address : <ip:port>
For compiling, include `-pthread`!
2) Execute tracker : `g++ -pthread -o tracker tracker.cpp` --> `./tracker trckf.txt`
3) Execute peer : `g++ -pthread -o peer peer.cpp` -->
3) File transfer : If the current chunk recieved is less than the expected chunk-size, it is requested again till the correct one is recieved `./peer 127.0.0.1:8000 trckf.txt`
4) On peer, create user first, then login and then use other commands

### Implementation specifics:
1) File can be downloaded from both seeders and leechers who are logged-in currently
2) Piece selection algorithm : Selection of chunks in a round robin way to distribute the chunks equally among seeders and leechers
3) User can directly join any group with join_group command
4) The chunk-size can be changed to any size, transfer would be handled in every case
5) Binary files can also be transfered

### Assumptions :
1) One user cannot be logged in on different terminals
2) In the same session, 2 users cannot login with same socket
3) A peer cannot terminate abruptly (Ctrl+C)
4) Group admin cannot leave the group
5) If a user (not admin) leaves a group, all the files which are shared ONLY by him are removed from the group and also his name is removed from seeders of the group files. But locally the user will have those files so they will be shown during 'show_downloads' command.
6) Tracker will always be active





