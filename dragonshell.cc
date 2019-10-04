#include <vector>
#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include <algorithm>
using namespace std;

#include <errno.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>

int masterpid=1;  //temporarily initialized to 1
bool command_complete = 0;
vector<int> process_list;  //vector of all the child processes that have been created

void handle_keystrokes(int signum) 
{
    if (((signum == SIGINT) || (signum == SIGTSTP)) && masterpid != (int) getpid()) {
        cout<<"ctrl c or z signal caught. Child is quiting"<<endl;
        _exit(1);
    }
    //handles the killing of zombie proccesses
    else if(signum == SIGCHLD){
      wait(NULL);
      //signal(signum, handle_keystrokes);
    }
}
/*
void handler_test(int signum){
  cout<<"HA! Caught ya"<<endl;;
  cout<<"this pid of this process is: "<<(int) getpid()<<endl;
  if(getpid()!= masterpid){
    cout<<"we reached here??"<<endl;
    _exit(1);
  cout<<"I am master"<<endl;
  }
}
*/

/**
 * @brief Tokenize a string 
 * 
 * @param str - The string to tokenize
 * @param delim - The string containing delimiter character(s)
 * @return std::vector<std::string> - The list of tokenized strings. Can be empty
 */
std::vector<std::string> tokenize(const std::string &str, const char *delim) {
  char* cstr = new char[str.size() + 1];
  std::strcpy(cstr, str.c_str());

  char* tokenized_string = strtok(cstr, delim);

  std::vector<std::string> tokens;
  while (tokenized_string != NULL)
  {
    tokens.push_back(std::string(tokenized_string));
    tokenized_string = strtok(NULL, delim);
  }
  delete[] cstr;

  return tokens;
}

void run_executable(vector<string> command_vector,string search_path){
  int command_size = command_vector.size();
  char wdir[1000];
  //cout << getcwd(wdir,1000) << endl;
 // char *argv7[]={"ls", "-al", NULL};
  char *argv1[command_size+1];
  char *envp[]={NULL};
 
  //formatting the argv1 parameter for the execve system call
  for(int i=0;i<command_size;i++){
    //reformat to allow second parameter in execve to work
    char *temp = (char *)(command_vector.at(i)).c_str();
    argv1[i] = temp;
  }
    argv1[command_size] = NULL;
  //checking directory for command
  DIR *dir;
  struct dirent *dir_struct;
  if ((dir = opendir(search_path.c_str())) != NULL) {
    /* print all the files and directories within directory */
    while ((dir_struct = readdir(dir)) != NULL) {
      if((command_vector.at(0)).compare(dir_struct->d_name) == 0){
        //printf ("For reference, this file in curdir is: %s\n", dir_struct->d_name);
        search_path.append(+ "/" + command_vector.at(0));
        closedir(dir);
        execve(search_path.c_str(),argv1,envp);
        cout<<"child is not terminating"<<endl;
      }
    }
    closedir(dir);
    } 
    else {
    //if we are here, something went wrong
    cout<<"child terminating with pid: "<<getpid();
    _exit(1);
    return;
    }

}
void find_executable(vector<string> command_vector,string path){
  //path_vec in this case is all directories stored in path
  vector<string> path_vec = tokenize(path,":");
  char wdir[1000];
  string current_file_path = getcwd(wdir,1000);
  
  //First check if the command is in local directory
  //cout<<"checking for command in local directory"<<endl;
  run_executable(command_vector,current_file_path);
  
  
  //Next check $PATH
  
  for(int i =0;i<path_vec.size();i++){
    //cout<<"checking in path: " << path_vec.at(i);
    run_executable(command_vector,path_vec.at(i));
  }

  //see if command is in ths form:  $DIR file_or_command
  if(command_vector.size() ==1){
  vector<string> dir_vec;
  dir_vec.assign(0,command_vector.at(1));
  run_executable(dir_vec,command_vector.at(0));
  }
}

//TODO: make a cute welcome logo
void boot_sequence(){

  printf("Welcome to dragonshell!!!\n");

}

string get_command(){

  string command;

  printf("dragonshell > ");
  getline(cin,command);
  
  if (cin.fail() || cin.eof()) {
    cin.clear(); // reset cin state
    cout<<"we are resetting cin"<<endl;
    return "";
  }
  return command;

}

bool validate_path(string path){
  struct stat temp;  //temp struct to use stat sys call
    if (stat(path.c_str(), &temp) == 0 && S_ISDIR(temp.st_mode)){
      return true;
    }
    else{
      return false;
    }
}

