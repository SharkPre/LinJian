csv��ȡ����
���ܣ�
1.ֱ�Ӷ�ȡ����Ŀ¼�µ�csv�ļ����Խ���
2.���ص�ʱ�����csv���ͺϷ��ԣ������߼�ʹ��ʱ�����ʣ���Ҫ���������ã�֧��int��float��string�Լ�һ��Ψһֵ������unique��
3.�������߲�ѯ�ٶȽϿ죨�����������ռ����Խ���k-v������
4.ͨ��С����find�����ġ��С����ݣ���ֱ����������ʵ��createʱ�����dict


����ʾ��(�ļ�����skills.csv)��
C_ID,Name,Distance,CD,Speed
����ID,��������,ʩ������,��ȴʱ��,�����ٶȣ�����Ϊע�ͣ����벻�ᴦ��
int&unique,string,float,float,float
2001,��ͨ����,10,0.8,20

ʹ��ʾ����
import ConfigManager

ConfigManager.Init()

_csv_file = ConfigManager.Get('skills'):

#�˴����ý�2001תΪstring�����ֲ�ѯ��ʽ��Ҫ�����ļ�����C_ID�У��������Ψһֵ�������ص�һ���鵽��ֵ
_skill = _csv_file.GetRowByIDValue(2001)

#��һ�ֲ�ѯ��ʽ,֧������k-v��ѯ
#_skills = _csv_file.GetRowByKV('Speed', 20)
#_skill = _skills[0] #����鵽���У����Ա�����ȡ

#�˴���������_cd�Ѿ���float���ͣ�����ֱ��ʹ��
_cd = _skill['CD']

δ��֮�������룬�������磬ϣ���Դ���а���
				