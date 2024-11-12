"""
This module provides a class to describe and control event loggers.
The main class is LoggerList which lists the loggers and provides
controls.

"""

from PyQt5.QtWidgets import (QTableWidget, QTableWidgetItem, QCheckBox, QPushButton, QWidget, QVBoxLayout)
from PyQt5.QtCore import pyqtSignal, Qt


class LoggerTable(QTableWidget):
    """This is a table of loggers.  The table has the following columns
    (A row descsribes a logger):
    
    Source - URI of the data source (ringbuffer).
    Destination - Path to the destination diretory
    C   - X'd if the logger is critical
    P   - X'd if the logger is partal.
    Enable - A check button that  checked if the logger is enabled.
    
    Important method:
    
    update - Takes the output from nscldaq.manager_client.Logger.list()
            and refreshes the table.  Note that, unlike programs, 
            we don't clear items.  We do an update in place. 

    Base Class:
        QTableWidget
    Signals:
        toggle(list) - Whena  checkbox is toggled.  The list contains a boolean and string:
             - bool - the new state of the checkbox - True  means checked. False not.
             - str  - Destination of the logger, e.g. if the user is enabling a logger that
             logs to /user/fox/stagearea/full, toggle will pass [True, "/user/fox/stagearea/full"]
    """
    toggle = pyqtSignal(list)
    def __init__(self, *args):
        """Construction.
                The base class is constructed and we set up the column labels.
                Content will be loaded/modified via calls to update().
           Arguments:
           args - is passed to the base class constructor.
           
           
        """
        super().__init__(*args)
        self.setColumnCount(5)
        self.setHorizontalHeaderLabels([
            "Source", "Destination", "C", "P", "Enable"
        ]
            
        )
    def update(self, listing):
        """Update the table in place.  The algorithm is pretty simple:
           - For each row in the table
             *  Find the listing by destination - if not found kill that row.
             *  Update the row except for the destination value.
             *  Remove that item from the listing.
             
           - For each list item, add a new table row.

        Args:
            listing (_type_): _description_
        """
        delete_rows = []
        delete_defs = []
        for row in range(self.rowCount()):
            dest = self.item(row, 1).text()
            (index, definition) = self._findDest(dest, listing) 
            if index is None:
                delete_rows.append(row)
            else:
                self._updateRow(row, definition)
                delete_defs.append(index)
        
        # Delete the rows that don't have definitions
        # and the definitions that we updated.
        
        delete_rows.sort(reverse=True)
        delete_defs.sort(reverse=True)
        
        for row in delete_rows:
            self.removeRow(row)
        
        for index in delete_defs:
            listing.pop(index)
            
        # Add rows for any remaining definitions:
        
        for definition in listing:
            self._addRow(definition)
                
    # Private utility methods    
    
    def _findDest(self, dest, listing):
        # Find a destination in a listing of loggers.
        # Returns (index, definition) if found else
        # (None, None) if not found.
        
        for index, definition in enumerate(listing):
            if definition['destination'] == listing:
                return (index, definition)
        return (None, None)      # stub - not found.)
    
    def _updateRow(self, row, definition):
        #  Update the contents of a row:
        #  The source might change, the criticality of the logger may change
        #  as well as the partialness, the state of the enable widget too
        #  May change.
        #  Since the row was found by destination, that's assumed constant.
        #  With the exception of the enable widget, we only modify a column if it's content
        #  Value should change.
        
        # Data source:
        
        sourceItem = self.item(row, 0)
        if sourceItem.text() != definition['ring']:
            sourceItem.setText(definition['ring'])
        
        # Critical flag:
        
        critical_text = 'X' if definition['critial'] else '' 
        criticalItem = self.item(row, 2)
        if critical_text != criticalItem.text():
            criticalItem.setText(critical_text)
           
        # Partial flag:
        
        partial_text = 'X' if definition['partial'] else '' 
        partialItem = self.item(row, 3)
        if partial_text != partialItem.text():
            partialItem.setText(partial_text)
            
        # Now the enable widget:
        
        enabled = definition['enabled'] == 1     # convert to a bool.
        self.cellWidget(row, 4).setCheckState(Qt.Checked if enabled else Qt.Unchecked)
        
        
    def _addRow(self, definition):
        new_row = self.rowCount()
        self.insertRow(new_row)
        
        # Now load up the row....and hook the checkbox to our
        # signal maker.
        
        self.setItem(new_row, 0, QTableWidgetItem(definition['ring']))
        self.setItem(new_row, 1, QTableWidgetItem(definition['destination']))
        self.setItem(new_row, 2, QTableWidgetItem('X' if definition['critical'] else ''))
        self.setItem(new_row, 3, QTableWidgetItem('X' if definition['enabled'] else ''))
        check = QCheckBox(self)
        self.setCellWidget(new_row, 4, check)
        enabled = definition['enabled'] == 1 
        check.setCheckState(Qt.Checked if enabled else Qt.Unchecked)
        
        check.stateChanged.connect(self._checkToggled)
        
    def _checkToggled(self, state):
        pass
        
        
        

# Test code:

if __name__ == "__main__":
    from PyQt5.QtWidgets import (QApplication, QMainWindow)
    import sys
    app = QApplication(sys.argv)
    main = QMainWindow()
    
    widget = LoggerTable()
    main.setCentralWidget(widget)
    
    widget.update([ 
        {'ring': 'tcp://localhost/ron', 'destination': '/home/ron/events/complete', 'critical': 1, 'partial': 0, 'enabled': 0},
        {'ring': 'tcp://localhost/fox', 'destination': '/home/ron/events/complete', 'critical': 0, 'partial': 1, 'enabled': 1}
    ])
    
    main.show()
    app.exec()
        