#!/bin/sh

find . -name "*.[ch]" -print | etags -
