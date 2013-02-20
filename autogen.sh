#!/bin/sh

libtoolize --install && \
automake --add-missing && \
autoreconf -v
