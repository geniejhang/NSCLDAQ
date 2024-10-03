'''
  This is the model for our ring item display.  The model has the following hierarchy:
  
  host   - a host in the model.
    ring   - information about the ring.
      Consumers
           - information about the consumers for that ring
      Remote
           proxy_ring - information about remote proxy
             Consumers information about the consumers for that proxy ring.
             
The following headeer4s exist:
  Column 0 - none
  Column 1 - Name - ringname used by ring item names.
  Column 2 - producer - Producer command - used by ring items
  COlumn 3 - producer pid - used by ring items.
  Column 4 - Size - size of ring in k used by ring items.
  Column 5 - Free space - bytes free in ring - used by ring items.
  Column 6 - Consumer - consumer command, used by consumers.
  Column 7 - pid - Process id of consumer used by consumers.
  Column 8 - backlog - back log used by consumers.

'''
import pprint
from PyQt5.QtGui import QStandardItem, QBrush
from PyQt5.Qt import *

# Columns:
RING_NAME = 1
RING_PRODUCER = 2
RING_PRODUCER_PID = 3
RING_SIZE = 4
RING_FREE=5
CONSUMER_CMD = 6
CONSUMER_PID = 7
CONSUMER_BACKLOG = 8

NUM_COLUMNS = 9

class RingModel:
    def __init__(self, view, alarm_pct):
        '''
        Initialize a RIngModel where:
        view - is the RingView whose data we manage.
            The view's model must be set and be a QStandardItemModel.
        
        alarm_pct - percentage backlog that represents a stall.
            Rings are marked red if their free space is below 100-alarm_pct of ring size.
            Consumers are marked red if their backlog is above alarm_pct of 
            the ring size. 
            Hosts and Proxy items are marked red if they have any rings that are red
            (note that a red ring implies a consumer that's red by definition).
            
        
        '''
        self._model = view.model()
        self._alarm = alarm_pct
        self._debug = False   # Enable debug printing.
        
        #  Make an item so we can get the normal background brush:
        
        fake_item = QStandardItem('')
        self._okbrush = fake_item.foreground()
        self._errbrush = QBrush()
        self._errbrush.setColor(Qt.red)
        

    def setDebug(self, state):
        self._debug =  state
    def update(self, data):
        '''
            Update the model with new data gotten from e.g. RingUsage.systemUsage()
        '''
        if self._debug:
            print('Updating with: ')
            pprint.pp(data)
    
        #  Each list is a host.
        
        for host in data:
            host_item = self._find_or_create_host(host['host'])
            
            # Each host has rings as children
            
            for ring in host['rings']:
                ring_item = self._find_or_create_ring(host_item, ring['name'], False)
                self._update_ring(ring_item, ring)
                
                # Each ring may have consumers.
                
                consumers = ring['consumers']
                for consumer in consumers:
                    self._update_consumer(ring_item, consumer)
                
                # Each ring may have remote rings.
                # This all has identical structure to ring but
                # without a remot item on the ring.
                #
                remote_parent = ring_item.child(1, 0)    # The parent of all remotes:
                remotes = ring['proxies']
                for remote in remotes:
                    remote_item = self._find_or_create_ring(remote_parent, remote_item['name'])
                    self._update_ring(remote_item, remote)
                    
                    # Remotes also have consumers:
                    
                    consumers = remote['consumers']
                    for consumer in consumers:
                        self._update_consumer(remote_item, consumer)
            
            
    def _sibling(self, item, column):
        #  Get the sibling of an item frequently used.
        
        return item.parent().child(item.row(), column)   
    
    def _insert_empty_row(self, parent):
        # insert an empty row under the parent and return the item in col 0.
        global NUM_COLUMNS
        row = []
        for i in range(NUM_COLUMNS):
            row.append(QStandardItem(''))
        
        parent.appendRow(row)
        return row[0]
        
    def _find_or_create_host(self, name):
        # The hosts are the top level of the hierarchy, their names
        # are in col 0.  If one already exists with text() == name,
        # return it if not, make one and return it.
        
        row = 0
        while True:
            host_name = self._model.item(row, 0)
            if host_name is not None:
                if host_name.text() == name:
                    return host_name
                else:
                    row += 1
            else:
                host_name = QStandardItem(name)
                self._model.appendRow([host_name,])
                return host_name
    
    def _find_or_create_ring(self, host_item, ring_name, is_remote):
        # If a ring of the given ringname exists, return its col 0.
        # if not create:
        #    The ring items, 
        #    A child with text 'Consumers'
        #    A child with text 'Remote
        #
        # Returns the column 0 ring item.
        
        row =0
        while True:
            ring_item = host_item.child(row, 1)
            if ring_item is None:
                break                        # Need to create
            else:
                if ring_item.text() == ring_name:
                    return self._sibling(ring_item, 0)
                row += 1
        
        # Have to create a new ring item:
        
       
        result = self._insert_empty_row(host_item)
        result.appendRow([QStandardItem('Consumers'),])
        if not is_remote:
            result.appendRow([QStandardItem('Remote')],)
        
        return result
    
    def _update_ring(self, ring_item, ring):
        # Update the row identified by ring_item with the ring statistics. for it's ring 
        # which are in ring:
        global RING_PRODUCER
        global RING_PRODUCER_PID
        global RING_SIZE
        global RING_FREE
        
        self._sibling(ring_item, RING_NAME).setText(ring['name'])
        self._sibling(ring_item, RING_PRODUCER).setText(ring['producer_command'])
        self._sibling(ring_item, RING_PRODUCER_PID).setText(str(ring['producer_pid']))
        self._sibling(ring_item, RING_SIZE).setText(str(ring['size']))
        self._sibling(ring_item, RING_FREE).setText(str(ring['free']))
    
    
    def _find_or_create_consumer(self, ring_item, consumer_data):
        #  Given a ring item (QStandardItemModel), and the data for a consumer,
        #  either finds the row for the consumer with the same PID as consumer_data['consumer_pid']
        #  Or creates a new one.
        # 
        #   THe caller should not assume anything about which item on that row is returned.
        #
        
        #  The first child of ring_item is the parent for consumers:
        
        consumer_parent = ring_item.child(0, 0)
        crow = 0
        while True:
            consumer_item = consumer_parent.child(crow, 0)
            if consumer_item is not None:
                pid_item = self._sibling(consumer_item, CONSUMER_PID)
                if int(pid_item.text()) == consumer_data['consumer_pid']:
                    return consumer_item
                else:
                    crow += 1
            else:
                break
        # Didn't find, have to create.
        
        return self._insert_empty_row(consumer_parent)
        
    
    def _update_consumer(self, ring_item, consumer_data):
        global CONSUMER_CMD
        global CONSUMER_PID
        global CONSUMER_BACKLOG
        #
        #  Update the informationa about a consumer
        #
        consumer = self._find_or_create_consumer(ring_item, consumer_data)
        self._sibling(consumer, CONSUMER_CMD).setText(consumer_data['consumer_command'])
        self._sibling(consumer, CONSUMER_PID).setText(str(consumer_data['consumer_pid']))
        self._sibling(consumer, CONSUMER_BACKLOG).setText(str(consumer_data['backlog']))
        
if __name__ == '__main__':
    import RingUsage
    import RingView

    from PyQt5.QtWidgets import QApplication, QMainWindow
    

    app = QApplication(['ringview test'])
    mw = QMainWindow()
    tree = RingView.RingView()
    contents = RingModel(tree, 0.9)
    usage = RingUsage.systemUsage()
    contents.update(usage)
    mw.setCentralWidget(tree)
    mw.show()
    app.exec()


