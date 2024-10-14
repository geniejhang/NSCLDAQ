#!/usr/bin/env python3 
'''
This script provides a Qt5 wizard for making 
programs.  Programs have:

A name, a host, a container, and an executable.

They have a type which might be:
e.g. 'Critical.

They have an execution environment that consists of:

A working directory, environment variables, an initialization script.

They are parameterized by options which typically (but need not) have values.
And parameters which are values.



Usage:
   $DAQBIN/mg_program_wizard database-file

'''
import sys
import sqlite3
from nscldaq.mg_database import Program, Container
from pathlib import Path

def Usage():
    sys.stderr.write(
        '''
Usage:
    $DAQBIN/mg_program_wizard config-file-path
Where:
    config-file-path - is the path to the configuration file to edit.
        '''
    )
    sys.exit(-1)
    
    
    

#  Entry point:

if len(sys.argv) != 2:
    Usage()
    
config = sys.argv[1]
p = Path(config)
if not p.exists():
    sys.stderr.write(f'No such config file "{config}"\n')
    exit(-1)
    
    
db = sqlite3.connect(config)