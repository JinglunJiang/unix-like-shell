# Unix-like-shell (msh)

A simple Unix-like shell, similar to a Unix shell, called "msh".

## Features

- Execute foreground and background jobs.
- Support for built-in commands.
- Implemented signal handlers.

## Getting Started

### Prerequisites

To run the Unix-like-shell, you need to have the following installed:

- C Compiler (e.g., GCC)

### Installation

Clone the repository:

```bash
git clone git@github.com:JinglunJiang/unix-like-shell.git
```

### Building the msh Executable

Since the source and header files are in different directories, you need to tell the compiler (i.e., gcc) where they are located. Here’s a simple way to do it:

```bash
gcc -I./include/ -o ./bin/msh src/*.c
```

### Project Structure

.
├── bin # Executable directory
├── data # Auxiliary data files and test data
├── include # Header files
│ └── shell.h
├── README.md # Documentation about the repository (to be expanded)
├── Makefile # (Optional) Makefile for compiling multiple C files
├── saqs # Directory for short answer questions
├── scripts # Bash scripts, including build.sh
│ └── build.sh # Script for building the shell
├── src # Source files
│ ├── msh.c
│ └── shell.c
└── tests # Testcases

## Functions

### Shell Module Functions

1. msh_t \*alloc_shell(int max_jobs, int max_line, int max_history);
   This function allocates (i.e., malloc), initializes, and returns a pointer to a msh_t value. The shell has limits on the number of jobs in existence, the maximum number of characters that can be entered on the command line, and how many previous commands are stored in its history. These are represented by the parameters to the function (max_jobs, max_line, and max_history) respectively.

2. char *parse_tok(char *line, int \*job_type);
   This function continuously retrieves separate commands from the provided command line until all commands are parsed. The special characters & and ; are used to separate multiple commands on the same command line. For each returned job, it will place a 1 at the address job_type points to if the job is a foreground job; otherwise, it places 0 at that address to represent the job is a background job.

3. char \**separate_args(char *line, int *argc, bool *is_builtin);
   This function separates out each word in the provided line and places them in a newly allocated array of strings. The command name is always at index 0. It places the number of arguments it found (including the program name) at the address of argc. The last element in the returned array must contain a NULL value.

4. int evaluate(msh_t *shell, char *line);
   This function evaluates the provided command line (line) and performs necessary actions such as checking line length, adding it to history, parsing jobs, and executing either built-in commands or external commands in foreground or background.

5. void waitfg(pid_t pid);
   This function waits for the foreground job with the provided process ID (pid) to complete.

6. char *builtin_cmd(msh_t *shell, char \*\*argv);
   This function handles the built-in commands that are recognized by the shell.

7. void exit_shell(msh_t \*shell);
   This function ensures that all background jobs have completed before exiting the shell. It frees allocated resources, including jobs and history.

### Built-in Commands

The Unix-like-shell (`msh`) supports the following built-in commands:

- **`jobs`**: Lists all background and suspended jobs, displaying their job ID, process ID, state (RUNNING or Stopped), and command line.

  Example:

  ```bash
  [1] 1234 RUNNING ls -la
  [2] 5678 Stopped cat file.txt

  ```

- **`history`**: Prints the command history stored in the shell.

  Example:

  ```bash
  1 ls -la
  2 cd ..
  3 cat file.txt

  ```

- **`!n`**: Prints the command history stored in the shell.

  Example:

  ```bash
  !2  # Re-executes the command at history index 2

  ```

- **`bg <job>`**:
  Resumes the specified background job. <job> can be the job ID (preceded by %) or the process ID.

- **`fg <job>`**:
  Resumes the specified foreground job, waiting for its completion. <job> can be the job ID (preceded by %) or the process ID.

- **`kill <signal> <job>`**:
  Sends a signal to the specified job. <signal> is the signal number, and <job> can be the job ID (preceded by %) or the process ID.

  Example:

  ```bash
  kill 9 %2  # Sends SIGKILL to the job with ID 2

  ```
