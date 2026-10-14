ReadMe for Aidan Rohm's Simple Shell (Terminal Like Application)

1.0 Building this Program and Its Features

    Admittedly, I was a bit ambitious when implementing some of the features I had previously desired in my proposal.
    At the base level, this program does what is expected: a child process is launched by the main() to handle a user's input. The following paragraphs will
    discuss the features that I could implement, what their respective functions do, etc. Upon opening the program (if you do), you will notice a 
    list of 10 function prototypes. Some functions were implemented for necessity, while others were implemented for user experience. There are 
    necessary functions to trim the user's input and then parse it, as tokenizing the input was generally required to separate commands from arguments, utilize a history
    queue, etc.
    
    - Isolated Environment -

    The isolated environment is handled directly in the main. Upon code execution, a directory is created that demonstrates a new working directory. 
    After you run the code once, you should expect that the directory will continue to exist, and every time you run it from there on out, you will be in the same directory as the
    first time you launched the program. This provides clear advantages. Upon the first execution, the directory is empty, which helps the user identify the changes they have made. When
    I first ran an early stage of my code; it was disappointing to see that the commands I used reflected the contents of my CISC5595 folder on Erdos. This felt underwhelming as
    I wanted to see the changes I made in real time and feel like I was almost on a brand-new computer (even though this was not the case). After implementing the
    isolated environment, my satisfaction significantly increased, and the user experience improved. 

    * How to use (quotations denote a test case/computer output) *

    Upon executing the code for the first time, you should see something like this:

        "
        Isolated environment created: /tmp/shell_env
        Welcome to Aidan's Shell!
        /tmp/shell_env/- <<>>
        "

        If you then execute ls, you will see:

        "
        /tmp/shell_env/- <<>> ls
        /tmp/shell_env/- <<>> 
        "
            
        This is logical since you have nothing in this directory. The following demonstrates what you can later expect using something like "touch":

        "
        /tmp/shell_env/- <<>> touch HelloWorld.cpp
        /tmp/shell_env/- <<>> ls
        /tmp/shell_env/- <<>> HelloWorld.cpp
        "
                
        Note: If you choose to create a new directory and later navigate to it, your current directory will change. This will be indicated by the path visible before the command prompter ("<<>>"). To remove the temporary directory, exit the shell ("exit" command) and then run: rm -rf /tmp/shell_env
        This will remove the temporary directory from your computer.

    
    - History Command - 
    
    The commands AddToHistory, PrintHistory, and ExecuteCommandFromHistory
    are somewhat self-explanatory. The AddToHistory command adds a command precisely as input to a queue with a length of 20. If this command already exists
    in the history, that command gets bumped to the last position. I like this functionality because otherwise, the same command could flood the entire history.
    The PrintHistory command is used to display the user's history. The user can then use the index of the command shown in the queue to execute
    "!n" where n is the index of the desired command. If the user inputs this format, the program automatically recognizes the desired intent and uses the ExecuteCommandFromHistory function to do what is expected. The ExecuteCommandFromHistory function has its child process creation. This is done so the 
    command can be logically executed instead of overwhelming the main with similar code. 

    * How to use (quotations denote a test case/computer output) *

        Say you entered the command "echo cat," you should expect the following upon searching for and executing the command from your history:

        "
        /tmp/shell_env/- <<>> echo cat
        cat
        /tmp/shell_env/- <<>> history
        1: cat
        /tmp/shell_env/- <<>> !1
        Executing: echo cat
        cat
        /tmp/shell_env/- <<>>
        "
            
        Note: Executing a command from history will also add that command to the history itself (if you execute !1, !1 will be added to the history).

    - Command Requires Arguments - 

    This feature is wonky. Upon implementing my shell, I occasionally noticed that an entered command would cause some hanging. For example, if
    I entered "cat" but did not include a file as an argument; hanging would occur, and I would have to close and use CTRL+C to force my program to quit. For obvious reasons,
    I wouldn't say I liked this, so I implemented a short error case. That being said, some other commands are handled not by this implemented function (CommandRequiresArguments)
    but by the OS itself (at least, that is how it seems). It is clear when this happens, as it appears like a typical issue that would arise in any other shell. That said, 
    the implementation did what was desired (mostly), though some consistency would be appreciated. If I had more time to polish this, I could try to get
    my shell to personally handle all errors, such as too few arguments for a command.

    - Input/Output Redirection - 

    The HandleRedirection function uses a pretty simple flag to indicate the redirection type. A while loop iterates through the user's
    input until it reaches the end.
    An if statement compares each part of the input to the redirection operators. If a redirection operator is found, the case is handled as expected. The input file is set to the next
    token after the input operator, and vice versa for the output case. 

    * How to use (quotations denote a test case/computer output) *

        "
        /tmp/shell_env/- <<>> echo cat > output.txt
        /tmp/shell_env/- <<>> cat output.txt
        cat
        "
    

        - Tab Autocompletion - 

        This was undoubtedly the most exciting part to have implemented. Two functions operate with the readline library to complete the task of tab autocompletion.
        The function CommandGenerator generates possible command completions based on the user's input. The list of auto-completeable commands is included in a commands[] list.
        This list is iterated as each command is compared to the user input. If a command in the list starts with the same characters as the user input, it returns
        the duplicated command. The CustomCompletion command again utilizes the readline library to do what is intended. An index is used to differentiate between commands and
        files. If that index happens to be zero, the function will call the readline function that completes commands. Otherwise, the function will call the readline function that completes file and directory names.

        * How to use (quotations denote a test case/computer output) *

        "
        /tmp/shell_env/- <<>> ec<\t> 
        "translates to"
        /tmp/shell_env/- <<>> echo
        "
       
    - Customized ls Command -

    This customization was done purely for user experience. The execute_ls_command function does precisely what it seems, though it adds some functionality.
    Before implementing this function, directories would appear the same color as files, which annoyed me. By implementing this function, directories will appear in blue.
    The function works by checking to see if the item about to be displayed is a directory or a file; if it is a directory, it colors it blue,
    otherwise, the standard color is shown.

