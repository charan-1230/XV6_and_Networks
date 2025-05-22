# MP2

## System Calls

### 1.Gotta count ‘em all

#### Implementation

- I am using a array to store the count of sys_calls called by a process, in `struct proc` of that process present in `proc.h`.The count value is increased in `syscall()` function in `kernel/syscall.c` based on the enumeration of syscalls in `syscall.h`.This works because every syscall when called should visit the `syscall()` function.

- The array is initialized to 0's in `allocproc()` in `kernel/proc.c` and necessary changes(like changing values to 0,-1 to indicate that they are unusable) are made in `freeproc()` too.

- To include the count from child process i added counts from child into parent when child is exiting.so i used `exit()` in `kernel/proc.c`.

- In the `sys_getSysCount()` extraction of sys_call_number to count is done using `argint()` and then function returns the corresponding count stored in `syscall_count` array of `struct proc`

- The `syscount.c` in user directory just deodes the mask and executes the command through `exec()` and calls the `getSysCount()`.

### 2.Wake me up when my timer ends

#### Implementation

- I am using a new `trapframe`(alarmframe) (to store the info of the process that calls sigalarm()),interval,elapsed_interval,handler function, a flag( to track whether execution is in alarm handler function) in `struct proc` of that process.

- Necessary initialisations and memory allocation of alarmframe are done in `allocproc()` in `kernel/proc.c` and necessary changes(like changing values to 0,-1 to indicate that they are unusable) are made in `freeproc()` too.

- `sys_sigalarm` updates the values of the interval and handler in the struct proc by extraction of arguments using `argint()` and `argaddr()` respectively.

- `sys_sigreturn` restores the process state of the process that called sigalarm using the previously stored alarmframe and returns corresponding a0 of that frame which ensures that the process resumes exactly from the same state as where it left off.

- The main logic is implemented in `kernal/trap.c` in `usertrap()`.Every time there is a timer interrupt,the elasped time is incremented till it reaches the desired interval, when reached the program counter is changed to handler function( so that execution shifts there) after storing the process's current trapframe in alarmframe.Also the elapsed time interval is reset to zero.

## Scheduling

### LBS

#### Implementation

- I am using the `rand()` function provided in `user/grind.c` to generate random numbers.The lottery_win_ticket is (random_number)%(total_tickets) at that time.

- I am using new variables tickets_owned(to store the tickets of the process),arrival_time in `struct proc` of that process and these values are initialized in `allocproc()` and necessary changes(like changing values to 0,-1 to indicate that they are unusable) are made in `freeproc()` too.

- `sys_settickets()` in `kernel/sysproc.c` just modifies the value of the tickets_owned to the value of argument provided.

- In the `fork()` i am assigning child process's tickets with parent's tickets.

- The scheduling policy is written in `scheduler()` of `kernel/proc.c`.The logic is to find the process which holds lottery_win_ticket(generated randomly) in its range of tickets with help of cummulative sum and for loop.(eg:if lottery_win_ticket=20 and process tickets in proc array are (5 10 30 20) then 20 lies in range of 3rd process tickets(16 to 45)).Then a for loop checks the min_arrival_time among all process with same tickets as above found process.The process found by the latter for loop is scheduled.

- As the timerinterrupt arrives after every 1 tick,which forces the executing process to yield the cpu,the time slice is ensured as 1 tick.

## Performance Analysis

| Policy | Avg run time | Avg wait time |
|-----------------|-----------------|-----------------|
| RR     |  15   | 159   |
| LBS    |  15   | 146   |
| MLFQ   |  15  |  137  |

- The unit of times mentioned above is ticks.

- LBS is efficient than RR because it has lower average wait time i.e., on an average the process waits for less amount of time to get control of the cpu.

- similarly MFLQ is more efficient than LBS as it has lower average wait time.

- The average run time doesn't depend on scheduling policy because it is just total run time of all process divided by no of processess.Here run time (depends on complexity of instructions ),total number of process  both are independent of scheduling policy.

### What is the implication of adding the arrival time in the lottery based scheduling policy?

- `Fairness Between Processes:` when multiple processes have the same number of tickets, the one that arrived first is prioritized. This prevents starvation of early-arriving processes which may happen in traditional LBS.

