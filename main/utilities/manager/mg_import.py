from nscldaq.mg_database import (
    Container, EventLog, Program
)
import sqlite3

import sys






def qualify_name(name, qualification):
    return f'{qualification}-{name}'


#   Import the containers from the source db:

def import_containers(inp, name, out):
    in_container_api = Container(inp)
    out_container_api = Container(out)
    
    #  Get the definitions to import: and cook the names:
    
    containers = in_container_api.list()
    for c in containers:
        c['name'] = qualify_name(c['name'], name)
        
    # Now add the definitions to the exp database...we're going to have to cook the definition
    # a bit since we only have the initscript ctonetns, not the initscript
    
    for c in containers:
        out_container_api.add(c['name'], c['image'], None, c['bindings'])
        
        # Update the  input script:
        
        cursor = out.cursor()
        cursor.execute(
'''
    UPDATE container SET init_script=? WHERE container = ?
    
''', (c['init_script'], c['name'])
        )
    out.commit()
    


if len(sys.argv) != 4:
    sys.stderr.write('''
Usage:
    mg_import input_db name exp_db
    
Where:
    input_db - is the experiment database to be imported into this experiment.
    name     - Is the imported experiment name.  This is used to uniquify names from the imported experiment.
    exp_db   - Is the experiment database into which the definitions in input_db are imported.
         
    ''')
    sys.exit(-1)
        

# open the databases and  get the import name.
    
    
db_to_import  = sqlite3.connect(sys.argv[1])
import_name   = sys.argv[2]
exp_db        = sqlite3.connect(sys.argv[3])


import_containers(db_to_import, import_name, exp_db)

                                             