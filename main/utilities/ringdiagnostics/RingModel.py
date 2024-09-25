'''
  See also RingView.py
     This module provides the code needed to build/maintain the model used by the ringview.
     See also, RingUsage.py - we manage the model based on the data that is produced
     by RingUsage.systemUsage()
     
     When we update the model from the data, we try very hard to just alter the data for
     existing ring items and not to delete/insert.  In this way, we can keep the parts of the
     tree that are open open for the user and just make it look like all the numbers are changing.
     
     -  Initial development cycle, we just maintain the tree.
     -  Second development cycle, we color bottlenecks in the data flow with a red background
     so that attention is drawn to them.   THe colorization goes up the hierarchy:
        *  If a ring, or proxy (remote) is dangerously backlogged, its ring or proxy folder is 
        highglighted as is its host folder. This draws attention to problems even in a completely 
        closed hierarchy.
    - 3'd development cycle - we prune the  model of items that are no longer 
    there.
'''

from PyQt5.QtGui import QStandardItem
from PyQt5.Qt import *

class RingModel:
    def __init__(self, view):
        self._model = view.model()
        
    def update(self, data):
        #
        #   Update the model with new data.
        #
        for host_rings in data:
            host = host_rings['host']
            ring_top = self._find_host(host)
            rings = host_rings['rings']
            for ring in rings:
                (ring_item, proxy_top) = self._update_ring(ring_top, ring, False)
                for consumer in ring['consumers']:
                    self._update_consumer(ring_item, consumer)
                for proxy in ring['proxies']:
                    (ring_item, junk) = self._update_ring(proxy_top, proxy, True)
                    for consumer in proxy['consumer']:
                        self._update_consumer(ring_item, consumer)
                                  
        
        
    def _find_host(self, host_name):
        #  Find a host in the model.
        #  If the host does not exist, make the framework for one
        # (hostname, rings folders)
        # Return the item create for the rings folder so stuff can be
        # edited/inserted there.
        # A host is an item in column 0 with a matching text and no parent:
        # Returns the 'Rings' item which is parent to all of its rings.
        #
        matches = self._model.findItems(host_name, Qt.MatchExactly, 0)
        for match in matches:
            if host_name == match.text():
                return match
        # We need to  insert the framework:
        
        host_item = QStandardItem(host_name)
        rings     = QStandardItem('Rings')
        self._model.appendRow(host_item)
        host_item.appendRow(rings)
        
        return rings
    
    def _find_proxies(self, ring_item):
        all_proxies = self._model.findItems('Remote', Qt.MatchExactly, 0)
        for proxy in  all_proxies:
            if proxy.parent() == ring_item:
                return proxy
        
        # No match is an error - this will likely make a run time error.
        return None
     
    def _find_ring(self, ring_top, ring):
        #
        #  Find the items that make up a ring item 
        #  given a parent.  A ring item is defined by 5 items and a list of those
        #  5 items are returned they have:
        #   0 - no text - hierarchy placeholder.
        #   1- The name of the ring.
        #   2- The producer command
        #   3- The producder PID
        #   4- The ring item size (considered invariant).
        #   5- The Free space in the ring.
        
        matches = self._model.findItems(ring['name'])
        for match in matches:
            # We need to get the tree place holder it should have ring_top as its parent:
            
            row = match.row()
            tree_item = self._model.item(row, 0)
            if tree_item.parent() == ring_top:
                # Matches..return the items:
                
                return (
                    tree_item, 
                    self._model.item(row, 1),
                    self._model.item(row, 2),
                    self._model.item(row, 3),
                    self._model.item(row, 4),
                    self._model.item(row, 5)
                )
        # Need to create a new row. To make it easier, we'll insert it after
        # the parent (above all the other rings).
        
        new_row = [
            QStandardItem(''),
            QStandardItem(ring['name']),
            QStandardItem(ring['producer_command']),
            QStandardItem(str(ring['producer_pid'])),
            QStandardItem(str(ring['size'])),
            QStandardItem(str(ring['free']))
        ]
        
        ring_top.appendRow(new_row)
        
        # The proxies folder:
        
        proxy_top = QStandardItem('Remote')
        ring_top.appendRow( proxy_top)
        
        
        return new_row
            
    
    
    def _update_ring(self, parent, ring, isProxy):
        # Given the 'Rings' folder and a ring definition,
        #  If the ring does not exist, create it and, if isProxy is false,
        #  create it's poxy child heading.
        #  Update the ring's  producer, and free space.
        # Returns the ring_item tree placeholder, and, if isProxy is False, the 
        # Proxy folder.
        #
        ring_item = self._find_ring(parent, ring)   # find or create - list of items.
        ring_item[1].setText(ring['name'])
        ring_item[2].setText(ring['producer_command'])
        ring_item[3].setText(str(ring['producer_pid']))
        ring_item[4].setText(str(ring['size']))     # In  case the ring was re-created.
        ring_item[5].setText(str(ring['free']))
        
        proxy_top = None
        if not isProxy:
            proxy_top = self._find_proxies(ring_item[0])
    
        
        return(ring_item[0], proxy_top)
    
    
    def _find_consumer(self, parent, consumer):
        # Locates a set of consumer items.  There are three consumer items.
        # in order:
        #   The consumer command.
        #   The consumer PID
        #   The consumer's backlog.
        # All three are returned.
        #  If the consumer cannot be found, one is created and inserted
        # into the parent and its items are returned.
        #  
        #  The consumer is identified by the item who's contents match
        #  the consumer['consumer_pid']
        #  and col0 has as a parent the parent passed in.
        #
        candidates = self._model.findItems(str(consumer['consumer_pid']))
        for match in candidates:
            row = match.row()
            if self._model.item(row, 0).parent() == parent:
                return (
                    self._model.item(row,6),
                    match,
                    self._model.item(row,8)
                )
        # There is no matching consumer so make one:
        
        new_consumer = [
            QStandardItem(''),   #hierarchy
            QStandardItem(''),           #ringname.
            QStandardItem(''),           #ring producer cmd
            QStandardItem(''),           #ring producer PID.
            QStandardItem(''),           #ring size.
            QStandardItem(''),           #ring free.
            QStandardItem(''),           #consumer cmd.
            QStandardItem(''),           # Consumer pid.
            QStandardItem(''),           # Backlog.
        ]
        parent.appendRow(new_consumer)
        return (
            new_consumer[6], new_consumer[7], new_consumer[8]
        )
    def _update_consumer(self, parent, consumer):
        consumer_items = self._find_consumer(parent, consumer)
        consumer_items[0].setText(consumer['consumer_command'])
        consumer_items[1].setText(str(consumer['consumer_pid']))
        consumer_items[2].setText(str(consumer['backlog']))
        
if __name__ == '__main__':
    import RingUsage
    import RingView
    
    from PyQt5.QtWidgets import QApplication, QMainWindow
    
    app = QApplication(['ringview test'])
    mw = QMainWindow()
    tree = RingView.RingView()
    contents = RingModel(tree)
    usage = RingUsage.systemUsage()
    contents.update(usage)
    mw.setCentralWidget(tree)
    mw.show()
    app.exec()