string built_commands(vector<string> command_vector,string path){
     //cd command
    if(command_vector.at(0).compare("cd")==0){
      if(command_vector.size()==2){
        int result = chdir((command_vector.at(1)).c_str());
        if(result == -1){
          printf("dragonshell: No such file or directory\n");
        }
        //testing to see if it worked
        //char wdir[1000];
        //cout << getcwd(wdir,1000) << endl;
      }
      else
        {
        printf("dragonshell: expected argument to \"cd\"\n");
        }
      command_complete=1;
      return path;
    }
    //pw command
    else if(command_vector.at(0).compare("pwd") == 0){
      char wdir[1000];
      cout << getcwd(wdir,1000) << endl;
      command_complete = 1;
      return path;
    }
    //$PATH command
    else if(command_vector.at(0).compare("$PATH") == 0){
      cout<<path<<endl;
      command_complete = 1;
      return path;
     }
    //a2path command
    else if(command_vector.at(0).compare("a2path") == 0){
      
      if(command_vector.size() == 2){
        //path_vec here is just the second arguement from the entire command broken up by :'s
        vector<string> path_vec = tokenize(command_vector.at(1),":");
       
        //this means we are appending
        if(path_vec.size()>1 && (path_vec.at(0)).compare("$PATH") == 0){
            string new_path = (command_vector.at(1)).erase(0,6);
            if(path.compare("") == 0){
              path = path.append(new_path);
              command_complete = 1;
              return path;
            }
            else{
            path = path.append(":" + new_path);
            command_complete = 1;
            return path;
            }
        }
        //this means we are overwriting the whole path
        else{
          path = command_vector.at(1);
          command_complete = 1;
          return path;
        }
      }
      //creating an empty path
      else if(command_vector.size() == 1){
         path = "";
         command_complete = 1;
         return path;
        }
      else{ 
          cout<<"dragonshell: Invalid arguments for \"a2path\""<<endl;
          command_complete = 1;
          return path;
        }
    }
    //exit command
    else if(command_vector.at(0).compare("exit") == 0){
      //in the event a process still somehow manages to remain a zombie(shouldnt happen), kill it.
      for(int i=0;i<process_list.size();i++){
        //cout<<"killing the pid: " << process_list.at(i)<< "kill returns: "<<kill(process_list.at(i),SIGSTOP)<<endl;
        kill(process_list.at(i),SIGSTOP);
      }
      cout<<"Exiting"<<endl;
      _exit(1);  //ends the program
    }
  return path;
}

void process_pipe(vector<string> pipe_vector,string path){

  int pipefd[2];
  pipe(pipefd);  //a shared memory between proccesses
  int rc = fork();
  vector<string> first_command_vector = tokenize(pipe_vector.at(0)," ");
  vector<string> second_command_vector = tokenize(pipe_vector.at(1)," ");

  if (rc < 0) {
    // fork failed; exit
    fprintf(stderr, "fork failed\n");
    _exit(1);
  } 
  else if (rc == 0) {
    // child (new process)
    printf("hello, I am child (pid:%d)\n", (int) getpid());
    close(pipefd[0]);    // close reading end in the child

    dup2(pipefd[1], 1);  // send stdout to the pipe
    dup2(pipefd[1], 2);  // send stderr to the pipe(just in case)

    close(pipefd[1]);

    find_executable(first_command_vector,path);  //output goes to pipe
      
    printf("child haasn't found any commands");
    _exit(1);
  }
  else {
    // parent goes down this path (original process)
    //char buffer[1024];

    close(pipefd[1]);  // close the write end of the pipe(parent)
    
    //note to self: This loop will read from pipe that the child has filled up and put it in a buffer(char array)
    //while (read(pipefd[0], buffer, sizeof(buffer)) != 0) //continue to read from the pipe until child finishes
    //{}  
    process_list.push_back(rc);
    int wc3 = wait(NULL);
    /*
    for(int i=0;i<=100;i++){
      cout<<buffer[i]<<endl;
    }
    */
    int rc2 = fork();
    if (rc2 < 0) {
      // fork failed; exit
      fprintf(stderr, "fork failed\n");
      exit(1);
    } 
    else if (rc2 == 0) {
      
      //read from the pipe it's STDIN for the next command
      //Standard STDOUT inherited from the parent
      dup2(pipefd[0], 0);

      find_executable(second_command_vector,path);
      cout<<"dragonshell: Second command failed"<<endl;
      _exit(1);
    }
    else {
      // parent goes down this path (original process)
      process_list.push_back(rc);
      int wc = wait(NULL);
      //printf("hello, I am parent of %d (wc:%d) (pid:%d)\n",
      //rc2, wc, (int) getpid());
    }
  }
}

/**
 * @brief Query through a command
 * 
 * @param command - String command passed by user
 * @param path - The current $PATH varaible to check for executables
 * @return string - return the new $PATH which may have changed
 */
