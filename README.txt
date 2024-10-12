simple C shell by Hala Abu Tair

Branch with simplified shell functionality, featuring background jobs, and input/output redirections. 
Users are presented with a command prompt where they can enter commands, which are then parsed and executed. 
It supports features like handling "and" and "or" operations, redirecting error output to files, and managing jobs with process IDs. 
The shell runs continuously, prompting for input until the "exit_shell" command is entered, after which it performs cleanup and exits.
Signal handling is also included to manage background processes effectively.

• Structs and Globals:
- linked list "Job" that contains the functions:
1. index
2. pid, process id
3. command name
4. next, pointer to the next node in the list
- cmd: command counter, incremented each time a command finishes successfully

• Functions:
1. analyze, analyze2nd: determine the type of command and operators, and the order the program should run in.
2. process, numOfArgs: makes sure that the input is not crossing bounds, and removes extra whitespace, quotes, and parentheses. 
3. toFile, redirectToFile, restoreDirection: create the file to redirect errors into, and then restore the stderr directory after the command.
4. readFile: executes the cat" command by printing the contents of the specified file.
5. addJob, deleteJob, jobs: dynamically allocate and deallocate memory for processes in the linked list, and print them.
6. and_or: handles the "and"\ "or" operations with the correct logic.
7. execute: ensures correct form of command and executes it using execvp in a child process.
8. sig_handler: if a command for a background process was passed, this function helps run it in the background by catching the child's SIGCHLD and deleting the job while the parent process runs concurrently.
9. exit_shell: frees any allocated memory that hasn't been free'd yet and exits.

• Program Files:
-ex2.c: contains the entire C program
-run_me.sh: bash code to compile the program
-README.txt: simple explanation of the program

• How to compile:
compile: gcc ex2.c -o ex2
run: ./ex2
