''' 
Provides a python interface to the manager database definition.  This
will grow as more python GUIs are added, I imagine.

The use case is to provide support for PyQt user interfaces for configuration
and so on.
'''

import sqlite3
def boolToInt(b):
    return  1 if b else 0
def boolToInt(b):
    return  1 if b else 0


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
    
    def id(self, name):
        '''
           Returns either the id of the named container or None if there is no match.
        '''
        
        cursor = self._db.cursor()
        result = cursor.execute('''
            SELECT id FROM container WHERE container = ?
                                ''', (name,))
        row = result.fetchone()
        if row is None:
            return None
        else:
            return row[0]
class EventLog:
    def __init__(self, handle):
        '''
            Supports editing the event log definition database.
            handle - the sqlite3 database handle open on the configuration database.
               The caller retains ownership.
               The same handle can be passed to as many database facade objects as desired.
        '''
        self._db = handle

    # Private methods:
    
    def _makeDict(self, row):
        '''
           Given a row delected from e.g. info or list, produces a dict
           that describes the logger in that row.  Note both of these do the
           appropriate inner joing with container to get the container name not id.
        '''
        
        return {
            'id': row[0],
            'root': row[1],
            'ring' : row[2],
            'host': row[3],
            'partial': True if row[4] > 0 else False,
            'destination': row[5],
            'critical': True if row[6] > 0 else False,
            'enabled' : True if row[7] > 0 else False,
            'container': row[8]
            
        }
    
    def _idExists(self, id):
        # Return True if the logger specified by ID exists.
        
        r = cursor = self._db.cursor();
        cursor.execute(
            '''
                SELECT COUNT(*) FROM logger WHERE id = ?
            ''', (id, )
        )
        (count, ) = r.fetchone()
        return count > 0
    
    # PUblic methods
    def exists(self, destination):
        '''
           Determines if there's a logger on the destination. 
           
           Returns boolean, True if there is one and False if not
        '''        
        cursor = self._db.cursor()
        r = cursor.execute(
            '''
            SELECT COUNT(*) from logger WHERE destination = ?
            ''', (destination,)
        )
        (count, ) = r.fetchone()
        return count != 0
        
    def add(self, root, source_uri, destination, container, host, options):
        '''
            Adds a new definition of an event logger to the syhstem.  One may only have
            one logger to a destination. Parameters are:
            *  root - NSCLDAQ root directory from which eventlog will be used.
            *  source_uri - URI of the ringbuffer from which the data will be logged.
            *  destination - Destination directory in which the logging will be done. 
                 See, however  the 'partial' option.
            * container - name of the container the logger will run in.  Note the
                Tcl world allows containerless loggers, but we don't.
            * host - the host in which the logger runs.
            * options:  A dict of options which provide additional control over the
               logger.  Keys that matter are:
                * 'partial' - Boolean valued which, if true, specifies the event logger runs
                    like the old multilogger, just accumulating event files in that directory.
                    The default for this is False (note this is different from the Tcl API).
                * 'critical' - Boolean, if True, If the logger exits unexpectedly,   
                   The experiment shuts down and needs to be rebooted.  This is True by default 
                * 'enabled' boolean that if True means the logger is enabled and will, once the
                   next run begins, start logging data. Default is True.
            On success, returns the id of the logger created.
        '''
        # Make sure the destination is unique.
        
        if self.exists(destination):
            raise ValueError(f'There is already a logger saving data at {destination}')
        
        #  Get the container
        
        c = Container(self._db)
        container_id = c.id(container)
        if container_id is None:
            raise ValueError(f'There is no container named {containr}')
        
        # Untangle the options:
        
        partial = 0
        critical = 0
        enabled = 0
        option_keys = options.keys()
        if 'partial' in option_keys:
            opt = options['partial']
            if type(opt) != bool:
                raise ValueError(f'The value of the "partial" option must be a boolean it was {opt}')
            partial = boolToInt(opt)
        if 'critical' in option_keys:
            opt = options['critical']
            if type(opt) != bool:
                raise ValueError(f'The value of the "critical" option must be a boolean it was {opt}')
            critical = boolToInt(opt)
        if 'enabled' in option_keys:
            opt = options['enabled']
            if type(opt) != bool:
                raise ValueError(f'The value of the "enabled" option must be a boolean it was {opt}')
            enabled = boolToInt(opt)
            
        # Now we can do the insert.
        
        cursor = self._db.cursor()
        cursor.execute(
            '''
            INSERT INTO logger 
                (daqroot, ring, host, partial, destination, critical, enabled, container_id)
            VALUES
                (?,?,?,?,?,?,?,?)
            
            ''',
            (root, source_uri, host, partial, destination, critical, enabled, container_id)
        )
        result = cursor.lastrowid
        self._db.commit()
        return result
        
    def info(self, destination):
        '''
        Returns a dict that describes the logger to destination (or  None if there is no such logger)
        The return value has the following keys (note that the dict can be used as the options
        for a create):
        
        * 'id'  -  Logger id (row' primary key).
        * 'root' - DAQROOT for the logger.
        * 'ring' - Ring URI from which the data are logged.
        * 'host' - Host on which the logger runs.
        * 'partial' - Bool that is true if the logger is partial.
        * 'destination' - Where the data are being logged.
        * 'critical' - Bool that is true if the logger is specified to be critical.
        * 'enabled' - Bool that is true if the logger is enabled.
        * 'container' - name of the container the logger runs in.
        '''
        
        cursor = self._db.cursor()
        r = cursor.execute(
            '''
            SELECT logger.id, daqroot, ring, host, partial, destination, critical, enabled, container
            FROM logger
            INNER JOIN container ON container.id = container_id
            WHERE destination = ?
            ''', (destination,)
        )
        row = r.fetchone()
        if row is None:
            return None
        
        return self._makeDict(row)
    def list(self):
        cursor = self._db.cursor()
        r = cursor.execute(
            '''
            SELECT logger.id, daqroot, ring, host, partial, destination, critical, enabled, container
            FROM logger
            INNER JOIN container ON container.id = container_id
            ''')
        result = []
        while True:
            row = r.fetchone()
            if row is None:
                break
            result.append(self._makeDict(row))
        return result
        
    def delete(self, id):
        if not self._idExists(id):
            raise ValueError(f"There is no logger with the id {id}")

        cursor = self._db.cursor()
        cursor.execute(
            '''
            DELETE FROM logger WHERE id = ?
            ''', (id,)
        )
        self._db.commit()
        
    def enable(self, id):
        if not self._idExists(id):
            raise ValueError(f"There is no logger with the id {id}")
        cursor = self._db.cursor()
        cursor.execute(
            '''
            UPDATE logger SET enabled = 1 WHERE id = ?
            ''', (id,)
        )
        self._db.commit()
    def disable(self, id):
        if not self._idExists(id):
            raise ValueError(f"There is no logger with the id {id}")
        cursor = self._db.cursor()
        cursor.execute(
            '''
            UPDATE logger SET enabled = 0 WHERE id = ?
            ''', (id,)
        )
        self._db.commit()
    def enable_all(self):
        
        cursor = self._db.cursor()
        cursor.execute(
            '''
            UPDATE logger SET enabled = 1
            '''
        )
        self._db.commit()
    def disable_all(self):
        cursor = self._db.cursor()
        cursor.execute(
            '''
            UPDATE logger SET enabled = 0
            '''
        )
        self._db.commit()
    def start_recording(self):
        cursor = self._db.cursor()
        cursor.execute(
            '''
            UPDATE recording SET state = 1
            '''
        )
    def stop_recording(self):
        cursor = self._db.cursor()
        cursor.execute(
            '''
            UPDATE recording SET state = 0
            '''
        )
        self._db.commit()
    def is_recording(self):
        cursor = self._db.cursor()
        res = cursor.execute(
            '''
            SELECT state FROM recording
            '''
        )
        row = res.fetchone()
        if row is None:
            raise RuntimeError("is_recording - was not able to fetch a rwo from recording")
        
        return True if row[0] != 0 else False