2.0 Necessary Information (How to compile etc.)

    This program uses the readline library, which was unfortunately not included on Erdos (at least not for me as a student), and I was also unable to install it... Because of this, I did most of the coding on my terminal. I am unsure whether this will cause issues, and if it does, I sincerely apologize. I noticed a problem when transferring the code from Erdos to my laptop. Are the compilers different? Anyway, the result was that a few errors came up that had yet to previously occur on Erdos (before I tried to do tab implementation). You must have access to the readline.h library, as this is how my tab autocompletion works.

    The following command compiles the shell and saves the executable as MyShell
    gcc -o MyShell AidanRohm_FinalProject.c -lreadline

    The following command runs the shell:
    ./MyShell


    3.0 Concluding Thoughts and Notes
    
    There are some things that I need help implementing, such as piping. I tried to stick to some of the more essential things to implement. Admittedly, I am
    not a pro when using Linux, and this program may exhibit that. In some cases where hanging occurs, I tried to see if it was expected compared to a regular Zsh shell
    (like the one on my laptop), and if it did, I just went along with it. Things work pretty well for the most part. You can open and edit 
    files using your choice of file editor (vim or Nano),
    start an interactive Python session, and even conduct an SSH connection (I tried this out by logging on to Storm... surprisingly, it worked).
    As specified by the project requirements, the code creates a child process to execute the command, and the parent waits until it is complete before continuing the process. Execvp() is called to take care of the commands by checking the system path, so as long as the command exists in your path, it should work. Please note that this is very experimental, and it is hard to say how effective every single command is, considering there are way too many to test each one individually.

    Note: I have to warn you that this file might have a lot of wonky formatting. About halfway through typing this out, I switched to a different computer. As a result, the indentation of the sentences has the potential to be very inconsistent.
