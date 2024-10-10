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
                Note handle ownership is not transferred to the 
                object.. you can happily pass the same handle
                to more than  one Container constructor or
                even to more than one type of database object.
        '''
        self._db = handle
        
    def _exists(self, name):
        # True if name is already a container.
        
        c  = self._db.cursor()
        res = c.execute('SELECT COUNT(*) FROM container WHERE container = ?', (name,))
        
        (count, ) = res.fetchone()
        return count != 0
    
        
    # Public interface:
    
    def exists(self, name):
        ''' determines if the container 'name' exists. '''
        return self._exists(name)

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
       
        #  We have enough to create the container ...
        # Which we do inside a transaction that is implicitly opened:
        # In case of error, we roll it back:
        cursor = self._db.cursor()
        try:
            cursor.execute(
                '''INSERT INTO container (container, image_path, init_script)
                        VALUES(?,?,?)
                ''',
                (name, image, init_contents)
            )
            container_id = cursor.lastrowid
            for binding in mountpoints:
                source = binding[0]
                if len(binding) > 1:
                    dest = binding[1]
                else:
                    dest = binding[0]
                cursor.execute('''
                        INSERT INTO bindpoint (container_id, path, mountpoint)
                            VALUES (?,?,?)
                    ''',
                    (container_id, source, dest)
                )
            self._db.commit()
        except:
            self._db.rollback()
            raise
        
    def remove(self, name):
        '''
            Removes the named container definition.
            Raises a value error if there is no such container.
        '''
        
        #  Get the id of the container.
        
        cursor = self._db.cursor()
        res = cursor.execute('''
            SELECT id FROM container WHERE container = ?
                                ''', (name,)                   
        )
        id = res.fetchone()
        if id is None:
            raise ValueError(f'No such container {name}')
    
        cursor.execute(
            '''
                DELETE FROM bindpoint WHERE container_id = ?
            ''', id
        )
        cursor.execute(
            '''
                DELETE FROM container where id = ?
            ''', id
        )
        
        self._db.commit()
    
    def replace(self, oldname, newname, image, initscript, mountpoints):
        '''
            Removes the container 'oldname' and replaces it with the
            container definition in newname, image, initsscript and
            mountpoints.
            
            This is just a convenience  front end to 'remove' followed by add.
            
            Note that while delete and add are both done in a transaction,
            those transactions are independent, so it's possible the delete will suceed
            but somehow the add will fail.
        '''
        self.remove(oldname)
        self.add(newname, image, initscript, mountpoints)
    
    def list(self):
        '''
            Returns a list of all containers.  Container lists are returned as
            a list of dicts with the keys:
            
            * name  - Name of the container.
            * image - Container image file.
            * init_script - the contents of the init_script.
            * bindings - The bindings specifications.  THese are an iterable containing
             two element lists of binding source binding destination.  For example,
             the binding of /usr/opt/opt-buster -> /usr/opt will be:
             ('/usr/opt/opt-buster', '/usr/opt'), For identity bindings both elements will
             be the same.
        '''

        containers = {}       # Indexed by container name.
        cursor = self._db.cursor()
        r = cursor.execute(
            '''
            SELECT container, image_path, init_script, path, mountpoint
            FROM container
            INNER JOIN bindpoint ON bindpoint.container_id = container.id
            '''
        )
        while True:
            row = r.fetchone()
            if row is None:
                break
            
            if row[0] not in containers.keys():
                name = row[0]
                image = row[1]
                script = row[2]
                containers[name] = {
                    'name': name, 'image': image, 'init_script': script, 'bindings': []
                }
        
            # Append the bindings to the container def:
            
            containers[name]['bindings'].append((row[3], row[4]))
        
        return list(containers.values())