string query_handling(string command,string path){

  vector<string> semicolon_vector = tokenize(command,";");  //this vector breaks the command by its ";"

  //loop through all the commands that are seperated by ";"
  for(int i = 0; i<semicolon_vector.size(); i++){
    vector<string> pipe_vector = tokenize(semicolon_vector.at(i),"|"); //vector that splits the current command by its pipes
    vector<string> command_vector = tokenize(semicolon_vector.at(i)," ");//vector that splits the current command by spaces
    string command_str = semicolon_vector.at(i);
    vector<string> out_vec = tokenize(command_str,">"); //vector that splits current command by ">" for output redirect

    if(pipe_vector.size() == 2){
      process_pipe(pipe_vector,path);  //if the command has a piping structure, proceed to pipe processing 
      continue;
    }
    else if(pipe_vector.size() > 2){
      cout<<"dragonshell: multiple piping not supported"<<endl;
      continue;
    }
    path = built_commands(command_vector,path);  //check if its a built in command. If it is, run it
   
    if(command_complete){  //if basic command was executed, move on to next command in semicolon_vector
      command_complete=0;
      continue;
    } 

    //checking for executables
    else{
      int rc = fork();
      if (rc < 0) {
          // fork failed; exit
          fprintf(stderr, "fork failed\n");
          exit(1);
      } 
      else if (rc == 0) {
          // child (new process)
          //printf("hello, I am child (pid:%d)\n", (int) getpid());

          //If we need to rederict output to a file, we will go through this if block
          if(out_vec.size() == 2){
            vector<string> vec_sendout = tokenize(out_vec.at(0)," ");
                
            int fd = open((out_vec.at(1)).c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
                
            dup2(fd, 1);   // send stdout to file
            close(fd);
            find_executable(vec_sendout,path);
          }
          //if the user entered the command with a directory as the first command parameter
          else if((command_vector.at(0)).at(0) == '/'){
            cout<<"checking directory given by user for command"<<endl;
            string temp_path = command_vector.at(0);
            vector<string> formatted_vector; //reformat to remove first element and pass to find_executable
            for(int i=1;i<command_vector.size();i++){
              formatted_vector.push_back(command_vector.at(i));  
            }
            find_executable(formatted_vector,temp_path);
          }
          //otherwise, search for the command
         else{ 
            find_executable(command_vector,path);
         }
          cout<<"dragonshell: Command not found"<<endl;
          exit(1);
     
        }
      else {
        // parent goes down this path (original process)
        process_list.push_back(rc);
        int wc = waitpid(rc,NULL,NULL);
        //printf("hello, I am parent of %d (wc:%d) (pid:%d)\n",
        //rc, wc, (int) getpid());
      }
    }
  }
  return path;
}

int main(int argc, char **argv) {
  boot_sequence();
  masterpid = (int) getpid();  //the high level parent

  void (*signal_ctrlc)(int);
  void (*signal_ctrlz)(int);
  void (*signal_child)(int);
  signal_ctrlc = signal(SIGINT, handle_keystrokes);
  signal_ctrlz = signal(SIGTSTP, handle_keystrokes);
  signal_child = signal(SIGCHLD,handle_keystrokes);
  string command;
  string path = "/bin/:/usr/bin/";
  
  while(true){
    
    cout<<"hello, this should be the master. my pid is: "<< (int) getpid()<<endl;
    command = get_command();
    
    if(command.compare("") == 0){
      continue;
    }
    
    int strsize = command.size();
    char lastchar = command.at(strsize-1);
    
    //Check if the command need to be put in the backround
    if(lastchar == '&'){
      vector<string> new_command_vector = tokenize(command,"&");
      string new_command = new_command_vector.at(0);
      int newrc = fork();
      if(newrc<0){
        fprintf(stderr, "fork failed\n");
        _exit(1);
      }
      else if(newrc == 0){
        //child is here
        cout<<"PID "<< (int) getpid()<<" is running in the background"<<endl;
        kill(getppid(),SIGCONT);  //tell parent to resume operation
        freopen("/dev/null", "w", stdout);  //prevents process from printing to terminal
        path = query_handling(new_command,path);  //continue running in backround
        _exit(2);  //this will send SIGCHLD to parent, in which parent will wait to kill zombie
        cout<<"Fatal Error"<<endl;
      }
      else{
        //parent is here
        waitpid(newrc,NULL,NULL);  //this just wait for the child to print the run in backround message. 
        process_list.push_back(newrc);
        cout<<"parent still going"<<endl;
        continue;
      }
      continue;
    }
    
    path = query_handling(command,path);  //if no command needs to run in backround, we will run command
  }
  return 0;
}
