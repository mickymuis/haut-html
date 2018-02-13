Haut HTML-parser
================

Haut is (yet another) HTML-tokenizer and -parser that is focussed on speed and uses a unique *pure* Finite State Machine (FSM) approach. It is written in C99 with zero external dependencies and should be very easy to include in your projects. Target applications are web crawlers, machine learning, data mining and usage as an educational tool.

The name *Haut* is based on the French word *haute* meaning 'high' as in 'high speed' (haute vitesse).

Features
--------
Currently Haut has near-complete support for all HTML5 language features, and:
* SAX-based (event-driven) API that is very easy to use
* Very fast and small memory footprint
* Resilient against mistakes in HTML-code
* Support for all HTML5 elements, text nodes, attributes, DOCTYPE-sections, SCRIPT-sections, CDATA-sections, comments and character references.
* Support for UTF-8 formatted input and UTF-8 compliant output
* Parsing of partial data (chunks) or a the whole buffer at once

Getting Started
---------------

Haut can simply be compiled using a C99-compliant compiler (or MSVC >= 2013). There are no external dependencies. First of all, obtain the sources by  clone into the Git repository:

```
git clone https://github.com/mickymuis/haut-html.git
```

Currently, a simple Unix-style Makefile is provided and on Unix-like systems a simple `make` command will suffice. Furthermore, the `examples/` folder contains some simple demonstrations of how the library is used.
There is also a Visual Studio (2013) solution included in the folder `visualc`.

Finite State Machine
--------------------

While Haut is a very simple parser - it has only a handful functions and under 1000 lines of C-code - under the hood it uses another concept to define its behavior. Haut utilizes four different pure Finite State Machines to do 90% of its parsing. These FSMs are coded as tables in linear memory and need no branching or character comparisons, making it extremely fast. While there are already many FSM solutions available, Haut uses its own FSM-compiler called *fsm2array*. fsm2array is a very simplistic program that translates a simple syntax into C-style arrays. Normally these FSMs are precompiled in the `src/` directory but all the tools you need to work with them are provided in the `util` directory.

Contributing
------------

Help me make Haut an awesome parsing by contributing to this project. You can send me bug reports and feedback through Github. Please also include the HTML-source that you used (if applicable).
If you need pointers on testing or other forms of contributing please contact me.
