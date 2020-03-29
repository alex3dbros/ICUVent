import sys
from os.path import join, dirname, abspath
import serial
import serial.tools.list_ports as port_list
from qtpy import uic
from qtpy.QtCore import Slot, QTimer, QThread, Signal, QObject, Qt
from qtpy.QtWidgets import QApplication, QMainWindow, QMessageBox, QAction, QDialog, QTableWidgetItem
from pyqtgraph import PlotWidget
from collections import deque

import numpy as np
import random
import qtmodern.styles
import qtmodern.windows
import time
import json


_UI = join(dirname(abspath(__file__)), 'VentUI.ui')

pressure_demultiplier = 500
tidalVolumeSettings = ["300", "400", "500", "600"]
respiratory_rate_settings = ["10", "12", "14", "16", "18", "20", "22", "24", "26", "28", "30"]
inhale_exhale_ratios = ["1:1", "1:2", "1:3"]

class WorkerThread(QObject):
    signal = Signal(dict)

    def __init__(self, s):
        self.s = s

        super().__init__()

    @Slot()
    def run(self):
        while 1:
            res = self.s.readline().decode('ascii')

            try:
                jMessage = json.loads(res)
                self.signal.emit(jMessage)
            except:
                jMessage = {}

            # doFunc(self.signal, jMessage)

class WorkerThread2(QObject):
    signal = Signal(dict)

    def __init__(self, s):
        self.s = s

        super().__init__()

    @Slot()
    def run(self):
        # for x in range(10000):
        #     if len(self.dat) > self.maxLen:
        #         self.dat.popleft()  # remove oldest
        #     self.dat.append(x)
        #     self.curve1.setData(self.dat)

        while 1:
            res = self.s.readline().decode('ascii')

            try:
                jMessage = json.loads(res)
                self.signal.emit(jMessage)
            except:
                jMessage = {}



class MainWindow(QMainWindow):
    def __init__(self):
        QMainWindow.__init__(self)

        self.widget = uic.loadUi(_UI, self)
        window_title = "ICU Ventilator Monitoring"
        self.widget.setWindowTitle(window_title)
        self.current_battery = ""
        self.total_capacity = 0
        self.total_cells = 0
        self.current_selected_item_id = 0

        self.dat = deque()
        self.dat2 = deque()

        self.airVolGraph.showGrid(x=True, y=True, alpha=None)
        self.pressureGraph.showGrid(x=True, y=True, alpha=None)

        # Setting defaults on vent settings
        self.widget.tidalVolume.addItems(tidalVolumeSettings)
        self.widget.respirationRate.addItems(respiratory_rate_settings)
        self.widget.inhaleExhaleRatio.addItems(inhale_exhale_ratios)

        self.curve1 = self.airVolGraph.plot()
        self.curve2 = self.pressureGraph.plot()
        self.s = ""
        self.ports = list(port_list.comports())


        for p in self.ports:
            self.widget.portsList.addItem(p[0])
            self.widget.monitoringPort.addItem(p[0])

    def drawGraph(self):
        x = np.random.normal(size=1000)
        y = np.random.normal(size=(3, 1000))
        for i in range(3):
            self.airVolGraph.plot(x, y[i], pen=(i, 3))

    @Slot()
    def on_connect_clicked(self):
        self.s = serial.Serial(self.portsList.currentText(), baudrate=9600, timeout=1)

        self.s2 = serial.Serial(self.monitoringPort.currentText(), baudrate=115200, timeout=100)

        res = self.s.readline().decode('ascii')
        try:
            jMessage = json.loads(res)
            self.set_defaults_from_machine(jMessage)
        except:
            jMessage = {}

        self.worker = WorkerThread(self.s)
        self.workerThread = QThread()
        self.workerThread.started.connect(self.worker.run)
        self.worker.signal.connect(self.write_info)
        self.worker.moveToThread(self.workerThread)
        self.workerThread.start()


        self.worker2 = WorkerThread2(self.s2)
        self.workerThread2 = QThread()
        self.workerThread2.started.connect(self.worker2.run)
        self.worker2.signal.connect(self.write_patient_monitoring)
        self.worker2.moveToThread(self.workerThread2)
        self.workerThread2.start()





    @Slot()
    def on_disconnect_clicked(self):
        self.workerThread.terminate()

    @Slot()
    def on_draw_clicked(self):
        self.update(203)

    @Slot()
    def on_scanPorts_clicked(self):
        self.ports = list(port_list.comports())
        self.widget.portsList.clear()
        self.widget.monitoringPort.clear()
        print(len(self.ports))
        for p in self.ports:


            self.widget.portsList.addItem(p[0])
            self.widget.monitoringPort.addItem(p[0])

    @Slot()
    def on_ventSetButton_clicked(self):

        airVol = self.widget.tidalVolume.currentText()
        breaths_per_min = self.widget.respirationRate.currentText()
        ie_ratio = self.widget.inhaleExhaleRatio.currentText().split(":")[1]

        command = "{\"airVol\": %i, \"breathsPerMinute\": %i, \"ieRate\": %i}" % (int(airVol), int(breaths_per_min), int(ie_ratio))

        print(command)

        self.s.write(bytes(command.encode()))
        print("Trying to write")


    def write_info(self, data_stream):

        self.maxLen = 1000  # max number of data points to show on graph

        # print(data_stream)


        if len(self.dat) > self.maxLen:
            self.dat.popleft()  # remove oldest
        self.dat.append(data_stream["pressure"] / pressure_demultiplier)
        self.curve1.setData(self.dat)

        if len(self.dat2) > self.maxLen:
            self.dat2.popleft()  # remove oldest
        self.dat2.append(data_stream["airflow"])
        self.curve2.setData(self.dat2)

        # print("Pressure: %s , Flow: %s" % (data_stream["pressure"], data_stream["airflow"]))


    def write_patient_monitoring(self, jMessage):

        if jMessage["heartrate"] > 50:
            self.heartrate.display(jMessage["heartrate"])
            self.o2value.display(jMessage["spo2"])
        else:
            self.heartrate.display(0)
            self.o2value.display(0)

    def set_defaults_from_machine(self, jMessage):

        tidalvol = str(jMessage["tidalvol"])
        rr = str(jMessage["resprate"])
        ieratio = "1:" + str(jMessage["ieratio"])

        tvindex = self.tidalVolume.findText(tidalvol, Qt.MatchFixedString)
        if tvindex != -1 and self.tidalVolume.currentText() != tidalvol:
            self.tidalVolume.setCurrentIndex(tvindex)

        rrindex = self.respirationRate.findText(rr, Qt.MatchFixedString)
        if rrindex != -1 and self.respirationRate.currentText() != rr :
            self.respirationRate.setCurrentIndex(rrindex)

        ieindex = self.inhaleExhaleRatio.findText(ieratio, Qt.MatchFixedString)
        if ieindex != -1 and self.inhaleExhaleRatio.currentText() != ieratio :
            self.inhaleExhaleRatio.setCurrentIndex(ieindex)


def catch_exceptions(t, val, tb):
    QMessageBox.critical(None,
                                   "An exception was raised",
                                   "Exception type: {}".format(t))
    old_hook(t, val, tb)

old_hook = sys.excepthook
sys.excepthook = catch_exceptions

if __name__ == '__main__':
    app = QApplication(sys.argv)

    qtmodern.styles.dark(app)

    mw_class_instance = MainWindow()
    mw = qtmodern.windows.ModernWindow(mw_class_instance)
    mw.show()
    sys.exit(app.exec_())

