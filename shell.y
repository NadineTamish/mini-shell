
/*
 * CS-413 Spring 98
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%token	<string_val> WORD

%token 	NOTOKEN GREAT NEWLINE LESS APPEND PIPE AMPERSAND EXIT CD APPENDAMPERSAND

%union	{
		char   *string_val;
	}

%{
extern "C" 
{
	int yylex();
	void yyerror (char const *s);
}
#define yylex yylex
#include <stdio.h>
#include "command.h"
#include <unistd.h>
%}

%%

goal:	
	commands
	;

commands: 
	command
	| commands command 
	;

command: simple_command
	;

simple_command: 
 command_and_args iomodifier_list_opt background_opt NEWLINE {
		printf("   Yacc: Execute command\n");
		Command::_currentCommand.execute();
	}
	| EXIT NEWLINE{
		printf("Good bye!!\n");
        exit(0); // Exit the shell

	} 
	| CD WORD NEWLINE{
		char list[200];
		printf("Previous directory was %s\n", getcwd(list, 200));
        
        // Change to the specified directory
        if (chdir($2) != 0) {
            perror("cd failed"); // Handle the case where chdir fails
        }
        
        printf("Current directory is %s\n", getcwd(list, 200));
	}
	|CD NEWLINE{
		char list[200];
		  printf("Previous directory was %s\n", getcwd(list, 200));
        
        // Change to the home directory
        const char* homeDir = getenv("HOME");
        if (homeDir != nullptr) {
            if (chdir(homeDir) != 0) {
                perror("cd failed"); // Handle the case where chdir fails
            }
        } else {
            fprintf(stderr, "Home directory not found.\n");
        }
        
        printf("Current directory is %s\n", getcwd(list, 200));
	}
	| error NEWLINE { yyerrok; }
	;
    


// command_and_args:
// 	command_word arg_list {
// 		Command::_currentCommand.
// 			insertSimpleCommand( Command::_currentSimpleCommand );
// 	} 
// 	;

command_and_args:
	command_word arg_list {
		Command::_currentCommand.insertSimpleCommand(Command::_currentSimpleCommand);
	}
	|command_word {
        Command::_currentCommand.insertSimpleCommand(Command::_currentSimpleCommand);
    }
	| command_and_args PIPE command_word arg_list {
		// Handle pipe logic here
		Command::_currentCommand.insertSimpleCommand(Command::_currentSimpleCommand);
	}
	;

// command_pipe:
// 	command_pipe PIPE command_and_args{
// 		Command::_currentCommand.insertSimpleCommand( Command::_currentSimpleCommand );
// 	}
// 	| command_and_args
// 	;


arg_list:
	arg_list argument
	| /* can be empty */
	;

iomodifier_list_opt:
	iomodifier_list
    | /* empty */
    ;

iomodifier_list:
    iomodifier_opt iomodifier_list
    | iomodifier_opt
    ;

argument:
	WORD {
			printf("   Yacc: insert argument \"%s\"\n", $1);
			Command::_currentSimpleCommand->insertArgument($1);
		
		}
	
	;


command_word:
	WORD {
               printf("   Yacc: insert command \"%s\"\n", $1);
	       
	       Command::_currentSimpleCommand = new SimpleCommand();
	       Command::_currentSimpleCommand->insertArgument( $1 );
	}
	;

iomodifier_opt:
	GREAT WORD {
		printf("   Yacc: insert output \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
	}
	
	
	| LESS WORD {
		printf("   Yacc: insert input \"%s\"\n", $2);
		Command::_currentCommand._inputFile = $2;
	}
	
	
	| APPEND WORD {
		printf("   Yacc: insert output \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
		// Command::_currentCommand._outFile = $2;
	}
	;

background_opt:
    AMPERSAND {
        printf("   Yacc: background execution\n");
        Command::_currentCommand._background = 1;
    }
    | /* empty */
    ;

%%

void
yyerror(const char * s)
{
	fprintf(stderr,"%s", s);
}

#if 0
main()
{
	yyparse();
}
#endif
