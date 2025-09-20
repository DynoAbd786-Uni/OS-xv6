# xv6 Operating System Development & Custom Shell Implementation

[![C](https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white)](https://en.wikipedia.org/wiki/C_(programming_language))
[![RISC-V](https://img.shields.io/badge/RISC--V-283272?style=for-the-badge&logo=riscv&logoColor=white)](https://riscv.org/)
[![Operating Systems](https://img.shields.io/badge/Operating%20Systems-FF6B6B?style=for-the-badge&logo=linux&logoColor=white)](https://en.wikipedia.org/wiki/Operating_system)
[![Unix Shell](https://img.shields.io/badge/Unix%20Shell-4EAA25?style=for-the-badge&logo=gnubash&logoColor=white)](https://en.wikipedia.org/wiki/Unix_shell)

> **Advanced Operating Systems Coursework (COMP2211)** - University of Leeds, Year 2
> Complete implementation of Unix-style shell with multi-element pipelines, I/O redirection, and custom system calls for the xv6 educational operating system.

## 📋 Table of Contents

- [🎯 Project Overview](#-project-overview)
- [🏗️ Architecture](#️-architecture)
- [✨ Features](#-features)
- [🔧 Technical Implementation](#-technical-implementation)
- [📁 Project Structure](#-project-structure)
- [🚀 Getting Started](#-getting-started)
- [💻 Usage Examples](#-usage-examples)
- [🧪 Testing](#-testing)
- [📚 Documentation](#-documentation)
- [🏆 Academic Achievement](#-academic-achievement)

## 🎯 Project Overview

This repository contains a comprehensive **11-week operating systems coursework** implementing advanced systems programming concepts through the xv6 educational operating system. The project demonstrates deep understanding of:

- **Operating System Internals**: Process management, memory allocation, system calls
- **Advanced Shell Implementation**: 582 lines of sophisticated C code with enterprise-level features
- **Kernel Development**: Custom system calls and kernel modifications
- **Process Communication**: Pipes, file descriptors, and inter-process communication

### 🎓 Academic Context

**Course**: COMP2211 - Operating Systems (2023/24)
**Institution**: University of Leeds, School of Computing
**Level**: Year 2 Computer Science
**Assessment**: Major coursework worth 30% of module grade

## 🏗️ Architecture

### Core Components

```
xv6-riscv/
├── kernel/          # Operating system kernel
│   ├── syscall.c    # System call implementations
│   ├── proc.c       # Process management
│   └── ...
├── user/            # User space programs
│   ├── my_shell.c   # Custom shell implementation (582 lines)
│   ├── ping-pong.c  # IPC demonstration
│   ├── find.c       # Directory search utility
│   └── ...
└── Makefile         # Build system configuration
```

### Technical Foundation

- **Platform**: xv6 operating system (MIT's Unix v6 reimplementation)
- **Architecture**: RISC-V with QEMU emulation
- **Development**: Containerized toolchain (Singularity/Docker)
- **Language**: Advanced C systems programming

## ✨ Features

### 🖥️ Advanced Shell Implementation

Our custom shell (`my_shell.c`) implements enterprise-grade features:

#### Multi-Element Pipelines
```bash
>>> ls | grep test | wc -l | cat
```
- **Recursive processing** with inverse recursion algorithm
- **Proper file descriptor management** for complex chains
- **Dynamic command parsing** and execution trees

#### I/O Redirection
```bash
>>> echo "Hello World" > output.txt
>>> cat < input.txt
>>> ls | grep test > results.txt
```
- **Input redirection** (`<`) from files to stdin
- **Output redirection** (`>`) from stdout to files
- **Combined with pipelines** for complex data flow

#### Sequential Command Execution
```bash
>>> make clean; make; echo "Build complete"
```
- **Semicolon operator** (`;`) for command sequences
- **Independent execution** with proper error handling
- **Process synchronization** between commands

### 🔧 System Programming Features

#### Custom System Calls
- **New kernel functions** added to xv6
- **User space interfaces** for seamless integration
- **Complete implementation** across 5 system files

#### Process Management
- **Fork/exec/wait patterns** for process creation
- **Inter-process communication** via pipes
- **Process synchronization** and lifecycle management

#### Memory Management
- **Dynamic allocation** with malloc/free optimization
- **16-byte memory alignment** for performance
- **Memory leak prevention** and debugging tools

## 🔧 Technical Implementation

### Shell Architecture

The shell implementation follows a sophisticated multi-stage architecture:

```c
// Command execution pipeline
typedef struct {
    char *argv[MAXARG];     // Command arguments
    char *endBuffer;        // Buffer boundary
    int endOfLine;          // Line termination marker
} execCommand;

// Processing stages
parseCommand() → tokenize() → parseLine() → walkCommand() → runCommand()
```

### Key Algorithms

#### Pipeline Processing
```c
void runPipelines(execCommand *commands[], int numPipes) {
    // Recursive pipeline implementation with proper
    // file descriptor management and process coordination
    if (execFork() == 0) {
        // Child: setup pipe output, recurse or execute
        close(1); dup(fds[1]); close(fds[0]); close(fds[1]);
        if (numPipes > 1) runPipelines(commands, numPipes - 1);
        else parseExec(commands[numPipes - 1], 0, commands[numPipes - 1]->endOfLine);
    } else {
        // Parent: setup pipe input, execute final command
        close(0); dup(fds[0]); close(fds[0]); close(fds[1]);
        parseExec(commands[numPipes], 0, commands[numPipes]->endOfLine);
    }
}
```

## 📁 Project Structure

### Key Files

| File | Purpose | Lines | Description |
|------|---------|-------|-------------|
| `user/my_shell.c` | Custom Shell | 582 | Complete shell implementation with advanced features |
| `user/ping-pong.c` | IPC Demo | 44 | Inter-process communication via pipes |
| `user/find.c` | File Search | 81 | Recursive directory tree search utility |
| `kernel/syscall.c` | System Calls | Modified | Custom system call implementations |
| `COMP2211-2023-24-lab-manual.pdf` | Documentation | 39 pages | Complete lab manual and requirements |

### User Programs Implemented

- **sleep**: Process suspension for specified time ticks
- **ping-pong**: Parent-child communication demonstration
- **find**: Recursive file search with pattern matching
- **xargs**: Command execution with piped input processing

## 🚀 Getting Started

### Prerequisites

- **Linux Environment**: Ubuntu/Debian preferred
- **RISC-V Toolchain**: Cross-compilation tools
- **QEMU**: RISC-V system emulation
- **Singularity/Docker**: Containerized development environment

### Installation

1. **Clone the repository**:
   ```bash
   git clone https://github.com/DynoAbd786-Uni/OS-xv6-Operating-System.git
   cd OS-xv6-Operating-System
   ```

2. **Set up development environment**:
   ```bash
   # Download containerized toolchain
   singularity pull docker://callaghanmt/xv6-tools:buildx-latest

   # Enter development container
   singularity shell xv6-tools_buildx-latest.sif
   ```

3. **Build and run xv6**:
   ```bash
   # Compile the operating system
   make clean && make

   # Boot xv6 with QEMU emulation
   make qemu
   ```

4. **Test custom shell**:
   ```bash
   # Inside xv6
   $ myshell
   >>> ls | grep user | wc -l
   >>> echo "Hello xv6" > test.txt
   >>> cat test.txt
   ```

### Quick Start Commands

```bash
# Build everything
make clean && make

# Run with debugging support
make qemu-gdb

# Exit QEMU emulation
# Press Ctrl+A, then X
```

## 💻 Usage Examples

### Basic Shell Operations

```bash
# Start custom shell
$ myshell

# Simple command execution
>>> ls -la
>>> echo "Hello World"

# Change directory (built-in command)
>>> cd user
>>> pwd
```

### Advanced Pipeline Examples

```bash
# Multi-stage pipeline
>>> ls | grep ".c" | wc -l

# Pipeline with I/O redirection
>>> find . "*.c" | xargs grep "main" > results.txt

# Sequential commands
>>> make clean; make; echo "Build complete"
```

### System Call Testing

```bash
# Test custom system calls
$ hw           # Custom "Hello World" program
$ getyear      # Returns Unix v6 release year (1975)
$ sleep 5      # Process suspension
$ pingpong     # IPC demonstration
```

## 🧪 Testing

### Automated Testing

The project includes comprehensive testing frameworks:

```bash
# Memory management stress tests
$ malloc_stress

# Shell functionality tests
$ find . "test" | xargs grep "function"

# Process communication tests
$ pingpong
```

### Manual Testing Scenarios

1. **Pipeline Stress Test**:
   ```bash
   >>> ls | grep test | cat | wc -l | echo
   ```

2. **I/O Redirection Validation**:
   ```bash
   >>> echo "test data" > temp.txt
   >>> cat < temp.txt > output.txt
   >>> cat output.txt
   ```

3. **Error Handling**:
   ```bash
   >>> nonexistent_command
   >>> cat < missing_file.txt
   >>> ls | invalid_program
   ```

## 📚 Documentation

### Academic Resources

- **[Lab Manual](COMP2211-2023-24-lab-manual.pdf)**: Complete 39-page coursework guide
- **[xv6 Book](https://pdos.csail.mit.edu/6.S081/2022/xv6/book-riscv-rev3.pdf)**: Technical reference
- **Source Code Comments**: Extensive inline documentation

### Key Learning Outcomes

- **Systems Programming**: Advanced C programming for operating systems
- **Process Management**: Fork, exec, wait patterns and IPC mechanisms
- **Memory Management**: Dynamic allocation, optimization, and debugging
- **Kernel Development**: System call implementation and kernel modification
- **Software Architecture**: Modular design for complex systems

## 🏆 Academic Achievement

### Technical Complexity

- **582 Lines of Shell Code**: Advanced command parsing and execution
- **Multi-Element Pipelines**: Recursive algorithm implementation
- **Enterprise Features**: I/O redirection, sequential commands
- **Kernel Programming**: Custom system calls and OS modification
- **Memory Optimization**: Advanced allocation strategies

### Skills Demonstrated

- **Low-Level Programming**: Direct system interface manipulation
- **Algorithm Design**: Complex parsing and execution algorithms
- **Error Handling**: Comprehensive validation and recovery
- **Performance Optimization**: Memory alignment and process efficiency
- **Professional Practices**: Testing, documentation, version control

### Assessment Context

This coursework represents **exceptional technical depth** for Year 2 Computer Science:

- **Industry-Level Complexity**: Enterprise shell features
- **Graduate-Level Concepts**: Kernel development and system programming
- **Professional Standards**: Comprehensive testing and documentation
- **Real-World Applications**: Direct relevance to systems development

---

## 📞 Contact & Support

**Developer**: Abdullah
**Institution**: University of Leeds, School of Computing
**Course**: COMP2211 - Operating Systems (2023/24)

For technical questions about implementation details or academic context, please refer to the comprehensive documentation provided in the repository.

---

*This project demonstrates advanced systems programming skills through practical implementation of operating system concepts, representing significant technical achievement in undergraduate computer science education.*