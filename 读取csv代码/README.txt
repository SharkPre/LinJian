csv读取代码
功能：
1.直接读取整个目录下的csv文件加以解析
2.加载的时候会检查csv类型合法性，降低逻辑使用时出错率（需要按规则配置，支持int、float、string以及一个唯一值检查规则unique）
3.遍历或者查询速度较快（会牺牲少量空间用以建立k-v索引）
4.通过小工具find出来的‘行’数据，可直接用于引擎实体create时传入的dict


配置示例(文件名：skills.csv)：
C_ID,Name,Distance,CD,Speed
技能ID,技能名称,施法距离,冷却时间,攻击速度（此行为注释，代码不会处理）
int&unique,string,float,float,float
2001,普通攻击,10,0.8,20

使用示例：
import ConfigManager

ConfigManager.Init()

_csv_file = ConfigManager.Get('skills'):

#此处不用将2001转为string，这种查询方式需要配置文件中有C_ID列，如果不是唯一值，将返回第一个查到的值
_skill = _csv_file.GetRowByIDValue(2001)

#另一种查询方式,支持任意k-v查询
#_skills = _csv_file.GetRowByKV('Speed', 20)
#_skill = _skills[0] #如果查到多行，可以遍历获取

#此处读出来的_cd已经是float类型，可以直接使用
_cd = _skill['CD']

未尽之处见代码，大神勿喷，希望对大家有帮助
				