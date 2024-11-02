
/*
 * CS354: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>   // For open() and file flags
#include "command.h"
#include <time.h>
#include <fstream>

SimpleCommand::SimpleCommand()
{
	// Creat available space for 5 arguments
	_numberOfAvailableArguments = 5;
	_numberOfArguments = 0;
	_arguments = (char **) malloc( _numberOfAvailableArguments * sizeof( char * ) );
}

void
SimpleCommand::insertArgument( char * argument )
{
	if ( _numberOfAvailableArguments == _numberOfArguments  + 1 ) {
		// Double the available space
		_numberOfAvailableArguments *= 2;
		_arguments = (char **) realloc( _arguments,
				  _numberOfAvailableArguments * sizeof( char * ) );
	}
	
	_arguments[ _numberOfArguments ] = argument;

	// Add NULL argument at the end
	_arguments[ _numberOfArguments + 1] = NULL;
	
	_numberOfArguments++;
}

Command::Command()
{
	// Create available space for one simple command
	_numberOfAvailableSimpleCommands = 1;
	_simpleCommands = (SimpleCommand **)
		malloc( _numberOfSimpleCommands * sizeof( SimpleCommand * ) );

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
	_appendFlag=0;
	_outputErrorCombinedFlag=0;
}

void
Command::insertSimpleCommand( SimpleCommand * simpleCommand )
{
	if ( _numberOfAvailableSimpleCommands == _numberOfSimpleCommands ) {
		_numberOfAvailableSimpleCommands *= 2;
		_simpleCommands = (SimpleCommand **) realloc( _simpleCommands,
			 _numberOfAvailableSimpleCommands * sizeof( SimpleCommand * ) );
	}
	
	_simpleCommands[ _numberOfSimpleCommands ] = simpleCommand;
	_numberOfSimpleCommands++;
}

void
Command:: clear()
{
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		for ( int j = 0; j < _simpleCommands[ i ]->_numberOfArguments; j ++ ) {
			free ( _simpleCommands[ i ]->_arguments[ j ] );
		}
		
		free ( _simpleCommands[ i ]->_arguments );
		free ( _simpleCommands[ i ] );
	}

	if ( _outFile ) {
		free( _outFile );
	}

	if ( _inputFile ) {
		free( _inputFile );
	}

	// if ( _errFile ) {
	// 	free( _errFile );
	// 	_errFile = nullptr;
	// }
	// Only free _errFile if it's different from _outFile
    if (_errFile && _errFile != _outFile) {
        free(_errFile);
        _errFile = nullptr;
    }

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
	_appendFlag=0;
	_outputErrorCombinedFlag=0;
}

void
Command::print()
{
	printf("\n\n");
	printf("              COMMAND TABLE                \n");
	printf("\n");
	printf("  #   Simple Commands\n");
	printf("  --- ----------------------------------------------------------\n");
	
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		printf("  %-3d ", i );
		for ( int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++ ) {
			printf("\"%s\" \t", _simpleCommands[i]->_arguments[ j ] );
		}
	}

	printf( "\n\n" );
	printf( "  Output       Input        Error        Background\n" );
	printf( "  ------------ ------------ ------------ ------------\n" );
	printf( "  %-12s %-12s %-12s %-12s\n", _outFile?_outFile:"default",
		_inputFile?_inputFile:"default", _errFile?_errFile:"default",
		_background?"YES":"NO");
	printf( "\n\n" );
	
}
void handle_sigint(int sig) {
    printf("\nmyshell> ");  // Print a new prompt on Ctrl-C
    fflush(stdout);          // Ensure the prompt appears immediately
}


void sigchld_handler(int signo) {
    static time_t lastTime;
    time(&lastTime); // Get the current time

    // Open the log file
    std::ofstream fw("termination.log", std::ios_base::app);
    if (fw.is_open()) {
        fw << "Child PID terminated at: " << ctime(&lastTime) << "\n";
    }
    fw.close();  
}


void
Command::execute()
{
	int defaultin = dup(0);     // Save default stdin
    int defaultout = dup(1);    // Save default stdout
    int defaulterr = dup(2);    // Save default stderr

	int infd = defaultin;
    int outfd = defaultout;
    int errfd = defaulterr;


	if (_inputFile != nullptr) {
        infd = open(_inputFile, O_RDONLY);
        if (infd < 0) {
            perror("Input file open error");
            return;
        }
        dup2(infd, 0);
        close(infd);
    }

    // Output redirection (overwrites or appends)
    // if (_outFile != nullptr) {
    //     if (_appendFlag) {
    //         outfd = open(_outFile, O_WRONLY | O_CREAT | O_APPEND, 0666);
    //     } else {
    //         outfd = open(_outFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    //     }
    //     if (outfd < 0) {
    //         perror("Output file open error");
    //         return;
    //     }
    //     dup2(outfd, 1);
    //     close(outfd);
    // }

    // Error redirection (overwrites or appends)
    // if (_errFile != nullptr) {
    //     if (_appendFlag) {
    //         errfd = open(_errFile, O_WRONLY | O_CREAT | O_APPEND, 0666);
    //     } else {
    //         errfd = open(_errFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    //     }
    //     if (errfd < 0) {
    //         perror("Error file open error");
    //         return;
    //     }
    //     dup2(errfd, 2);
    //     close(errfd);
    // }
	   // Output and error redirection with combined flag
    if (_outFile != nullptr) {
        if (_outputErrorCombinedFlag) {
            // Open output file for both stdout and stderr redirection (append mode if _appendFlag is set)
            outfd = open(_outFile, O_WRONLY | O_CREAT | (_appendFlag ? O_APPEND : O_TRUNC), 0666);
            if (outfd < 0) {
                perror("Output file open error");
                return;
            }
            // Redirect stdout
            dup2(outfd, 1);
            // Redirect stderr to the same file as stdout
            dup2(outfd, 2);
            close(outfd);
        } else {
            // Standard output redirection (append mode if _appendFlag is set)
            outfd = open(_outFile, O_WRONLY | O_CREAT | (_appendFlag ? O_APPEND : O_TRUNC), 0666);
            if (outfd < 0) {
                perror("Output file open error");
                return;
            }
            dup2(outfd, 1);
            close(outfd);

            // Standard error redirection if _errFile is specified
            if (_errFile) {
                errfd = open(_errFile, O_WRONLY | O_CREAT | (_appendFlag ? O_APPEND : O_TRUNC), 0666);
                if (errfd < 0) {
                    perror("Error file open error");
                    return;
                }
                dup2(errfd, 2);
                close(errfd);
            }
        }
    }
	

	// Don't do anything if there are no simple commands
	if ( _numberOfSimpleCommands == 1 && strcmp(_simpleCommands[0]->_arguments[0],"exit")==0 ) {
		printf("Good Bye !!\n");
		//prompt();
		//return;
		clear();
		dup2(defaultin, 0);
        dup2(defaultout, 1);
        dup2(defaulterr, 2);
        close(defaultin);
        close(defaultout);
        close(defaulterr);
		exit(0); // this exits the shell
	}
	int pipefd[2];
    int prev_pipe_read = infd;
	pid_t pid;
    // Loop over each simple command
    for (int i = 0; i < _numberOfSimpleCommands; i++) {
        // Set up the next pipe if there is a following command
        if (i < _numberOfSimpleCommands - 1) {
            if (pipe(pipefd) == -1) {
                perror("pipe failed");
                exit(EXIT_FAILURE);
            }
            outfd = pipefd[1];
        } else {
            // Last command: use output redirection, if specified
            outfd = (_outFile) ? outfd : defaultout;
        }

        // Fork process for each command
        pid = fork();
        if (pid == -1) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) { // Child process
            // Redirect input
            dup2(prev_pipe_read, 0);
			if (prev_pipe_read != infd) 
				close(prev_pipe_read);

  

            // Redirect output
            if (outfd != defaultout) {
                dup2(outfd, 1);
                close(outfd);
            }

            // Redirect error
            if (_errFile) {
                dup2(errfd, 2);
                close(errfd);
            }

            // Execute the command
            execvp(_simpleCommands[i]->_arguments[0], _simpleCommands[i]->_arguments);
            perror("execvp failed");
            exit(1);
        } else { // Parent process
            // Close the write end of the current pipe in the parent
            if (outfd != defaultout) 
				close(outfd);
			
			if (prev_pipe_read != infd) 
				close(prev_pipe_read);

            // Prepare for the next command in the pipeline
            prev_pipe_read = pipefd[0];
        }
    }

	// Print contents of Command data structure
	print();

	// Add execution here
	// For every simple command fork a new process
	// Setup i/o redirection
	// and call exec
	
	// Execute each simple command
	// pid_t pid;
    // for (int i = 0; i < _numberOfSimpleCommands; i++) {
    //     pid = fork();
    //     if (pid == -1) {
    //         perror("fork failed\n");
    //         exit(2);
    //     }

    //     if (pid == 0) {
    //         // Child process
    //         printf("Executing command: %s\n", _simpleCommands[i]->_arguments[0]); // Debugging statement
    //         execvp(_simpleCommands[i]->_arguments[0], _simpleCommands[i]->_arguments);
    //         perror("execvp failed");
    //         exit(1);
    //     } else {
    //         printf("Forked child process with PID: %d\n", pid); // Debugging statement
    //     }
    
	// }

	// Restore input, output, and error

	dup2( defaultin, 0 );
	dup2( defaultout, 1 );
	dup2( defaulterr, 2 );

	// Close file descriptors that are not needed

	close( defaultin );
	close( defaultout );
	close( defaulterr );
	
	
    signal(SIGINT, handle_sigint);

	// Wait for last command if not in background
    if (!_background) {
        waitpid(pid, nullptr, 0);
    }

	// Clear to prepare for next command
	clear();
	
	// Print new prompt
	prompt();
}

// Shell implementation

void
Command::prompt()
{
	printf("myshell>");
	fflush(stdout);
}

Command Command::_currentCommand;
SimpleCommand * Command::_currentSimpleCommand;

int yyparse(void);

int 
main()
{
	
    signal(SIGCHLD, sigchld_handler);

	signal(SIGINT, handle_sigint);
	//signal(SIGINT,SIG_IGN); //sets the SIGINT signal handler to SIG_IGN, which tells the program to ignore the signal instead of terminating.
	Command::_currentCommand.prompt();
	yyparse();
	return 0;
}

