# tui-notes

A simple terminal-based note-taking application written in C using ncurses.

## Features

- Create, edit, and delete notes
- Navigate with arrow keys
- Fast and lightweight
- Notes stored as plain text files in `~/.notes/`
- Clean TUI interface

## Requirements

- Termux (or any Linux terminal)
- ncurses
- A text editor (nano, vim, micro, etc.)

## Installation

```bash
## Install dependencies (Termux)
pkg install ncurses nano gcc make

## Clone or download the source
git clone https://github.com/enrilinux/tui-notes.git
cd tui-notes

## Build and install
make && make install
```

Usage

Simply run:

```bash
tui-notes
```

## Keyboard Shortcuts

Key Action
↑/↓ Navigate notes
Enter or e Edit selected note
n Create new note
d Delete selected note
Home Go to first note
End Go to last note
q Quit application

## Note Storage

Notes are saved as .txt files in:

```
~/.notes/
```

## Editor

By default, tui-notes uses the editor defined in your $EDITOR environment variable. If not set, it falls back to nano.

To change the default editor:

```bash
export EDITOR=vim
```

## Building from Source

```bash
make          # Build the application
make debug    # Build with debugging symbols
make clean    # Remove compiled files
make install  # Install to $PREFIX/bin or ~/.local/bin/
make uninstall # Remove the application
make run      # Build and run
make help     # Show available commands
```

## Uninstall

```bash
make uninstall
```

## Contributing

Pull requests are welcome. For major changes, please open an issue first.

## License

MIT License
