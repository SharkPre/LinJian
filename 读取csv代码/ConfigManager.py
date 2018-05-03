# -*- coding: utf-8 -*-
import os
import csv
import KBEngine
from KBEDebug import *

class CsvFile:
    def __init__(self, _filename):
        """
        list结构(用以提供上层应用快速遍历)
        dict结构(用以提供上层应用快速查询)
        """
        self.m_list_datas = []
        self.m_dict_datas = {}
        self.m_titles = []
        self.m_rules = {}

        _listDatas = csv.reader(open(KBEngine.getResFullPath(_filename), encoding='utf-8'))
        _index = 0
        for row_data in _listDatas:
            if _index == 0:
                self.m_titles = row_data
            elif _index > 1:
                #构建list和dict结构
                _data = {}
                for i in range(0, len(row_data)):
                    if _index == 2:
                        self.m_rules[self.m_titles[i]] = row_data[i]
                    else:
                        values = self.getValue(self.m_titles[i], row_data[i])
                        if values[1]:
                            _data[self.m_titles[i]] = values[0]
                            self.insert(self.m_titles[i], row_data[i], _data)
                        else:
                            #TODO:直接让进程挂掉
                            ERROR_MSG('load csv(%s) key(%s):value(%s) Error(%s)!!!' % (_filename, self.m_titles[i], row_data[i], values[2]))
                            return

                if _index > 2:
                    self.m_list_datas.append(_data)
            _index += 1

    def unique(self, _key, _value):
        ok = True
        value = str(_value)
        if _key in self.m_dict_datas.keys():
            value_dict = self.m_dict_datas[_key]
            if value in value_dict.keys():
                ok = False

        return ok

    def getValue(self, _key, _value):
        rules = self.m_rules[_key]
        rule_list = rules.split("&")
        ok = True
        error = "ok"
        for rule in rule_list:
            if rule == "float":
                _value = float(_value)
            elif rule == "int":
                _value = int(_value)
            elif rule == "bool":
                if _value.upper() == "TRUE":
                    _value = True
                elif _value.upper() == "FALSE":
                    _value = False
                else:
                    ok = False
                    error = "[bool] rule error, just allow false or true"
            elif rule == "string":
                ok = True
            elif rule == "unique":
                ok = self.unique(_key, _value)
                if not ok:
                    error = "[unique] rule error"
            else:
                ok = False
                error = "not allow rule:" + rule

        return (_value, ok, error)

    def insert(self, _key, _value, _row):
        if _key in self.m_dict_datas.keys():
            value_dict = self.m_dict_datas[_key]
            if _value in value_dict.keys():
                rows = value_dict[_value]
                rows.append(_row)
            else:
                rows = [_row]
                value_dict[_value] = rows
        else:
            value_dict = {}
            rows = [_row]
            value_dict[_value] = rows
            self.m_dict_datas[_key] = value_dict

    def Count(self):
        return len(self.m_dict_datas)

    def GetRow(self, _index):
        return self.m_dict_datas[_index]

    def GetRowByIDValue(self, _value):
        rows = self.GetRowsByKV("C_ID", str(_value))
        if len(rows) <=0:
            return {}
        return rows[0]

    def GetRowsByKV(self, _title, _value):
        _value = str(_value)
        if _title in self.m_dict_datas.keys():
            __dict = self.m_dict_datas[_title]
            if _value in __dict.keys():
                __list = __dict[_value]
                return __list

        return []

Configs = {}

def Init():
    Configs.clear()
    for (root, dirs, files) in os.walk(r"scripts/data/csvs/"):
        for filename in files:
            path = os.path.join(root, filename)
            csv_data = CsvFile(path)
            __temp = filename.split('.', 1)
            key = __temp[0]
            DEBUG_MSG('root[%s] file[%s]' % (root, key))
            Configs[key] = csv_data

def Get(_file_name):
    if _file_name in Configs.keys():
        return Configs[_file_name]
    return None