### Are there any pitfalls to watch out for? 

- `Complexity in Implementation:`Introducing the arrival time as a factor in the lottery system can add complexity to the scheduler's logic.

- If most of the process have same number of tickets(say x) and for all the times when process holding x tickets wins, the process among this group with least arrival time is scheduled until it exits.so the newly arrived process among this group face tremendous starvation.

### What happens if all processes have the same number of tickets?

- If all have same number of tickets,anyone can be the winner but the first arrived is being scheduled first till it completes/exits which reduces the fairness drastically.

- `Effect on Randomness:`If all processes have the same number of tickets(say 30 30 30), pure randomness becomes the deciding factor for which process is scheduled next. This means each process has an equal chance of being selected.(If random number%90 lies in 0-30,31-60,61-30 then process 1,2,3 are scheduled respectively.here probability for each would be 1/3).


## Networking

### XOXO

### TCP_server

- `concatenate_and_send():` Combines messages and sends them to the respective client.

- `checkWinner():` Evaluates the game board to determine if there is a winner or if the game is a draw.

- `initialize_BoardString():` Initializes an empty Tic Tac Toe board.

- `update_boardstr():` Validates and processes a player's move, updating the board and checking for the game's result.

- Socket Creation: The server creates a TCP socket and binds it to port 34567.
- Listening for Clients: The server listens for incoming client connections.
    - It accepts connections from two clients.
- Client Synchronization: 
    - Once both clients connect,the server sends a "wait" message to the first client until the second client joins.

- Message Handling:
    - After both clients are connected, the server enters a loop to handle communication.
    - The server waits for a message from one client, then sends the received message to the other client.

- Turn-Based Communication:
    - The server ensures that communication between clients happens in turns.
    - One client types and sends their message, while the other client receives it. After sending the message, the sending client waits for a response from the other.

- Message Forwarding:
    - The server reads incoming messages from one client using recv(). The message is then relayed to the other client using send().
    - If either client sends the termination message (e.g., "no"), the server stops the relay and closes the connection with both clients.
    
- Client Disconnection:
    -If a client disconnects abruptly or a message indicating the end of communication (such as "sorry") is received, the server terminates the session for both clients.

### How to Use( for local host also)

`Compile the code:`
- gcc -o tcp_server tcp_server.c

`Run:`
- ./tcp_server `<IP address>`

### TCP_client


- Socket Creation: The client creates a TCP socket.

- Server Connection: It connects to the server using the provided IP and port 34567.

- Message Handling: The client continuously listens for messages from the server. Based on the message content:
    - If prompted, the user inputs a response.
    - Messages like "wait" are ignored.

- Input Handling: User input is captured, stripped of newline characters, and sent back to the server.

- Termination: The connection terminates when the server sends a "sorry" message or when the client inputs "no."

### How to Use( for local host also)

`Compile the code:`
- gcc -o tcp_client tcp_client.c

`Run:`
- ./tcp_client `<IP address>`

### UDP_server

- The implementation of game part is same as tcp but with few changes on the communcation part
    - Using socket(AF_INET, SOCK_DGRAM, 0) to create a UDP socket.
    - sendto/recvfrom: Used sendto and recvfrom to handle sending and receiving messages with the client. Each call to sendto and recvfrom includes the client address (client_addr1 or client_addr2).
    - Address Tracking: The server keeps track of the clients' addresses in client_addr1 and client_addr2. These addresses are used when communicating with the respective clients.

### How to Use( for local host also)

`Compile the code:`
- gcc -o udp_server udp_server.c

`Run:`
- ./udp_server `<IP address>`

### UDP_client

- The implementation of game part is same as tcp but with few changes on the communcation part
    - Using socket(AF_INET, SOCK_DGRAM, 0) to create a UDP socket.
    - Sending Data: Replaced send() with sendto(), which requires the server's address as a parameter.
    - Receiving Data: Replaced recv() with recvfrom(), which also captures the server's address for proper communication.

### How to Use( for local host also)

`Compile the code:`
- gcc -o udp_client udp_client.c

`Run:`
- ./udp_client `<IP address>`



# XV6_and_Networks
