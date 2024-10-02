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
import pprint
from PyQt5.QtGui import QStandardItem, QBrush
from PyQt5.Qt import *


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
        for host_rings in data:
            host = host_rings['host']
            ring_top = self._find_host(host)
            rings = host_rings['rings']
            for ring in rings:
                (ring_item, proxy_top) = self._update_ring(ring_top, ring, False)
                if self._debug:
                    print("After initial update:", ring_item, proxy_top)
                for consumer in ring['consumers']:
                    self._update_consumer(ring_item, consumer)
                for proxy in ring['proxies']:
                    if self._debug:
                        print('Updating proxies', proxy_top.text(), proxy, True)
                    (ring_item, junk) = self._update_ring(proxy_top, proxy, True)
                    for consumer in proxy['consumers']:
                        self._update_consumer(ring_item, consumer)
        
        self._prune(data)      # Get rid of vanished stuff.
        self._colorize()                          
        
    
    # Update data.
    
    def _get_children(self, item):
        # Return the, possibly empty list of children the item has.
        
        result = []
        row = 0
        if item.hasChildren():
            while True:           
                child = item.child(row, 0)
                if child is None:
                    break
                result.append(child)
                row = row + 1
        return result    
        
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
        host_item.setEditable(False)
        self._model.appendRow(host_item)
        
        return host_item
    
    def _find_proxies(self, ring_item):
        proxy = ring_item.parent().child(1, 0)
        if self._debug:
            print("proxy parent: ", proxy)
            print(proxy.text())
        
        return proxy
     
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
        
        
        children = self._get_children(ring_top)
        for row_item in children:
            row = row_item.row()
            ring_name_item = ring_top.child(row, 1)
            # Proxy has no row 1.
            if ring_name_item is not None and ring_name_item.text() == ring['name']:

                return (
                    row_item, ring_name_item,
                    ring_top.child(row, 2),
                    ring_top.child(row, 3),
                    ring_top.child(row, 4),
                    ring_top.child(row, 5)
                )
        
        # Have to make a new ring item:
        
        new_row = [
            QStandardItem(''),
            QStandardItem(ring['name']),
            QStandardItem(ring['producer_command']),
            QStandardItem(str(ring['producer_pid'])),
            QStandardItem(str(ring['size'])),
            QStandardItem(str(ring['free']))
        ]
        for item in new_row:
            item.setEditable(False)
        
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
        if self._debug and isProxy:
            print("update rings for proxies", parent, ring,)
            
        ring_item = self._find_ring(parent, ring)   # find or create - list of items.
        if isProxy:
            ring_item[1].setText(f'{ring["name"]}@{ring["proxyhost"]}')
        else:            
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
        candidate_rows = self._get_children(parent)
        for crow in candidate_rows:
            row_num = crow.row()
            pid_item = parent.child(row_num, 7)
            if str(consumer['consumer_pid']) == pid_item.text():
                return (
                    parent.child(row_num, 6),
                    pid_item,
                    parent.child(row_num,8)
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
        for c in new_consumer :
            c.setEditable(False)
        parent.appendRow(new_consumer)
        return (
            new_consumer[6], new_consumer[7], new_consumer[8]
        )
    def _update_consumer(self, parent, consumer):
        consumer_items = self._find_consumer(parent, consumer)
        consumer_items[0].setText(consumer['consumer_command'])
        consumer_items[1].setText(str(consumer['consumer_pid']))
        consumer_items[2].setText(str(consumer['backlog']))
    
    #  Methods that have to do with pruning the tree:
    
    
    def _getModelHosts(self):
        # Return all the host name items:
        
        row = 1   # row 0 is headers.
        result = []
        while True:
            host_item = self._model.item(row, 0)
            if host_item is not None:
                result.append(host_item)
                row += 1
            else:
                break
        
        return result
   
    def _enumerate_ui_rings(self, host):
        # Return [(name, row, item), ...] for the ring items
        # under the host.
        
        result = []
        ring_row = 0
        while True:
            proxy_or_empty = host.child(ring_row, 0)
            if proxy_or_empty is not None:
                if proxy_or_empty.text() == '':
                    # Other wise it's the parent of the proxies.
                    
                    ring_item = host.child(ring_row, 1)
                    result.append((ring_item.text(), ring_row, ring_item))
                ring_row += 1
            else:
                break
        return result
        
    def _enumerate_host_rings(self, host, data):
        # Return a list oif the names of rings in a host.
        #  host -name of the host.
        #  data -data passed in  to update.
        #  assumption: the host only appears once in the data.
        
        result = []
        for host_dict in data:
            if host_dict['host'] == host:
                host_rings = host_dict['rings']
                host_ring_names = [x['name'] for x in host_rings]    
                result += host_ring_names
        return result
    
    def _enumerate_data_consumers(self, host_name, ring_name, data):
        # Returns the pids of the consumers for 
        # the named host, ring.
        
        result = []
        for host in data:
            if host['host'] == host_name:
                for ring in host['rings']:
                    if ring['name'] == ring_name:
                        result += [x['consumer_pid'] for x in ring['consumers']]
        return result
    
    def _enumerate_ui_consumers(self, ring_item):
        # Returns a list of (consumer_pid, child_row) consumer descriptions.
        # for ring_item in the model.
        
        result = []
        row = 0
        parent = ring_item.parent().child(ring_item.row(), 0)
        
        while True:
            pid_item = parent.child(row, 7)
            if pid_item is not None:
                result.append((int(pid_item.text()), row))
                row += 1
            else:
                break
        
        return result
    
    def _prune_consumers(self, host, ring_item, data):
        #  For a ring buffer item, prune the consumers that 
        # are gone.  We use the PID to identify consumers.
        
        host_name = host.text()
        ring_name = ring_item.text()
        ui_consumers = self._enumerate_ui_consumers(ring_item)
        data_consumers = self._enumerate_data_consumers(host_name, ring_name, data)
        
        delete_rows = [x[1] for x in ui_consumers if x[0] not in data_consumers]
        delete_rows.sort(reverse=True)
        
        parent = ring_item.parent().child(ring_item.row(), 0)
        for row in delete_rows:
            parent.takeRow(row)
        
    def _prune_rings(self, host, data):
        # Prune rings that are no longer present in a host.
        # This includes the remote rings.
        #  host - the host item parenting the rings in the GUI.
        #  data - the data passed into update.
        ring_items = self._enumerate_ui_rings(host)  # [(name, row),...]
        ring_names = self._enumerate_host_rings(host.text(), data)
        
        ui_names = [name[0] for name in ring_items]
        
        # Accumulate the child rows to kill:
        
        kill_rows = [x[1]  for x in ring_items if x[0] not in ring_names]
        kill_rows.sort(reverse=True)

        for row in kill_rows:
            host.takeRow(row+1)    # The remote item.
            host.takeRow(row)      # The ring item
    def _prune_hosts(self, data):
        #
        # Kill hosts from the model that are no longer present in 
        # the data
        
        data_hosts = [x['host'] for x in data] # Host names
        displayed_hosts = []  # [(name, row)]
        host_row = 1
        while True:
            host = self._model.item(host_row, 0)
            if host is None:
                break
            displayed_hosts.append((host.text(), host_row))
            host_row = host_row+ 1
        
        # Make a list of rows that have hosts in displayed_hosts not in
        # data_hosts
        
        kill_hosts = [x[1] for x in displayed_hosts if x[0] not in data_hosts]
        
        # Sort descending as that's how we have to remove them:
        
        kill_hosts.sort(reverse=True)
        
        # KIll those lines from the model.
        
        for r in kill_hosts:
            self._model.takeRow(r)
            
    
    def _get_ui_proxy_rings(self,  proxy_top):
        # Enumemrate the ring items that are proxies for the given
        # host and 'Remote' item
        #
        #  proxy_top 'Remote' item which is the parent of all proxy rings for a host.
        #
        #  Returns a list of ring items
        result = []
        row = 0
        while True:
            child = proxy_top.child(row, 1)
            if child is not None:
                row = row + 1
                result.append(child)
            else:
                break
                
        return result
    
    def  _get_data_proxy_rings(self, hostname, ring_name, data):
        
        #  Return a list of the proxy ring names associated with
        #  A ring in a host.
        
        result = []
        for h in data:
            if h['host'] == hostname:
                for r in h['rings']:
                    if r['name'] == ring_name:
                        result.append([x['name'] for x in r['proxies']])
                        break
        return result
    
    def _prune_proxy_rings(self, hostname, ring_name, proxy_top, data):
        # Remove rings (and the consumers) of any proxies
        # that are no longer in the data.
        # hostname - Name of the host.
        # proxy_top - Parent of all proxies.
        # data      - The data passed to update
        
        proxy_ring_items = self._get_ui_proxy_rings(proxy_top)
        data_proxy_rings = self._get_data_proxy_rings( hostname, ring_name, data)
        
        delete_rows = [x.row() for x in proxy_ring_items if x.text() not in data_proxy_rings]
        delete_rows.sort(reverse=True)
        
        for row in delete_rows:
            proxy_top.takeRow(row)
            
    def _get_data_proxy_consumers(self, host_name, ring_name, data):
        # Returns a list of PIDS of consumers of proxies for a specific
        # proxy ring name. 
        result =[]
        for h in data:
            if h['host']  == host_name:
                for pr in h['host']['rings']['proxies']:
                    if pr['name'] == ring_name:
                        result.append([x['consumer_pid'] for x in pr['consumers']])
        return result
    
    def _get_ui_proxy_consumers(self, parent):
        #  Returns [(pid, row)] of consumers for a given ui proxy ring.
        
        result = []
        row  = 0
        while True:
            pid_item = parent.child(row, 7)
            if pid_item is not None:
                result.append((int(pid_item(text)), row))
                row = row + 1
            else:
                break
        
        return result
        
            
    def _prune_proxy_consumers(self, host_name, proxy, proxy_ring, data):
        #  For a proxy ring, prunes no longer existing consumers.
        #  host_name - name of the host being scrutinized.
        #  proxy     - Model itm that is the parent of all conusmers.
        #  proxy_ring- proxy ring item...col 0 of that is the parent.
        #  data     - Data passed to update.
        consumer_parent = proxy_ring.parent().item(proxy_ring.row(), 0)
        data_consumers = self._get_data_proxy_consumers(
            host_name, proxy_ring.text(), data
        )
        ui_consumers = self._get_ui_proxy_consumers(consumer_parent)

        delete_rows = [x[1] for x in ui_consumers if ui_consumers[0] not in data_consumers]
        delete_rows.sort(reverse=True)
        
        for row  in delete_rows:
            consumer_parent.takeRow(row
                                    )
    def _prune_proxies(self, host_item, ring_item, data):
        #
        # Rings have zero or more proxies.
        # This method prunes dead proxy rings and dead
        # proxy consumers.
        #
        ring_row = ring_item.row()
        proxy = ring_item.parent().child(ring_row+1, 0) # Proxy follows ring.
        host_name = host_item.text()
        ring_name = ring_item.text()
        
        self._prune_proxy_rings(host_name, ring_name, proxy, data) # Kill dead proxy rings.
        proxy_ui_rings = self._get_ui_proxy_rings(proxy)
        for proxy_ring in proxy_ui_rings:
            self._prune_proxy_consumers(
                host_name, proxy, proxy_ring, data
        )
        
        pass    
    def _prune(self, data):
        # Prune items from the tree that no longer exist.
        
        self._prune_hosts(data)
        
        model_hosts = self._getModelHosts()
        for host in model_hosts:
            self._prune_rings(host, data)
            ring_items = [x[2] for x in self._enumerate_ui_rings(host)]
            for ring in ring_items:
                self._prune_consumers(host, ring, data)
                self._prune_proxies(host, ring, data)
            
        
    
    #   Alarm handling.
    
    #  colorize the consumers of a ring.
    #  note that colorizing a consumer implies, if its in error, 
    #  colorizing its parents as well.
    #   ring - the col0 of the ring.
    #   name - the name item of the ring.
    #
    #   Get the size of the ring and compute what backlogged means from
    #   self_alarm.
    # 
    #   for each consumer, if the consumer backlog is bigger than the
    #   alarm level,
    #     * Color the command red.
    #     * color the name of the ring red.
    #     * Color the col 0 parents red \
    #        That might be a 'host only or a "Remote' a ring and host.
    #
    def _colorize_ring(self, ring_item, name_item):
        # Figure out the threshold
        
        size_item = ring_item.parent().child(ring_item.row(), 4)
        size = int(size_item.text())
        threshold = (size * self._alarm)/100
        
        # Page through the consumer backlogs:
        
        consumer_row = 0
        while True:
            consumer_backlog = ring_item.child(consumer_row, 8)
            consumer_command = ring_item.child(consumer_row, 6)
            if consumer_backlog is None:
                return
            consumer_command.setForeground(self._okbrush)
            if int(consumer_backlog.text()) > threshold:
                if int(consumer_backlog.text()) > threshold:
                    consumer_command.setForeground(self._errbrush)
                    name_item.setForeground(self._errbrush)
                    parent = name_item.parent()
                    while parent is not None:
                        parent.setForeground(self._errbrush)
                        parent = parent.parent()
            consumer_row = consumer_row+1
                
                
    
    def _colorize(self):
        #  Look down into the hierarchy and colorize the things that should be colorized.
        #  We really only need to look at the consumers in the ring and proxies.
        # our approach is fairly simple
        #  For each host, 
        #    Color it normal
        #    for each ring  in the host:
        #      Color it normal
        #      Color the proxy root normal.
        #      for each consumer in the ring
        #         If the consumer is backlogged:
        #           color it red
        #           color all parents read.
        #         Else color it normal
        #      for each proxy ring of the ring:
        #         color it normal.
        #         for each remote consumer:
        #            if the consumer is backlogged, 
        #               Color it red
        #               color all parents red.
        #
        
        # For all hosts:
        
        view_row = 1    # Zero is the headers.
        while True:
            host = self._model.item(view_row, 0)
            if host is not None:

                host.setForeground(self._okbrush)
                #
                #  Children of a host are
                #  Either a ringbuffer or a
                #  Proxy.
                #  RingBuffers have no text in col 0.
                #
                #  Proxies have the text "Remote" in col0.
                #
                host_row = 0
                while True:
                    ring_or_remote = host.child(host_row, 0)
                    if self._debug:
                        print("Ring or remote text: ", ring_or_remote.text())
                    if ring_or_remote is None:
                        break    
                    elif ring_or_remote.text() == '':
                        #ring:
                        name_item = host.child(host_row, 1)
                        name_item.setForeground(self._okbrush)
                        self._colorize_ring(ring_or_remote, name_item)
                    elif ring_or_remote.text() == 'Remote':
                        ring_or_remote.setForeground(self._okbrush)
                        rings = self._get_children(ring_or_remote)
                        if self._debug:
                            print('RIngs in colorize:', rings)
                        for ring in rings:
                            if self._debug:
                                print("Processing ring: ", ring, ring.text())
                                print("Parent and row are: is:", ring.parent(), ring.row(), ring.column())
                                print("Parent has children? ", ring.parent().hasChildren()) 
                                print("Parent's txt is", ring.parent().text())                                   
                            name_item = ring_or_remote.child(ring.parent().child(ring.row(), 1))
                            if self._debug:
                                print("name_item", name_item)
                            name_item.setForeground(self._okbrush)
                            self._colorize_ring(ring, name_item)
                    else:      
                        # don't know what this is, skip
                        pass
                    host_row = host_row+1
                view_row = view_row+1
            else:
                break
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
    
    
    