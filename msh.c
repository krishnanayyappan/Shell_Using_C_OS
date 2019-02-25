/*
    Name: KRISHNAN AYYAPPAN
    ID:   1001603608
*/

#define _GNU_SOURCE 1

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 11    // Mav shell only supports ten arguments excluding command

  int  showpid_array[50];       // Array to store history of PIDs
  char *history_array[50] = { NULL }; // Array to store history of commands
  int sarr_val = 0, harr_val = 0;     // Index variables of history of commands & PIDs
  pid_t forkone = 0;                  // forkone is the variable to store the PID
  
/* handle_signal function will take an action based on the signal received
*/
static void handle_signal (int sig )
{

  switch( sig )
  {
    case SIGINT: 
     if ( forkone != 0 )
     {
      kill(forkone,SIGTERM);
     }
    break;

    case SIGTSTP: 
     if ( forkone != 0 )
     {
      kill(forkone,SIGTSTP);
     }
    break;

    default: 
    break;

  }

}

int main()
{

  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE); //Allocating memory for the input (cmd_str)
  int complete = 1, i = 0;
  int h = 0;

  struct sigaction act;
 
  memset (&act, '\0', sizeof(act)); // Initialize the sigaction struct

  
  //  Set the handler to use the function handle_signal()
  
  act.sa_handler = &handle_signal;

  //  Install the handler for SIGINT and SIGTSTP and check the 
  //  return value. 
  if (sigaction(SIGINT , &act, NULL) < 0) // Call sigaction with SIGINT as the parameter
  {                                       // when a Ctrl-C signal is caught
    perror ("sigaction: ");
    return 1;
  }

  if (sigaction(SIGTSTP , &act, NULL) < 0) // Call sigaction with SIGTSTP as the parameter
  {                                        // when a Ctrl-Z signal is caught
    perror ("sigaction: ");
    return 1;
  }

  while( complete )
  {
    // Print out the msh prompt
    printf ("msh> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int   token_count = 0; // Index variable for the tokens

    // Pointer to point to the token
    // parsed by strsep
    char *arg_ptr;

    char *working_str  = strdup( cmd_str );

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

/* Below block of code implements the functionality of executing a command that was run earlier
   User can do this inputting !n command, where n is the sequence number */
    char * char_count;
    char_count = strchr (cmd_str, '!'); // Detecting whether the input token has an !
    int char_1 = char_count-cmd_str + 1, char_2 = 0;
    if ( char_1 > 0 )      // WHen the value is > 0, it means that ! is present in the input
    {
      int hist_rep_val = ((working_root[char_1]) - '0'); //Convert the value in the array to decimal
      char_2 = char_1 + 1;
     // Condition in the below IF checks whether there is a 2nd digit in the input (!n)
     // If yes, then the value is computed to access the history of commands and execute it
     if ( ((working_root[char_2]) - '0') >= 0 && ((working_root[char_2]) - '0') <= 9 )
      {
        int hist_rep_val2 = ((working_root[char_1+1]) - '0');
        hist_rep_val = (hist_rep_val * 10) + hist_rep_val2;
      }

      if ( hist_rep_val < harr_val )
      {
       if ( harr_val < 15 )
       {
         working_str = history_array[hist_rep_val];
       }
       else
       {
         working_str = history_array[harr_val + hist_rep_val - 15];
       }
      }
      else
      {
        printf("Command not in history.\n");
        continue;
      }
    }

    // Tokenize the input stringswith whitespace used as the delimiter
    while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) &&
              (token_count<=MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }

    // Now print the tokenized input as a debug check
    // \TODO Remove this code and replace with your shell functionality

    // Continue & print the next line (msh>) if there is no input ie.NULL
    if ( token[0] == '\0' )
    {
      continue;
    }

    //If input is cd, then use the chdir() to change the directory
    else if ( strcmp(token[0],"cd") == 0 ) 
    {
      chdir(token[1]);
      strcat (token[0], " ");
      // Concatenate the cd and following token and store it in the history array
      history_array[harr_val] = strcat (token[0],token[1]);
      harr_val++;               // Increment the array index after storing
      continue;
    }

    else if ( (strcmp(token[0],"quit") == 0) || (strcmp(token[0],"exit") == 0) )
    {
      exit(0); // Exit the shell abruptly if the input is quit/exit
    }

    //when the input is bg, send SIGCONT signal to the suspended process
    //indicating it to continue running in the background
    else if ( strcmp(token[0],"bg") == 0 )
    {
      kill(forkone,SIGCONT);
      continue;
    }

    else
    {
      forkone = fork(); // Fork a child process
      if (forkone < 0)  // If the value returned is < 0, then exit the process
      {
         printf("\n System error occured. Exiting...\n");
         fflush(NULL);
         exit(0);
      }
      else if (forkone == 0) // PID of 0 denotes the child process
      {
         int execval = execvp (token[0], token); //Execute the command in child process entered
         if ( strcmp(token[0],"showpids") == 0 ) //showpids  command will show the PIDs of the
         {                                       // processes that were run earlier
           history_array[harr_val] = token[0];   // Record the execution of 'showpids' command in
           harr_val++;                           // the history. Increment the index after storing
           for (i = 0; i < sarr_val; i++)
           {
             printf("%d: %d\n", i, showpid_array[i]); //print the PIDs
           }
           exit(0);
           continue;
         }
         if ( strcmp(token[0],"history") == 0 ) //history command will show the commands
         {                                      // run in the past
           history_array[harr_val] = token[0];  // Record the execution of 'history' command also
           harr_val++;                          // array. Increment the index after storing
          if ( harr_val < 15 )      // msh will show only the past 15 commands
          {                         // If the array storing the history has <= 15 commands
           for (h = 0; h < harr_val; h++) // this for loop will directly print the 15 commands
           {
             printf("%d: %s\n", h, history_array[h]);
           }
          }
          else
          {
           int k = 0; // If there are more than 15 elements/commands in the array, then print
           for (i = (harr_val - 15); i < harr_val; i++) //only the recent 15 commands
           {
             printf("%d: %s\n", k, history_array[i]);
             k++;
           }
          }
           exit(0);
           continue;
         }
         else
         {
           if (execval == -1) //If the entered command doesn't fall true in any of the above
           {                  // then it would be an invalid input. So, print the error message
             printf("%s: Command not found.\n", token[0]);
             exit (0);
           }
         }
      }
      else
      {
         int status;
         waitpid(forkone, &status, 0); //Wait for the child process to complete execution
         showpid_array[sarr_val] = forkone; //Store the PID of the recently completed child
         sarr_val++;                        //in the array that stores the history of PIDs
                                            //After incrementing, increase the array index
         int m = 0;
         for (m=1; token[m] != '\0'; m++)
         {
           strcat(token[0]," ");      //Concatenate the tokens so that the whole command
           strcat(token[0],token[m]); // is stored in the history array
                                      // For example, ls -lt, ps -a -e -f
         }
         history_array[harr_val] = token[0]; //After executing, store the command in array
         harr_val++;                         //Increment the array index after storing
      }
    }
    free( working_root ); //Free the memory that was allocated
  }
  return 0;
}
