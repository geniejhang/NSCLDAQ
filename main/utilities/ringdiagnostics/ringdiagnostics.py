#!/usr/bin/env python3
from nscldaq import RingUsage
from nscldaq import  RingView
from nscldaq import RingModel

from PyQt5.QtWidgets import QApplication, QMainWindow

app = QApplication(['ringview test'])
mw = QMainWindow()
tree = RingView.RingView()
contents = RingModel.RingModel(tree)
usage = RingUsage.systemUsage()
contents.update(usage)
mw.setCentralWidget(tree)
mw.show()
app.exec()