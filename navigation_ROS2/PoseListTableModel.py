import datetime as dt
import copy
from python_qt_binding.QtCore import QAbstractTableModel, Qt
from operator import itemgetter


class PoseListTableModel(QAbstractTableModel):
    def __init__(self, parent=None):
        super(PoseListTableModel, self).__init__(parent)
        self.contents = []
        self.header_labels = ['Name', 'X', 'Y', 'Heading Angle', 'Active']

    def headerData(self, section, orientation, role=Qt.DisplayRole):
        if role == Qt.DisplayRole and orientation == Qt.Horizontal:
            return self.header_labels[section]
        return QAbstractTableModel.headerData(self, section, orientation, role)


    def rowCount(self, parent):
        return len(self.contents)

    def columnCount(self, parent):
        return len(self.header_labels)

    def data(self, index, role):
        col = index.column()
        if not index.isValid():
            return None
        #elif role == Qt.TextAlignmentRole:
        #    return Qt.AlignLeft
        elif role == Qt.DisplayRole:
            return self.contents[index.row()][col]
        elif role == Qt.EditRole:
            if col != 4:
                return self.contents[index.row()][col]
        elif role == Qt.CheckStateRole:
            if col == 4:
                return Qt.Checked if self.contents[index.row()][col] else Qt.Unchecked
            else:
                return None
        else:
            return None

    def clear(self):
        self.contents = []
        self.layoutChanged.emit()

    def insert_row(self, idx, content):
        self.contents.insert(idx, content)
        self.layoutChanged.emit()

    def remove_row(self, idx):
        del self.contents[idx]
        self.layoutChanged.emit()

    def flags(self, index):
        if not index.isValid():
            return Qt.NoItemFlags
        elif index.column() == 4:
            return Qt.ItemIsUserCheckable | Qt.ItemIsSelectable | Qt.ItemIsEnabled | Qt.ItemIsEditable
        else:
            return Qt.ItemIsSelectable | Qt.ItemIsEnabled | Qt.ItemIsEditable

    def move_up(self, index):
        if index != 0:
            self.contents[index], self.contents[index-1] = self.contents[index-1], self.contents[index]
            self.layoutChanged.emit()
            return True
        else:
            return False

    def move_down(self, index):
        if index != len(self.contents) - 1:
            self.contents[index], self.contents[index+1] = self.contents[index+1], self.contents[index]
            self.layoutChanged.emit()
            return True
        else:
            return False

    def sort(self, column, order=Qt.AscendingOrder):
        if order == Qt.AscendingOrder:
            self.contents.sort(key=itemgetter(column), reverse=False)
        elif order == Qt.DescendingOrder:
            self.contents.sort(key=itemgetter(column), reverse=True)
        self.layoutChanged.emit()

    def setData(self, index, value, role=None):
        col = index.column()
        row = index.row()
        if role == Qt.CheckStateRole:
            if col == 4:
                self.contents[row][col] = True if value == Qt.Checked else False
                self.dataChanged.emit(index, index)
        elif role == Qt.EditRole:
            self.contents[row][col] = value
            self.dataChanged.emit(index, index)
            return True
        return False

    def set_poses_from_csv_reader(self, csv_reader):
        self.contents = []
        for row in csv_reader:
            name = row[0]
            x = float(row[1])
            y = float(row[2])
            angle = float(row[3])
            act = False
            if 'True' in row[4] or "TRUE" in row[4]:
               act = True
            self.contents.append([name, x, y, angle, act])
        self.layoutChanged.emit()


    def add_row(self, name, x, y, angle, row = -1):
        content = [name, x, y, angle, True]
        if row == -1:
            self.contents.append(content)
        else:
            self.contents.insert(row, content)
        self.layoutChanged.emit()

    def clone(self, row_index):
        new_row = copy.deepcopy(self.contents[row_index])
        self.contents.insert(row_index, new_row)
        self.layoutChanged.emit()

    def auto_rename(self):
        for idx in range(len(self.contents)):
            self.contents[idx][0] = str(idx)
        self.layoutChanged.emit()
