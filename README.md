Haut HTML-parser
================

Haut is (yet another) HTML-tokenizer and -parser that is focussed on speed and uses a unique *pure* Finite State Machine (FSM) approach. It is written in C99 with zero external dependencies and should be very easy to include in your projects. Target applications are web crawlers, machine learning, data mining and usage as an educational tool.

The name *Haut* is based on the French word *haute* meaning 'high' as in 'high speed' (haute vitesse).

Features
--------
Currently Haut has near-complete support for all HTML5 language features, and:
* SAX-based (event-driven) API that is very easy to use
* Very fast and small memory footprint
* Resillient against mistakes in HTML-code
* Support for all HTML5 elements, text nodes, attributes, DOCTYPE-sections, SCRIPT-sections, CDATA-sections, comments and character references.
* Support for UTF-8 formatted input and UTF-8 compliant output
In the future we would like to add:
* Support for character streaming (as opposed to operating on the whole buffer)
* Character encodings other than UTF-8

Getting Started
---------------

Haut can be simply compiled using a C99-compliant compiler. There are no external dependencies. First of all, obtain the sources by  clone into the Git repository:

```
git clone https://github.com/mickymuis/haut-html.git
```

Currently, only a Unix-style Makefile is provided and on Unix-like systems a simple `make` command will suffice. Furthermore, the `examples/` folder contains some simple demonstrations of how the library is used.
