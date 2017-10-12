import ctypes
import win32con
import win32gui

from PyQt5.QtCore import Qt, QUrl, pyqtSignal, pyqtSlot
from PyQt5.QtWidgets import QWidget
from PyQt5.QtQuickWidgets import QQuickWidget

class Window(QQuickWidget):
  def __init__(self, engine, qmlPath, **kwargs):
    super().__init__(engine, None)
    self.qmlPath = qmlPath
    self.setClearColor(Qt.transparent)
    self.setAttribute(Qt.WA_NoSystemBackground)
    self.setAttribute(Qt.WA_TranslucentBackground)
    self.setAttribute(Qt.WA_TransparentForMouseEvents, 'transparent' in kwargs and kwargs['transparent'])
    if 'frameless' in kwargs and kwargs['frameless']:
      self.setWindowFlags(Qt.Tool | Qt.WindowStaysOnTopHint | Qt.FramelessWindowHint)
    else:
      self.setWindowFlags(Qt.Tool | Qt.WindowStaysOnTopHint)
    self.setSource(QUrl(qmlPath))

    if 'parent' in kwargs:
      self.parentWindow = kwargs['parent']
      self.offset = kwargs['offset']
      self.parentWindow.quickWindow().xChanged.connect(self.repositionToParent)
      self.parentWindow.quickWindow().yChanged.connect(self.repositionToParent)
      self.quickWindow().visibleChanged.connect(self.repositionToParent)
    elif 'initialPos' in kwargs:
      self.move(*kwargs['initialPos'])

    self.Global = engine.rootObjects()[0].property('o')

  @pyqtSlot()
  def repositionToParent(self):
    self.move(self.parentWindow.quickWindow().x() + self.offset[0], self.parentWindow.quickWindow().y() + self.offset[1])

  def showEvent(self, event):
    super().showEvent(event)
    hwnd = int(self.winId())
    ctypes.windll.user32.SetWindowLongW(hwnd, win32con.GWL_EXSTYLE, ctypes.windll.user32.GetWindowLongW(hwnd, win32con.GWL_EXSTYLE) | win32con.WS_EX_NOACTIVATE | win32con.WS_EX_APPWINDOW)

class TransparentWindow(Window):
  def __init__(self, engine, qmlPath, **kwargs):
    super().__init__(engine, qmlPath, transparent=True, frameless=True, **kwargs)

class OpaqueWindow(Window):
  def __init__(self, engine, qmlPath, **kwargs):
    super().__init__(engine, qmlPath, transparent=False, frameless=True, **kwargs)