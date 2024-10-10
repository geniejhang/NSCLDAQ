''' 
Provides a python interface to the manager database definition.  This
will grow as more python GUIs are added, I imagine.

The use case is to provide support for PyQt user interfaces for configuration
and so on.
'''

import sqlite3


class Container:
    def __init__(self, handle):
        ''' Construct an interface to the container part of a db.
            
            handle - is an sqlite3 handle open on the database.
        '''
        self._db = handle
        
    def _exists(self, name):
        # True if name is already a container.
        
        c  = self._db.cursor()
        c.execute('SELECT COUNT(*) FROM container WHERE container = ?', (name,))
        
        (count, ) = c.fetchone()
        return count != 0
    
        
    # Public interface:

    def add(self, name, image, initscript, mountpoints):
        '''
            Add a new container to the database.
            
            name - name of the container - this is a 'handle' the user can use to refer to the
                  container.
            image - The container image file in the host filesystem.
            initscript - Path to the initscript that will be  used to intialize the container 
                environment for programs run in it.  Note:
                *  The file path is in the host.
                *  The script will be sucked into the database rather than referenced externally.
                *  If the value of ths is None, then no init script is provided for the
                container.
            mountpoints -  
                A list of host -> container filesystem bindings.  Each binding is a one or
                two element list.  If a one element list the target for the binding will be
                the same as it is in the host.  If a two element list the target is specified
                in the second element so, for example:
                 (/mnt/evtdata/0400x,) - mounts /mnt/evtdata/0400x -> /mnt/evtdata/0400x in the container.
                 (/usr/opt/opt-buster, /usr/opt) mounts /usr/opt/opt-buster -> /usr/opt in the container
            
        '''
        
        # Ensure the name is unique:
        
        if self._exists(name):
            raise ValueError(f'{name} is already a container')
        
        # Suck in the init script ...raises if the file is not found.
        
        init_contents = ''
        if initscript is not None:
            with open(initscript, 'r') as file:
                init_contents = file.read()
       

        