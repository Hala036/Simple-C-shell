Simple Shell by Hala Abu Tair
This program implements a simple shell that accepts commands, executes them, and supports aliasing of commands.
It also maintains statistics on the number of commands, aliases, and script lines processed, and handles command inputs that include quotes.
Structure and Functions:
• Structs and Globals:
	o Alias: A structure to store aliases, which map a key to a command.
	o cmd_count, alias_count, script_count, quotes_count: Counters for commands, aliases, script lines, and commands with quotes, respectively.
	o pr: String for the shell prompt.
	o quotes: Boolean flag to track if the command contains quotes.
	o aliases: Dynamic array to store aliases.
• Functions:
	o prompt(): Updates the shell prompt with current counts.
	o addAlias(): Adds a new alias or updates an existing one.
	o splitKeyCmd(): Splits a string into a key and command based on a delimiter.
	o deleteAliasByKey(): Deletes an alias by its ksey.
	o execute(): Parses, processes, and executes a command.
	o exitShell(): Cleans up resources and exits the shell.
• How to compile:
	o Compile: gcc ex1.c –o ex1
	o Run : ./ex1
• Program files :
	o ex1.c- contains all of the code
	o run_me.sh- to compile and run the program
	o README.txt- this file, a brief explanation about the code


