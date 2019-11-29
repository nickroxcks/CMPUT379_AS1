# DRAGON SHELL
Dragonshell interactive shell

System Calls Used:
-chdir()
-fork()
-execve()
-_exit()
-wait()
-waitpid()
-open()
-close()
-dup2()
-pipe()
-kill()
-cwdir()

Initial bootup:

On boot the program initially displays a welcome prompt and calls a command
getcommand() to fetch commands from the user. Once a command has been entered
the prgoram will first determine what type of command it is. The program prioritzes
first checking for backround proccess commands. 

Program Architecture:

As mentioned previously, the program first checks if the command is a backround command. 
If the command is a backround command (command has '&' as the last entered character),
the program proceeds to running the command in the backround. All commands that
need to be executed first go through a query_handling() function. This functions purpose
is to break up the user entered command into individual commands (those that are seperated by ';')
and run each of them sequentially. A loop will go through each of these individual commands and decides
what to do with each of the commands. Checking for piping structure in the individual command is checked 
for first(commands that have a '|'). If a piping structure is found, the program procees to a piping
function to run the pipe command(maximum of one '|' allowed). If the command is not a piping command, 
program proceeds to checking if the command entered is a built in command(piping does not support basic
commands unless the basic commands are in the $PATH variable). If not a built in command, program
checks if the commands needs to be outputted to a file or checks different locations for where the
command might be. For this, a find_executable function is run in which it will search the current directory,
$PATH, or if the command entered specified a directory(in the form: $DIR COMMAND). A found command will
be executed, otherwise program will tell the user the command was not found and prompt for another command
back in the main proccess.

Basic commands:

cd - used chdir()
pwd - used getcwdir()
a2path - checked for different cases of a2path entered, and simply updated a string called path which is passed
between functions.
exit - since basic commands do not involve forks, the one calling this function is the "master parent". In
the event however there are still somehow still active children(backround proccesses, etc) a vector called 
process_list has a list of pids of all the proccesses the parent has forked. The parent will loop through and send a 
SIGSTOP signal to all other processes beneath it. (if the proccess in the vector is killed already, kill() just returns -1 and 
parent continues checking vector). Afterwards, the parent gracefully calls _exit() system call. The same happens when 
ctr+d is pressed, but this is handled by the getcommand() function.

Run External Programs:

The functions mentioned in program architecture explain the functions being called for this. For all programs
being executed(external or local), a fork is called and a child runs execve() to run the command. Running
a command succefully terminates the child right then and there. Otherwise, the child manually exits and program 
procceeds to parent, which has been in a wait() state this whole time. 

Output redirection:

To do this we simple create a file descriptor with open() and pass the file name given by the user
into the open function. We then dup2() and rederict output to the file, and then execute the command.

Pipe:

Parent runs the pipe() function to create space in memory to allow the parent and its children to write and read
from. An initial fork happens and the childs output gets redirected into the pipe, after which the child executes the command
and returns to parent. The parent then forks again, and the child reads from this pipe the output of the previous command.
The child then uses this output as STDIN into the next command to be run for piping. Since the child inherits parents 
STDOUT, the output of this command gets displayed into the terminal.

Running Multiple Commands:

This was explained in the architecture

Handling signals:

On boot, the program calls the signal() functions to handle the different types of signals sent from user keystrokes. 
A function called handle_keystrokes is passed into signal(), which allows the program to catch the signal and prevent
the program from closing. Looking at the handler function, all children(they dont have the "master pid" from the top parent) simply 
exit(there should not be a case where these children have children of themsevles). I called exit because the assignemnt description didn't
stait what to do otherwise. The main process(master parent) would simply ignore these signals and continue running.

Putting jobs in backround:

As specified in the architecture, checking if a command is to be put in the backround happens as soon as the user enters a command. In the 
event it does, the parent forks. The parent first waits and allows the child to print a message to the screen displaying its pid. The 
child then sends a signal to the parent to continue running, and the child proceeds to executing the command given by the user(the parent and
child are running at the same time). After the child finished its command, the child sends a signal to its parent denoting it is finished. At
this point, the child is a zombie proccess as it needs its parent to call wait(). As a solution to this problem, the parent has a handler installed
that will react to a SIGCHLD signal, and interupt the parent to then call the wait() function to allow the child to gracefully terminate.






