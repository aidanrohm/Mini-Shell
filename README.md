<h1 align="center">🐚 Mini Shell</h1>

<p align="center">
  A Unix-style command-line interpreter written in C, with command history, I/O redirection,<br>
  readline tab completion, a colorized <code>ls</code>, and an isolated working environment.
</p>

---

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Getting Started](#getting-started)
- [Usage and Examples](#usage-and-examples)
- [How It Works](#how-it-works)
- [Known Limitations](#known-limitations)
- [Notes](#notes)

---

## Overview

Mini Shell was built as a final project for a systems course (CISC 5590, Fordham University). At its core it does what a shell should: for every command you enter, the shell forks a child process, runs the command with `execvp()`, and the parent waits for it to finish before showing the next prompt. Any program on your `PATH` works, including `vim`, `nano`, an interactive Python session, and even `ssh`.

I was ambitious with the features I proposed, and not everything made the cut (see [Known Limitations](#known-limitations)). What did make it in is listed below.

## Features

| Feature | Description |
| --- | --- |
| 📁 **Isolated environment** | Starts in a persistent sandbox directory at `/tmp/shell_env`, so you begin in an empty folder instead of your real files. |
| 🕘 **Command history** | Keeps the last 20 unique commands. Re-running a command bumps it to the most recent spot. Recall with `!n`. |
| ↔️ **I/O redirection** | Supports `<` and `>` for input and output redirection. |
| ⇥ **Tab completion** | Powered by GNU readline. Completes commands for the first word and file/directory names after that. |
| 🎨 **Colorized `ls`** | Directories print in blue so they stand out from files. |
| 🛡️ **Argument check** | Catches `cat`, `cp`, `mv`, and `rm` with no arguments instead of hanging. |

## Getting Started

### Prerequisites

You need a C compiler (`gcc`) and the GNU readline library.

```bash
# macOS (Homebrew)
brew install readline

# Debian / Ubuntu
sudo apt install libreadline-dev
```

### Build

```bash
gcc -o MyShell Mini_Shell.c -lreadline
```

### Run

```bash
./MyShell
```

> **Note:** On macOS, Homebrew's readline may not be on the default compiler path. If the build can't find `readline/readline.h`, add the include and library flags, for example `-I$(brew --prefix readline)/include -L$(brew --prefix readline)/lib`.

## Usage and Examples

### Isolated environment

On first launch you will see something like this:

```text
Isolated environment created: /tmp/shell_env
Welcome to Aidan's Shell!
/tmp/shell_env/- <<>>
```

The directory starts empty, so `ls` prints nothing. Create a file and it shows up:

```text
/tmp/shell_env/- <<>> touch HelloWorld.cpp
/tmp/shell_env/- <<>> ls
HelloWorld.cpp
```

The directory persists between runs, so you will land in the same place every time. If you `cd` into a subdirectory, the path in front of the `<<>>` prompt updates to match.

**Cleaning up:** exit the shell with `exit`, then remove the sandbox:

```bash
rm -rf /tmp/shell_env
```

### Command history

```text
/tmp/shell_env/- <<>> echo cat
cat
/tmp/shell_env/- <<>> history
1: echo cat
/tmp/shell_env/- <<>> !1
Executing: echo cat
cat
```

Running a command from history (`!1`) also adds it to the history.

### Redirection

```text
/tmp/shell_env/- <<>> echo cat > output.txt
/tmp/shell_env/- <<>> cat output.txt
cat
```

### Tab completion

Type the start of a command and press <kbd>Tab</kbd>:

```text
/tmp/shell_env/- <<>> ec<Tab>
/tmp/shell_env/- <<>> echo
```

## How It Works

The source is organized around ten functions.

| Function | Purpose |
| --- | --- |
| `TrimInput` | Cleans up whitespace in the raw user input. |
| `ParseUserInput` | Tokenizes the input to separate the command from its arguments. |
| `AddToHistory` | Adds a command to a 20-entry queue; duplicates move to the newest position. |
| `PrintHistory` | Prints the numbered history. |
| `ExecuteCommandFromHistory` | Handles `!n`. Runs the chosen command in its own child process to keep `main()` uncluttered. |
| `HandleRedirection` | Scans the tokens for `<` and `>` and sets up the input and output files. |
| `CommandRequiresArguments` | Flags commands that would otherwise hang when given no arguments. |
| `CommandGenerator` | Returns tab-completion matches from a built-in list of common commands. |
| `CustomCompletion` | Chooses between command completion (first word) and file/directory completion (everything else). |
| `ExecuteLS` | Custom `ls` that prints directories in blue. |

## Known Limitations

- **No piping.** This was the main feature I did not get to.
- **Argument checking is partial.** `CommandRequiresArguments` covers only a few commands. For others, the OS handles missing arguments, so behavior can be inconsistent. Some hangs (like a bare `cat` waiting on input) mirror what you would see in a regular Zsh shell, and I left those as is.
- **Experimental.** Any command on your `PATH` should work, but there are far too many to test individually.

## Notes

I wrote this on a school server (Erdos) at first, but it did not have the readline library available to me, so I moved most of the development to my own laptop. A few compiler errors showed up after the move, mostly when I added tab completion. If you hit build issues, make sure readline is installed and visible to your compiler.

---

<p align="center">Built by Aidan Rohm</p>
